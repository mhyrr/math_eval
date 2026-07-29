// Fixed-replication local search for C(48,8,2).
//
// The hot state is a 48 x b incidence matrix represented as b uint64_t masks.
// Every move swaps one occurrence between two blocks, preserving block size and
// the chosen point-replication vector. Pair multiplicities are updated exactly.

#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
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
#include <tuple>
#include <utility>
#include <vector>

namespace {

constexpr int V = 48;
constexpr int K = 8;
constexpr int PAIRS = V * (V - 1) / 2;

struct Options {
  int blocks = 46;
  std::uint64_t seed = 1;
  std::uint64_t iterations = 5000000;
  int restarts = 8;
  int samples = 32;
  int stagnation = 100000;
  int max_replication = 8;
  double seconds = 0.0;
  std::string output = "best-local.txt";
  bool self_test = false;
};

struct Change {
  int id;
  int delta;
};

struct Move {
  int left_block = -1;
  int right_block = -1;
  int left_position = -1;
  int right_position = -1;
  int uncovered_delta = 0;
  std::int64_t energy_delta = 0;
  std::vector<Change> changes;
  bool valid = false;
};

class Search {
 public:
  Search(const Options& options, std::mt19937_64& rng)
      : options_(options), rng_(rng), blocks_(options.blocks),
        masks_(options.blocks), pair_count_(PAIRS), penalty_(PAIRS, 1),
        uncovered_position_(PAIRS, -1) {
    int id = 0;
    for (int left = 0; left < V; ++left) {
      for (int right = left + 1; right < V; ++right) {
        pair_id_[left][right] = pair_id_[right][left] = id;
        pair_left_[id] = left;
        pair_right_[id] = right;
        ++id;
      }
    }
    if (id != PAIRS) throw std::logic_error("pair table size");
  }

  void initialize(int restart) {
    std::fill(pair_count_.begin(), pair_count_.end(), 0);
    std::fill(penalty_.begin(), penalty_.end(), 1);
    std::fill(uncovered_position_.begin(), uncovered_position_.end(), -1);
    uncovered_pairs_.clear();

    std::vector<int> replication(V, 7);
    int surplus = options_.blocks * K - 7 * V;
    if (surplus < 0) {
      throw std::runtime_error("block target cannot give every point replication 7");
    }
    std::vector<int> order(V);
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), rng_);
    for (int point : order) {
      while (surplus > 0 && replication[point] < options_.max_replication) {
        ++replication[point];
        --surplus;
        if (options_.max_replication == 8) break;
        // With max 9, spread most increments before assigning a second one.
        if (surplus >= V) continue;
        break;
      }
      if (surplus == 0) break;
    }
    if (surplus > 0) {
      for (int point : order) {
        while (surplus > 0 && replication[point] < options_.max_replication) {
          ++replication[point];
          --surplus;
        }
      }
    }
    if (surplus != 0) throw std::runtime_error("max replication is too small");

    // Randomized bipartite Havel-Hakimi. Retrying the whole construction is
    // cheap at this density and keeps initialization easy to audit.
    bool built = false;
    for (int attempt = 0; attempt < 1000 && !built; ++attempt) {
      std::vector<int> remaining = replication;
      built = true;
      for (int block = 0; block < options_.blocks; ++block) {
        std::vector<int> candidates(V);
        std::iota(candidates.begin(), candidates.end(), 0);
        std::shuffle(candidates.begin(), candidates.end(), rng_);
        std::stable_sort(candidates.begin(), candidates.end(),
                         [&](int a, int b) { return remaining[a] > remaining[b]; });
        // Perturb the maximum-remaining rule within a small band. The hard
        // feasibility guard below prevents a late stranded point.
        int band = std::min(V, 16);
        std::shuffle(candidates.begin(), candidates.begin() + band, rng_);
        std::array<int, V> jitter{};
        for (int point = 0; point < V; ++point) {
          jitter[point] = static_cast<int>(rng_() & 7);
        }
        std::stable_sort(candidates.begin(), candidates.end(), [&](int a, int b) {
          return remaining[a] * 16 + jitter[a] >
                 remaining[b] * 16 + jitter[b];
        });
        std::array<int, K> chosen{};
        int count = 0;
        int blocks_after = options_.blocks - block - 1;
        for (int point : candidates) {
          if (remaining[point] <= 0) continue;
          chosen[count++] = point;
          if (count == K) break;
        }
        if (count != K) {
          built = false;
          break;
        }
        for (int point : chosen) --remaining[point];
        if (std::any_of(remaining.begin(), remaining.end(),
                        [&](int value) { return value > blocks_after; })) {
          built = false;
          break;
        }
        blocks_[block] = chosen;
      }
      if (built) {
        // The greedy guard should imply this; assert it before scoring.
        std::vector<int> got(V, 0);
        for (const auto& block : blocks_) {
          for (int point : block) ++got[point];
        }
        built = got == replication;
      }
    }
    if (!built) throw std::runtime_error("could not initialize incidence matrix");

