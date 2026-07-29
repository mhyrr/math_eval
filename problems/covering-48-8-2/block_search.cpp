// Adaptive block-replacement search for C(48,8,2).
//
// Unlike local_search.cpp, this lane does not freeze the replication vector.
// It alternates one-point insertions that force an uncovered pair together
// with whole-block GRASP rebuilds around the current deficit set.

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace {

constexpr int V = 48;
constexpr int K = 8;
constexpr int NPAIRS = V * (V - 1) / 2;
using Block = std::array<int, K>;

struct Options {
  int blocks = 46;
  std::uint64_t seed = 1;
  std::uint64_t iterations = 3000000;
  int restarts = 8;
  int samples = 32;
  int stagnation = 75000;
  double seconds = 0.0;
  std::string output = "best-block.txt";
  std::string initial;
  bool self_test = false;
};

struct Delta {
  int pair;
  int amount;
};

struct Move {
  int block = -1;
  Block replacement{};
  std::vector<Delta> deltas;
  int uncovered_delta = 0;
  std::int64_t energy_delta = 0;
  bool valid = false;
};

class Solver {
 public:
  Solver(const Options& options, std::mt19937_64& rng)
      : options_(options), rng_(rng), blocks_(options.blocks),
        masks_(options.blocks), counts_(NPAIRS), penalties_(NPAIRS, 1),
        uncovered_position_(NPAIRS, -1) {
    int id = 0;
    for (int a = 0; a < V; ++a) {
      for (int b = a + 1; b < V; ++b) {
        pair_id_[a][b] = pair_id_[b][a] = id;
        pair_a_[id] = a;
        pair_b_[id] = b;
        ++id;
      }
    }
  }

  void initialize(int restart) {
    restart_ = restart;
    if (!options_.initial.empty()) {
      load_initial(options_.initial);
      std::fill(penalties_.begin(), penalties_.end(), 1);
      recompute();
      best_uncovered_ = uncovered_;
      return;
    }
    std::vector<int> points(V);
    std::iota(points.begin(), points.end(), 0);
    int block = 0;
    // Seven random parallel classes guarantee initial replication >= 7 and
    // give a more useful start than independent random blocks.
    for (int round = 0; round < 7; ++round) {
      std::shuffle(points.begin(), points.end(), rng_);
      for (int part = 0; part < 6 && block < options_.blocks; ++part) {
        for (int j = 0; j < K; ++j) blocks_[block][j] = points[part * K + j];
        std::sort(blocks_[block].begin(), blocks_[block].end());
        ++block;
      }
    }
    std::shuffle(points.begin(), points.end(), rng_);
    while (block < options_.blocks) {
      for (int j = 0; j < K; ++j) blocks_[block][j] = points[(block * K + j) % V];
      std::sort(blocks_[block].begin(), blocks_[block].end());
      ++block;
    }
    // Extremely unlikely duplicate parallel parts are repaired by regenerating
    // the offending block uniformly.
    for (int i = 0; i < options_.blocks; ++i) {
      masks_[i] = mask_of(blocks_[i]);
      while (duplicate_of_other(i, masks_[i])) {
        std::shuffle(points.begin(), points.end(), rng_);
        std::copy_n(points.begin(), K, blocks_[i].begin());
        std::sort(blocks_[i].begin(), blocks_[i].end());
        masks_[i] = mask_of(blocks_[i]);
      }
    }
    std::fill(penalties_.begin(), penalties_.end(), 1);
    recompute();
    best_uncovered_ = uncovered_;
  }

  bool run(const std::chrono::steady_clock::time_point deadline,
           int& global_best) {
    std::uint64_t last_improvement = 0;
    for (std::uint64_t iteration = 1; iteration <= options_.iterations;
         ++iteration) {
      if (uncovered_ == 0) {
        save(iteration);
        return true;
      }
      if (options_.seconds > 0.0 && (iteration & 4095U) == 0U &&
          std::chrono::steady_clock::now() >= deadline) {
        return false;
      }
      int target = uncovered_pairs_[rng_() % uncovered_pairs_.size()];
      Move best;
      best.energy_delta = std::numeric_limits<std::int64_t>::max();
      for (int sample = 0; sample < options_.samples; ++sample) {
        Move candidate;
        if ((sample % 4) < 3) {
          candidate = insertion_move(target, (sample & 1) != 0);
        } else {
          candidate = rebuild_move(target);
        }
        if (!candidate.valid) continue;
        evaluate(candidate);
        if (!best.valid || candidate.energy_delta < best.energy_delta ||
            (candidate.energy_delta == best.energy_delta &&
             candidate.uncovered_delta < best.uncovered_delta)) {
          best = std::move(candidate);
        }
      }
      if (!best.valid) continue;
      double phase =
          static_cast<double>(iteration % 150000U) / static_cast<double>(150000U);
      double temperature = 12.0 * std::pow(0.05 / 12.0, phase);
      bool accept = best.energy_delta <= 0;
      if (!accept) {
        std::uniform_real_distribution<double> unit(0.0, 1.0);
        accept = unit(rng_) <
                 std::exp(-static_cast<double>(best.energy_delta) / temperature);
      }
      if (accept) apply(best);

      if (uncovered_ < best_uncovered_) {
        best_uncovered_ = uncovered_;
        last_improvement = iteration;
        if (uncovered_ < global_best) {
          global_best = uncovered_;
          save(iteration);
          std::cerr << "lane=block seed=" << options_.seed
                    << " restart=" << restart_ << " iteration=" << iteration
                    << " best_uncovered=" << uncovered_
                    << " concentration=" << concentration_
                    << " replication_cost=" << replication_cost_
                    << " hash=" << canonical_hash() << "\n";
        }
      }
      if (options_.stagnation > 0 &&
          iteration - last_improvement >=
              static_cast<std::uint64_t>(options_.stagnation)) {
        for (int id : uncovered_pairs_) ++penalties_[id];
        compute_energy();
        // Destroy one low-value block and replace it with a fresh deficit block.
        Move kick = rebuild_move(
            uncovered_pairs_[rng_() % uncovered_pairs_.size()]);
        if (kick.valid) {
          evaluate(kick);
          apply(kick);
        }
        last_improvement = iteration;
      }
      if ((iteration & ((1U << 19) - 1U)) == 0U) audit();
    }
    return false;
  }

  void self_test() {
    initialize(0);
    for (int i = 0; i < 5000; ++i) {
      int target = uncovered_pairs_[rng_() % uncovered_pairs_.size()];
      Move move = (i & 1) ? insertion_move(target, false)
                          : rebuild_move(target);
      if (!move.valid) continue;
      evaluate(move);
      int prior_uncovered = uncovered_;
      std::int64_t prior_energy = energy_;
      apply(move);
      if (uncovered_ != prior_uncovered + move.uncovered_delta ||
          energy_ != prior_energy + move.energy_delta) {
        throw std::logic_error("reported move delta is wrong");
      }
      audit();
    }
    std::cout << "block-search self-test: ok\n";
  }

  int uncovered() const { return uncovered_; }

 private:
  const Options& options_;
  std::mt19937_64& rng_;
  std::vector<Block> blocks_;
  std::vector<std::uint64_t> masks_;
  std::array<std::array<int, V>, V> pair_id_{};
  std::array<int, NPAIRS> pair_a_{};
  std::array<int, NPAIRS> pair_b_{};
  std::vector<int> counts_;
  std::vector<int> penalties_;
  std::vector<int> uncovered_pairs_;
  std::vector<int> uncovered_position_;
  std::array<int, V> replication_{};
  int uncovered_ = 0;
  int weighted_uncovered_ = 0;
  int concentration_ = 0;
  int replication_cost_ = 0;
  std::int64_t energy_ = 0;
  int best_uncovered_ = NPAIRS;
  int restart_ = 0;

  void load_initial(const std::string& path) {
    std::ifstream input(path);
    if (!input) throw std::runtime_error("cannot read initial candidate " + path);
    std::vector<Block> parsed;
    std::string line;
    while (std::getline(input, line)) {
      std::size_t comment = line.find('#');
      if (comment != std::string::npos) line.erase(comment);
      std::istringstream fields(line);
      std::vector<int> values;
      int value = 0;
      while (fields >> value) values.push_back(value);
      if (values.empty()) continue;
      if (values.size() != K) {
        throw std::runtime_error("initial candidate has a block of wrong size");
      }
      Block block{};
      for (int i = 0; i < K; ++i) {
        if (values[i] < 1 || values[i] > V) {
          throw std::runtime_error("initial candidate point outside 1..48");
        }
        block[i] = values[i] - 1;
      }
      std::sort(block.begin(), block.end());
      if (std::adjacent_find(block.begin(), block.end()) != block.end()) {
        throw std::runtime_error("initial candidate repeats a point in a block");
      }
      parsed.push_back(block);
    }
    if (static_cast<int>(parsed.size()) != options_.blocks) {
      throw std::runtime_error("initial candidate block count does not match --blocks");
    }
    blocks_ = parsed;
    for (int block = 0; block < options_.blocks; ++block) {
      masks_[block] = mask_of(blocks_[block]);
      if (duplicate_of_other(block, masks_[block])) {
        throw std::runtime_error("initial candidate has duplicate blocks");
      }
    }
  }

  static std::uint64_t mask_of(const Block& block) {
    std::uint64_t mask = 0;
    for (int point : block) mask |= std::uint64_t{1} << point;
    return mask;
  }

  static int repetition_cost(int count) {
    int excess = std::max(0, count - 1);
    return excess * excess;
  }

  int point_cost(int replication) const {
    // Six times the balanced target: b/6 is the average replication.
    int offset = 6 * replication - options_.blocks;
    return offset * offset;
  }

  bool duplicate_of_other(int changed_block, std::uint64_t mask) const {
    for (int i = 0; i < options_.blocks; ++i) {
      if (i != changed_block && masks_[i] == mask) return true;
    }
    return false;
  }

  void recompute() {
    std::fill(counts_.begin(), counts_.end(), 0);
    replication_.fill(0);
    for (const Block& block : blocks_) {
      for (int point : block) ++replication_[point];
      for (int i = 0; i < K; ++i) {
        for (int j = i + 1; j < K; ++j) {
          ++counts_[pair_id_[block[i]][block[j]]];
        }
      }
    }
    rebuild_uncovered();
    compute_energy();
  }

  void rebuild_uncovered() {
    uncovered_pairs_.clear();
    std::fill(uncovered_position_.begin(), uncovered_position_.end(), -1);
    for (int id = 0; id < NPAIRS; ++id) {
      if (counts_[id] == 0) {
        uncovered_position_[id] = static_cast<int>(uncovered_pairs_.size());
        uncovered_pairs_.push_back(id);
      }
    }
    uncovered_ = static_cast<int>(uncovered_pairs_.size());
  }

  void compute_energy() {
    weighted_uncovered_ = 0;
    concentration_ = 0;
    for (int id = 0; id < NPAIRS; ++id) {
      if (counts_[id] == 0) weighted_uncovered_ += penalties_[id];
      concentration_ += repetition_cost(counts_[id]);
    }
    replication_cost_ = 0;
    for (int count : replication_) replication_cost_ += point_cost(count);
    energy_ = 10LL * weighted_uncovered_ + concentration_;
  }

  void add_delta(std::vector<Delta>& deltas, int pair, int amount) const {
    for (Delta& delta : deltas) {
      if (delta.pair == pair) {
        delta.amount += amount;
        return;
      }
    }
    deltas.push_back({pair, amount});
  }

  Move replacement_move(int block_index, Block replacement) const {
    Move move;
    std::sort(replacement.begin(), replacement.end());
    if (std::adjacent_find(replacement.begin(), replacement.end()) !=
        replacement.end()) {
      return move;
    }
    std::uint64_t replacement_mask = mask_of(replacement);
    if (replacement_mask == masks_[block_index] ||
        duplicate_of_other(block_index, replacement_mask)) {
      return move;
    }
    move.block = block_index;
    move.replacement = replacement;
    move.valid = true;
    const Block& old = blocks_[block_index];
    for (int i = 0; i < K; ++i) {
      for (int j = i + 1; j < K; ++j) {
        add_delta(move.deltas, pair_id_[old[i]][old[j]], -1);
        add_delta(move.deltas, pair_id_[replacement[i]][replacement[j]], 1);
      }
    }
    move.deltas.erase(
        std::remove_if(move.deltas.begin(), move.deltas.end(),
                       [](const Delta& delta) { return delta.amount == 0; }),
        move.deltas.end());
    return move;
  }

  Move insertion_move(int target, bool reverse) {
    int anchor = pair_a_[target];
    int inserted = pair_b_[target];
    if (reverse) std::swap(anchor, inserted);
    std::vector<int> containing;
    for (int block = 0; block < options_.blocks; ++block) {
      if ((masks_[block] >> anchor) & 1U) containing.push_back(block);
    }
    if (containing.empty()) return {};
    int block = containing[rng_() % containing.size()];
    int start = static_cast<int>(rng_() % K);
    Move best;
    best.energy_delta = std::numeric_limits<std::int64_t>::max();
    for (int offset = 0; offset < K; ++offset) {
      int position = (start + offset) % K;
      if (blocks_[block][position] == anchor) continue;
      Block replacement = blocks_[block];
      replacement[position] = inserted;
      Move candidate = replacement_move(block, replacement);
      if (!candidate.valid) continue;
      evaluate(candidate);
      if (!best.valid || candidate.energy_delta < best.energy_delta) {
        best = std::move(candidate);
      }
    }
    return best;
  }

  int weak_block() {
    int best = static_cast<int>(rng_() % options_.blocks);
    int best_unique = K * (K - 1) / 2 + 1;
    for (int trial = 0; trial < 5; ++trial) {
      int candidate = static_cast<int>(rng_() % options_.blocks);
      int unique = 0;
      const Block& block = blocks_[candidate];
      for (int i = 0; i < K; ++i) {
        for (int j = i + 1; j < K; ++j) {
          if (counts_[pair_id_[block[i]][block[j]]] == 1) ++unique;
        }
      }
      if (unique < best_unique) {
        best = candidate;
        best_unique = unique;
      }
    }
    return best;
  }

  Move rebuild_move(int target) {
    int removed = weak_block();
    std::array<bool, NPAIRS> deficit{};
    for (int id = 0; id < NPAIRS; ++id) deficit[id] = counts_[id] == 0;
    const Block& old = blocks_[removed];
    for (int i = 0; i < K; ++i) {
      for (int j = i + 1; j < K; ++j) {
        int id = pair_id_[old[i]][old[j]];
        if (counts_[id] == 1) deficit[id] = true;
      }
    }

    Block replacement{};
    replacement[0] = pair_a_[target];
    replacement[1] = pair_b_[target];
    int chosen = 2;
    std::uint64_t chosen_mask =
        (std::uint64_t{1} << replacement[0]) |
        (std::uint64_t{1} << replacement[1]);
    while (chosen < K) {
      std::array<std::pair<int, int>, V> ranked{};
      int candidate_count = 0;
      for (int point = 0; point < V; ++point) {
        if ((chosen_mask >> point) & 1U) continue;
        int score = 0;
        for (int j = 0; j < chosen; ++j) {
          int id = pair_id_[point][replacement[j]];
          if (deficit[id]) score += 32 * penalties_[id];
          if (counts_[id] == 1) score += 2;
        }
        score += std::max(0, 8 - replication_[point]);
        score += static_cast<int>(rng_() % 13);
        ranked[candidate_count++] = {score, point};
      }
      std::sort(ranked.begin(), ranked.begin() + candidate_count,
                [](const auto& left, const auto& right) {
                  return left.first > right.first;
                });
      int restricted = std::min(4, candidate_count);
      int point = ranked[rng_() % restricted].second;
      replacement[chosen++] = point;
      chosen_mask |= std::uint64_t{1} << point;
    }
    return replacement_move(removed, replacement);
  }

  void evaluate(Move& move) const {
    if (!move.valid) return;
    move.uncovered_delta = 0;
    move.energy_delta = 0;
    for (const Delta& delta : move.deltas) {
      int before = counts_[delta.pair];
      int after = before + delta.amount;
      if (after < 0) throw std::logic_error("negative pair count");
      if (before == 0 && after > 0) {
        --move.uncovered_delta;
        move.energy_delta -= 10LL * penalties_[delta.pair];
      } else if (before > 0 && after == 0) {
        ++move.uncovered_delta;
        move.energy_delta += 10LL * penalties_[delta.pair];
      }
      move.energy_delta += repetition_cost(after) - repetition_cost(before);
    }
    std::array<int, V> replication_delta{};
    for (int point : blocks_[move.block]) --replication_delta[point];
    for (int point : move.replacement) ++replication_delta[point];
    int replication_change = 0;
    for (int point = 0; point < V; ++point) {
      if (replication_delta[point] != 0) {
        replication_change +=
            point_cost(replication_[point] + replication_delta[point]) -
            point_cost(replication_[point]);
      }
    }
    (void)replication_change;
  }

  void remove_uncovered(int id) {
    int position = uncovered_position_[id];
    int last = uncovered_pairs_.back();
    uncovered_pairs_[position] = last;
    uncovered_position_[last] = position;
    uncovered_pairs_.pop_back();
    uncovered_position_[id] = -1;
  }

  void add_uncovered(int id) {
    uncovered_position_[id] = static_cast<int>(uncovered_pairs_.size());
    uncovered_pairs_.push_back(id);
  }

  void apply(const Move& move) {
    for (const Delta& delta : move.deltas) {
      int before = counts_[delta.pair];
      int after = before + delta.amount;
      if (before == 0 && after > 0) {
        remove_uncovered(delta.pair);
        --uncovered_;
        weighted_uncovered_ -= penalties_[delta.pair];
      } else if (before > 0 && after == 0) {
        add_uncovered(delta.pair);
        ++uncovered_;
        weighted_uncovered_ += penalties_[delta.pair];
      }
      concentration_ += repetition_cost(after) - repetition_cost(before);
      counts_[delta.pair] = after;
    }
    std::array<int, V> replication_delta{};
    for (int point : blocks_[move.block]) --replication_delta[point];
    for (int point : move.replacement) ++replication_delta[point];
    int cost_change = 0;
    for (int point = 0; point < V; ++point) {
      if (replication_delta[point] != 0) {
        cost_change +=
            point_cost(replication_[point] + replication_delta[point]) -
            point_cost(replication_[point]);
        replication_[point] += replication_delta[point];
      }
    }
    replication_cost_ += cost_change;
    blocks_[move.block] = move.replacement;
    masks_[move.block] = mask_of(move.replacement);
    energy_ += move.energy_delta;
  }

  void audit() const {
    std::vector<int> counts(NPAIRS, 0);
    std::array<int, V> replication{};
    for (int block = 0; block < options_.blocks; ++block) {
      if (__builtin_popcountll(masks_[block]) != K ||
          masks_[block] != mask_of(blocks_[block])) {
        throw std::logic_error("block mask invariant");
      }
      if (duplicate_of_other(block, masks_[block])) {
        throw std::logic_error("duplicate block");
      }
      for (int point : blocks_[block]) ++replication[point];
      for (int i = 0; i < K; ++i) {
        for (int j = i + 1; j < K; ++j) {
          ++counts[pair_id_[blocks_[block][i]][blocks_[block][j]]];
        }
      }
    }
    if (counts != counts_ || replication != replication_) {
      throw std::logic_error("incremental state drift");
    }
    int uncovered =
        static_cast<int>(std::count(counts.begin(), counts.end(), 0));
    int weighted = 0;
    int concentration = 0;
    for (int id = 0; id < NPAIRS; ++id) {
      if (counts[id] == 0) weighted += penalties_[id];
      concentration += repetition_cost(counts[id]);
    }
    int replication_cost = 0;
    for (int count : replication) replication_cost += point_cost(count);
    std::int64_t energy = 10LL * weighted + concentration;
    if (uncovered != uncovered_ || weighted != weighted_uncovered_ ||
        concentration != concentration_ ||
        replication_cost != replication_cost_ || energy != energy_) {
      throw std::logic_error("incremental score drift");
    }
  }

  std::uint64_t canonical_hash() const {
    std::vector<std::uint64_t> sorted = masks_;
    std::sort(sorted.begin(), sorted.end());
    std::uint64_t hash = 1469598103934665603ULL;
    for (std::uint64_t mask : sorted) {
      hash ^= mask;
      hash *= 1099511628211ULL;
    }
    return hash;
  }

  void save(std::uint64_t iteration) const {
    std::vector<Block> sorted = blocks_;
    std::sort(sorted.begin(), sorted.end());
    std::string temporary = options_.output + ".tmp";
    {
      std::ofstream output(temporary);
      if (!output) throw std::runtime_error("cannot write " + temporary);
      output << "# lane=block seed=" << options_.seed << " blocks="
             << options_.blocks << " restart=" << restart_
             << " iteration=" << iteration << " uncovered=" << uncovered_
             << " concentration=" << concentration_
             << " replication_cost=" << replication_cost_
             << " hash=" << canonical_hash() << "\n";
      for (const Block& block : sorted) {
        for (int j = 0; j < K; ++j) {
          if (j) output << ' ';
          output << block[j] + 1;
        }
        output << '\n';
      }
    }
    if (std::rename(temporary.c_str(), options_.output.c_str()) != 0) {
      throw std::runtime_error("cannot replace " + options_.output);
    }
  }
};

std::uint64_t u64(const char* text, const char* label) {
  char* end = nullptr;
  unsigned long long value = std::strtoull(text, &end, 10);
  if (!end || *end) throw std::runtime_error(std::string("bad ") + label);
  return static_cast<std::uint64_t>(value);
}

int integer(const char* text, const char* label) {
  std::uint64_t value = u64(text, label);
  if (value > static_cast<std::uint64_t>(std::numeric_limits<int>::max())) {
    throw std::runtime_error(std::string(label) + " too large");
  }
  return static_cast<int>(value);
}

double real(const char* text, const char* label) {
  char* end = nullptr;
  double value = std::strtod(text, &end);
  if (!end || *end) throw std::runtime_error(std::string("bad ") + label);
  return value;
}

Options options_from(int argc, char** argv) {
  Options options;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    auto next = [&]() {
      if (++i >= argc) throw std::runtime_error("missing value after " + arg);
      return argv[i];
    };
    if (arg == "--blocks") {
      options.blocks = integer(next(), "blocks");
    } else if (arg == "--seed") {
      options.seed = u64(next(), "seed");
    } else if (arg == "--iterations") {
      options.iterations = u64(next(), "iterations");
    } else if (arg == "--restarts") {
      options.restarts = integer(next(), "restarts");
    } else if (arg == "--samples") {
      options.samples = integer(next(), "samples");
    } else if (arg == "--stagnation") {
      options.stagnation = integer(next(), "stagnation");
    } else if (arg == "--seconds") {
      options.seconds = real(next(), "seconds");
    } else if (arg == "--output") {
      options.output = next();
    } else if (arg == "--initial") {
      options.initial = next();
    } else if (arg == "--self-test") {
      options.self_test = true;
    } else if (arg == "--help") {
      std::cout << "usage: block_search [--blocks N] [--seed N]"
                   " [--iterations N] [--restarts N] [--samples N]"
                   " [--stagnation N] [--seconds S] [--output PATH]"
                   " [--initial PATH] [--self-test]\n";
      std::exit(0);
    } else {
      throw std::runtime_error("unknown argument " + arg);
    }
  }
  if (options.blocks < 42 || options.restarts < 1 || options.samples < 1 ||
      options.stagnation < 0 || options.seconds < 0.0) {
    throw std::runtime_error("invalid option range");
  }
  return options;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    Options options = options_from(argc, argv);
    std::mt19937_64 rng(options.seed);
    Solver solver(options, rng);
    if (options.self_test) {
      solver.self_test();
      return 0;
    }
    int global_best = NPAIRS;
    auto start = std::chrono::steady_clock::now();
    auto deadline =
        options.seconds > 0.0
            ? start + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                          std::chrono::duration<double>(options.seconds))
            : std::chrono::steady_clock::time_point::max();
    bool valid = false;
    int restarts = 0;
    for (int restart = 0; restart < options.restarts; ++restart) {
      if (options.seconds > 0.0 && std::chrono::steady_clock::now() >= deadline) {
        break;
      }
      solver.initialize(restart);
      // The supplied initial state may already be the global checkpoint.
      // Solver::run only reports strict improvements from that restart.
      if (restart == 0 && !options.initial.empty()) {
        global_best = std::min(global_best, solver.uncovered());
      }
      valid = solver.run(deadline, global_best);
      ++restarts;
      if (valid) break;
    }
    double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
            .count();
    std::cout << std::fixed << std::setprecision(3)
              << "lane=block blocks=" << options.blocks << " seed=" << options.seed
              << " restarts=" << restarts << " best_uncovered=" << global_best
              << " valid=" << (valid ? "yes" : "no")
              << " elapsed_seconds=" << elapsed
              << " output=" << options.output << "\n";
    return valid ? 0 : 1;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << "\n";
    return 2;
  }
}