    for (int block = 0; block < options_.blocks; ++block) {
      std::sort(blocks_[block].begin(), blocks_[block].end());
      masks_[block] = mask_of(blocks_[block]);
      for (int i = 0; i < K; ++i) {
        for (int j = i + 1; j < K; ++j) {
          ++pair_count_[pair_id_[blocks_[block][i]][blocks_[block][j]]];
        }
      }
    }
    if (has_duplicate_block()) {
      // Duplicates are legal search states but poor initial states. A few
      // random valid swaps almost surely remove them.
      for (int shake = 0; shake < 10000 && has_duplicate_block(); ++shake) {
        Move move = random_move();
        if (move.valid && !would_duplicate(move)) apply(move);
      }
      recompute_counts();
    }
    rebuild_uncovered();
    concentration_ = compute_concentration();
    weighted_uncovered_ = uncovered_;
    energy_ = weighted_uncovered_;
    best_uncovered_ = uncovered_;
    restart_ = restart;
  }

  bool run(std::uint64_t iterations, const std::chrono::steady_clock::time_point deadline,
           const std::string& output, int& global_best) {
    std::uint64_t last_best_iteration = 0;
    for (std::uint64_t iteration = 1; iteration <= iterations; ++iteration) {
      if (options_.seconds > 0.0 && (iteration & 8191U) == 0U &&
          std::chrono::steady_clock::now() >= deadline) {
        return false;
      }
      if (uncovered_ == 0) {
        save(output, iteration);
        return true;
      }

      Move best;
      best.energy_delta = std::numeric_limits<std::int64_t>::max();
      bool target = (rng_() % 100) < 88 && !uncovered_pairs_.empty();
      int target_pair = -1;
      if (target) {
        target_pair = uncovered_pairs_[rng_() % uncovered_pairs_.size()];
      }
      for (int sample = 0; sample < options_.samples; ++sample) {
        Move candidate =
            target ? targeted_move(target_pair, (sample & 1) != 0) : random_move();
        if (!candidate.valid || would_duplicate(candidate)) continue;
        evaluate(candidate);
        if (!best.valid || candidate.energy_delta < best.energy_delta ||
            (candidate.energy_delta == best.energy_delta &&
             candidate.uncovered_delta < best.uncovered_delta)) {
          best = std::move(candidate);
        }
      }
      if (!best.valid) continue;

      double phase =
          static_cast<double>(iteration % 200000U) / static_cast<double>(200000U);
      double temperature = 1.6 * std::pow(0.015 / 1.6, phase);
      bool accept = best.energy_delta <= 0;
      if (!accept) {
        double probability =
            std::exp(-static_cast<double>(best.energy_delta) / temperature);
        std::uniform_real_distribution<double> unit(0.0, 1.0);
        accept = unit(rng_) < probability;
      }
      if (accept) apply(best);

      if (uncovered_ < best_uncovered_) {
        best_uncovered_ = uncovered_;
        last_best_iteration = iteration;
        if (uncovered_ < global_best) {
          global_best = uncovered_;
          save(output, iteration);
          std::cerr << "lane=local seed=" << options_.seed
                    << " restart=" << restart_ << " iteration=" << iteration
                    << " best_uncovered=" << uncovered_
                    << " concentration=" << concentration_
                    << " hash=" << canonical_hash() << "\n";
        }
      }

      if (options_.stagnation > 0 &&
          iteration - last_best_iteration >=
              static_cast<std::uint64_t>(options_.stagnation)) {
        for (int id : uncovered_pairs_) {
          ++penalty_[id];
          ++weighted_uncovered_;
          ++energy_;
        }
        // A short perturbation of neutral/random swaps changes which exact
        // pairs carry the accumulated adaptive pressure.
        for (int shake = 0; shake < 30; ++shake) {
          Move move = random_move();
          if (move.valid && !would_duplicate(move)) {
            evaluate(move);
            apply(move);
          }
        }
        last_best_iteration = iteration;
      }

      if ((iteration & ((1U << 20) - 1U)) == 0U) {
        audit_incremental_state();
      }
    }
    return false;
  }

  int uncovered() const { return uncovered_; }

  void self_test() {
    initialize(0);
    for (int trial = 0; trial < 10000; ++trial) {
      Move move = (trial & 1) ? random_move()
                              : targeted_move(uncovered_pairs_[rng_() %
                                                              uncovered_pairs_.size()],
                                              false);
      if (!move.valid || would_duplicate(move)) continue;
      evaluate(move);
      int old_uncovered = uncovered_;
      std::int64_t old_energy = energy_;
      apply(move);
      if (uncovered_ != old_uncovered + move.uncovered_delta ||
          energy_ != old_energy + move.energy_delta) {
        throw std::logic_error("move delta mismatch");
      }
      audit_incremental_state();
    }
    std::cout << "local-search self-test: ok\n";
  }

 private:
  const Options& options_;
  std::mt19937_64& rng_;
  std::vector<std::array<int, K>> blocks_;
  std::vector<std::uint64_t> masks_;
  std::vector<int> pair_count_;
  std::vector<int> penalty_;
  std::array<std::array<int, V>, V> pair_id_{};
  std::array<int, PAIRS> pair_left_{};
  std::array<int, PAIRS> pair_right_{};
  std::vector<int> uncovered_pairs_;
  std::vector<int> uncovered_position_;
  int uncovered_ = 0;
  int concentration_ = 0;
  int weighted_uncovered_ = 0;
  std::int64_t energy_ = 0;
  int best_uncovered_ = PAIRS;
  int restart_ = 0;

  static std::uint64_t mask_of(const std::array<int, K>& block) {
    std::uint64_t mask = 0;
    for (int point : block) mask |= std::uint64_t{1} << point;
    return mask;
  }

  bool has_duplicate_block() const {
    for (int i = 0; i < options_.blocks; ++i) {
      for (int j = i + 1; j < options_.blocks; ++j) {
        if (masks_[i] == masks_[j]) return true;
      }
    }
    return false;
  }

  void recompute_counts() {
    std::fill(pair_count_.begin(), pair_count_.end(), 0);
    for (const auto& block : blocks_) {
      for (int i = 0; i < K; ++i) {
        for (int j = i + 1; j < K; ++j) {
          ++pair_count_[pair_id_[block[i]][block[j]]];
        }
      }
    }
  }

  void rebuild_uncovered() {
    uncovered_pairs_.clear();
    std::fill(uncovered_position_.begin(), uncovered_position_.end(), -1);
    for (int id = 0; id < PAIRS; ++id) {
      if (pair_count_[id] == 0) {
        uncovered_position_[id] = static_cast<int>(uncovered_pairs_.size());
        uncovered_pairs_.push_back(id);
      }
    }
    uncovered_ = static_cast<int>(uncovered_pairs_.size());
  }

  int compute_concentration() const {
    int value = 0;
    for (int count : pair_count_) {
      int repetition = std::max(0, count - 1);
      value += repetition * repetition;
    }
    return value;
  }

  int position_of(int block, int point) const {
    for (int position = 0; position < K; ++position) {
      if (blocks_[block][position] == point) return position;
    }
    return -1;
  }

  Move make_move(int left_block, int right_block, int left_position,
                 int right_position) const {
    Move move;
    if (left_block == right_block || left_position < 0 || right_position < 0) {
      return move;
    }
    int left_point = blocks_[left_block][left_position];
    int right_point = blocks_[right_block][right_position];
    if (left_point == right_point ||
        ((masks_[left_block] >> right_point) & 1U) != 0U ||
        ((masks_[right_block] >> left_point) & 1U) != 0U) {
      return move;
    }
    move.left_block = left_block;
    move.right_block = right_block;
    move.left_position = left_position;
    move.right_position = right_position;
    move.valid = true;

    auto add_change = [&](int id, int delta) {
      for (Change& change : move.changes) {
        if (change.id == id) {
          change.delta += delta;
          return;
        }
      }
      move.changes.push_back({id, delta});
    };
    for (int position = 0; position < K; ++position) {
      if (position != left_position) {
        int other = blocks_[left_block][position];
        add_change(pair_id_[left_point][other], -1);
        add_change(pair_id_[right_point][other], 1);
      }
      if (position != right_position) {
        int other = blocks_[right_block][position];
        add_change(pair_id_[right_point][other], -1);
        add_change(pair_id_[left_point][other], 1);
      }
    }
    move.changes.erase(
        std::remove_if(move.changes.begin(), move.changes.end(),
                       [](const Change& change) { return change.delta == 0; }),
        move.changes.end());
    return move;
  }

  Move targeted_move(int id, bool reverse) {
    int left_point = pair_left_[id];
    int right_point = pair_right_[id];
    if (reverse) std::swap(left_point, right_point);
    std::vector<int> left_blocks;
    std::vector<int> right_blocks;
    for (int block = 0; block < options_.blocks; ++block) {
      if ((masks_[block] >> left_point) & 1U) left_blocks.push_back(block);
      if ((masks_[block] >> right_point) & 1U) right_blocks.push_back(block);
    }
    if (left_blocks.empty() || right_blocks.empty()) return {};
    int left_block = left_blocks[rng_() % left_blocks.size()];
    int right_block = right_blocks[rng_() % right_blocks.size()];
    int right_position = position_of(right_block, right_point);
    int start = static_cast<int>(rng_() % K);
    for (int offset = 0; offset < K; ++offset) {
      int left_position = (start + offset) % K;
      int displaced = blocks_[left_block][left_position];
      if (displaced == left_point ||
          ((masks_[right_block] >> displaced) & 1U) != 0U) {
        continue;
      }
      return make_move(left_block, right_block, left_position, right_position);
    }
    return {};
  }

  Move random_move() {
    int left_block = static_cast<int>(rng_() % options_.blocks);
    int right_block = static_cast<int>(rng_() % (options_.blocks - 1));
    if (right_block >= left_block) ++right_block;
    int left_position = static_cast<int>(rng_() % K);
    int right_position = static_cast<int>(rng_() % K);
    return make_move(left_block, right_block, left_position, right_position);
  }

  bool would_duplicate(const Move& move) const {
    int left_point = blocks_[move.left_block][move.left_position];
    int right_point = blocks_[move.right_block][move.right_position];
    std::uint64_t left_mask =
        (masks_[move.left_block] & ~(std::uint64_t{1} << left_point)) |
        (std::uint64_t{1} << right_point);
    std::uint64_t right_mask =
        (masks_[move.right_block] & ~(std::uint64_t{1} << right_point)) |
        (std::uint64_t{1} << left_point);
    if (left_mask == right_mask) return true;
    for (int block = 0; block < options_.blocks; ++block) {
      if (block == move.left_block || block == move.right_block) continue;
      if (masks_[block] == left_mask || masks_[block] == right_mask) return true;
    }
    return false;
  }

  static int repetition_cost(int count) {
    int repetition = std::max(0, count - 1);
    return repetition * repetition;
  }

  void evaluate(Move& move) const {
    move.uncovered_delta = 0;
    move.energy_delta = 0;
    for (const Change& change : move.changes) {
      int old_count = pair_count_[change.id];
      int new_count = old_count + change.delta;
      if (new_count < 0) throw std::logic_error("negative multiplicity");
      if (old_count == 0 && new_count > 0) {
        --move.uncovered_delta;
        move.energy_delta -= penalty_[change.id];
      } else if (old_count > 0 && new_count == 0) {
        ++move.uncovered_delta;
        move.energy_delta += penalty_[change.id];
      }
    }
  }

  void remove_uncovered(int id) {
    int position = uncovered_position_[id];
    if (position < 0) throw std::logic_error("pair absent from uncovered index");
    int last = uncovered_pairs_.back();
    uncovered_pairs_[position] = last;
    uncovered_position_[last] = position;
    uncovered_pairs_.pop_back();
    uncovered_position_[id] = -1;
  }

  void add_uncovered(int id) {
    if (uncovered_position_[id] >= 0) {
      throw std::logic_error("pair already in uncovered index");
    }
    uncovered_position_[id] = static_cast<int>(uncovered_pairs_.size());
    uncovered_pairs_.push_back(id);
  }

  void apply(const Move& move) {
    for (const Change& change : move.changes) {
      int old_count = pair_count_[change.id];
      int new_count = old_count + change.delta;
      if (old_count == 0 && new_count > 0) {
        remove_uncovered(change.id);
        --uncovered_;
        weighted_uncovered_ -= penalty_[change.id];
      } else if (old_count > 0 && new_count == 0) {
        add_uncovered(change.id);
        ++uncovered_;
        weighted_uncovered_ += penalty_[change.id];
      }
      concentration_ += repetition_cost(new_count) - repetition_cost(old_count);
      pair_count_[change.id] = new_count;
    }
    int left_point = blocks_[move.left_block][move.left_position];
    int right_point = blocks_[move.right_block][move.right_position];
    blocks_[move.left_block][move.left_position] = right_point;
    blocks_[move.right_block][move.right_position] = left_point;
    std::sort(blocks_[move.left_block].begin(), blocks_[move.left_block].end());
    std::sort(blocks_[move.right_block].begin(), blocks_[move.right_block].end());
    masks_[move.left_block] = mask_of(blocks_[move.left_block]);
    masks_[move.right_block] = mask_of(blocks_[move.right_block]);
    energy_ += move.energy_delta;
  }

  void audit_incremental_state() const {
    std::vector<int> counts(PAIRS, 0);
    for (const auto& block : blocks_) {
      std::uint64_t mask = mask_of(block);
      if (__builtin_popcountll(mask) != K) {
        throw std::logic_error("block size invariant");
      }
      for (int i = 0; i < K; ++i) {
        for (int j = i + 1; j < K; ++j) {
          ++counts[pair_id_[block[i]][block[j]]];
        }
      }
    }
    if (counts != pair_count_) throw std::logic_error("pair-count drift");
    int uncovered = static_cast<int>(
        std::count(counts.begin(), counts.end(), 0));
    int weighted = 0;
    for (int id = 0; id < PAIRS; ++id) {
      if (counts[id] == 0) weighted += penalty_[id];
    }
    int concentration = 0;
    for (int count : counts) concentration += repetition_cost(count);
    if (uncovered != uncovered_ || weighted != weighted_uncovered_ ||
        concentration != concentration_ ||
        energy_ != weighted) {
      throw std::logic_error("score drift");
    }
    if (has_duplicate_block()) throw std::logic_error("duplicate block");
  }

  std::uint64_t canonical_hash() const {
    std::vector<std::uint64_t> sorted = masks_;
    std::sort(sorted.begin(), sorted.end());
    std::uint64_t hash = 1469598103934665603ULL;
    for (std::uint64_t mask : sorted) {
      for (int byte = 0; byte < 8; ++byte) {
        hash ^= (mask >> (byte * 8)) & 0xffU;
        hash *= 1099511628211ULL;
      }
    }
    return hash;
  }

  void save(const std::string& path, std::uint64_t iteration) const {
    std::vector<std::array<int, K>> sorted = blocks_;
    std::sort(sorted.begin(), sorted.end());
    std::ostringstream temporary;
    temporary << path << ".tmp";
    {
      std::ofstream output(temporary.str());
      if (!output) throw std::runtime_error("cannot write " + temporary.str());
      output << "# lane=local seed=" << options_.seed << " blocks="
             << options_.blocks << " restart=" << restart_
             << " iteration=" << iteration << " uncovered=" << uncovered_
             << " concentration=" << concentration_
             << " hash=" << canonical_hash() << "\n";
      for (const auto& block : sorted) {
        for (int position = 0; position < K; ++position) {
          if (position) output << ' ';
          output << block[position] + 1;
        }
        output << '\n';
      }
    }
    if (std::rename(temporary.str().c_str(), path.c_str()) != 0) {
      throw std::runtime_error("cannot replace " + path);
    }
  }
};

std::uint64_t parse_u64(const char* text, const char* name) {
  char* end = nullptr;
  unsigned long long value = std::strtoull(text, &end, 10);
  if (!end || *end != '\0') throw std::runtime_error(std::string("bad ") + name);
  return static_cast<std::uint64_t>(value);
}

int parse_int(const char* text, const char* name) {
  std::uint64_t value = parse_u64(text, name);
  if (value > static_cast<std::uint64_t>(std::numeric_limits<int>::max())) {
    throw std::runtime_error(std::string(name) + " too large");
  }
  return static_cast<int>(value);
}

double parse_double(const char* text, const char* name) {
  char* end = nullptr;
  double value = std::strtod(text, &end);
  if (!end || *end != '\0') throw std::runtime_error(std::string("bad ") + name);
  return value;
}

Options parse_options(int argc, char** argv) {
  Options options;
  for (int i = 1; i < argc; ++i) {
    std::string arg = argv[i];
    auto next = [&]() -> const char* {
      if (++i >= argc) throw std::runtime_error("missing value after " + arg);
      return argv[i];
    };
    if (arg == "--blocks") {
      options.blocks = parse_int(next(), "blocks");
    } else if (arg == "--seed") {
      options.seed = parse_u64(next(), "seed");
    } else if (arg == "--iterations") {
      options.iterations = parse_u64(next(), "iterations");
    } else if (arg == "--restarts") {
      options.restarts = parse_int(next(), "restarts");
    } else if (arg == "--samples") {
      options.samples = parse_int(next(), "samples");
    } else if (arg == "--stagnation") {
      options.stagnation = parse_int(next(), "stagnation");
    } else if (arg == "--max-replication") {
      options.max_replication = parse_int(next(), "max-replication");
    } else if (arg == "--seconds") {
      options.seconds = parse_double(next(), "seconds");
    } else if (arg == "--output") {
      options.output = next();
    } else if (arg == "--self-test") {
      options.self_test = true;
    } else if (arg == "--help") {
      std::cout
          << "usage: local_search [--blocks 44..] [--seed N]\n"
             "  [--iterations N] [--restarts N] [--samples N]\n"
             "  [--stagnation N] [--max-replication 8|9]\n"
             "  [--seconds S] [--output PATH] [--self-test]\n";
      std::exit(0);
    } else {
      throw std::runtime_error("unknown argument " + arg);
    }
  }
  if (options.blocks < 42 || options.blocks > 100 || options.restarts < 1 ||
      options.samples < 1 || options.max_replication < 8 ||
      options.max_replication > 10 || options.seconds < 0.0) {
    throw std::runtime_error("invalid option range");
  }
  return options;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    Options options = parse_options(argc, argv);
    std::mt19937_64 rng(options.seed);
    Search search(options, rng);
    if (options.self_test) {
      search.self_test();
      return 0;
    }
    int global_best = PAIRS;
    auto start = std::chrono::steady_clock::now();
    auto deadline =
        options.seconds > 0.0
            ? start + std::chrono::duration_cast<std::chrono::steady_clock::duration>(
                          std::chrono::duration<double>(options.seconds))
            : std::chrono::steady_clock::time_point::max();
    bool found = false;
    int completed_restarts = 0;
    for (int restart = 0; restart < options.restarts; ++restart) {
      if (options.seconds > 0.0 && std::chrono::steady_clock::now() >= deadline) {
        break;
      }
      search.initialize(restart);
      if (search.uncovered() < global_best) {
        global_best = search.uncovered();
      }
      found = search.run(options.iterations, deadline, options.output, global_best);
      ++completed_restarts;
      if (found) break;
    }
    double elapsed =
        std::chrono::duration<double>(std::chrono::steady_clock::now() - start)
            .count();
    std::cout << std::fixed << std::setprecision(3)
              << "lane=local blocks=" << options.blocks << " seed=" << options.seed
              << " restarts=" << completed_restarts
              << " iterations_per_restart=" << options.iterations
              << " best_uncovered=" << global_best
              << " valid=" << (found ? "yes" : "no")
              << " elapsed_seconds=" << elapsed
              << " output=" << options.output << "\n";
    return found ? 0 : 1;
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << "\n";
    return 2;
  }
}
