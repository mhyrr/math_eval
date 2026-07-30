// Clean-room structured search for C(48,8,2).
//
// The point set is six layers of Z_8.  The generator adds one to the
// coordinate in every layer.  A candidate consists of five full block orbits
// (5*8 blocks), one orbit of length four, and one orbit of length two.
//
// Build:
//   c++ -std=c++20 -O3 -DNDEBUG orbit8_search.cpp -o /tmp/orbit8_search
//
// Example:
//   /tmp/orbit8_search --seed 8101 --steps 5000000 --restarts 8 \
//     --output orbit8_best_8101.txt --checkpoint orbit8_checkpoint_8101.txt \
//     --log orbit8_experiments.csv

#include <algorithm>
#include <array>
#include <bit>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <map>
#include <numeric>
#include <random>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

namespace {

constexpr int kLayers = 6;
constexpr int kCycle = 8;
constexpr int kPoints = kLayers * kCycle;
constexpr int kBlockSize = 8;
constexpr int kFullOrbits = 5;
constexpr int kPairClasses = 144;
constexpr int kBlocks = 46;

using Mask = std::uint64_t;
using Coverage = std::array<std::uint8_t, kPairClasses>;

struct PairSystem {
  std::array<std::array<int, kPoints>, kPoints> class_of{};
  std::array<int, kPairClasses> class_size{};
  std::array<std::vector<std::pair<int, int>>, kPairClasses> members;

  PairSystem() {
    for (auto& row : class_of) row.fill(-1);
    std::map<std::pair<int, int>, int> ids;
    int next_id = 0;
    for (int left = 0; left < kPoints; ++left) {
      for (int right = left + 1; right < kPoints; ++right) {
        std::pair<int, int> representative{kPoints, kPoints};
        for (int shift = 0; shift < kCycle; ++shift) {
          int a = shift_point(left, shift);
          int b = shift_point(right, shift);
          if (a > b) std::swap(a, b);
          representative = std::min(representative, std::pair{a, b});
        }
        auto [where, inserted] = ids.emplace(representative, next_id);
        if (inserted) ++next_id;
        const int id = where->second;
        class_of[left][right] = class_of[right][left] = id;
        ++class_size[id];
        members[id].push_back({left, right});
      }
    }
    if (next_id != kPairClasses) {
      throw std::logic_error("pair-orbit count is not 144");
    }
  }

  static int shift_point(int point, int amount) {
    const int layer = point / kCycle;
    const int coordinate = point % kCycle;
    return layer * kCycle + (coordinate + amount) % kCycle;
  }
};

const PairSystem kPairs;

Mask point_bit(int point) { return Mask{1} << point; }

std::vector<int> points_of(Mask block) {
  std::vector<int> points;
  points.reserve(kBlockSize);
  while (block != 0) {
    const int point = std::countr_zero(block);
    points.push_back(point);
    block &= block - 1;
  }
  return points;
}

Mask shift_mask(Mask block, int amount) {
  Mask shifted = 0;
  while (block != 0) {
    const int point = std::countr_zero(block);
    shifted |= point_bit(PairSystem::shift_point(point, amount));
    block &= block - 1;
  }
  return shifted;
}

int orbit_size(Mask block) {
  for (int period = 1; period <= kCycle; ++period) {
    if (shift_mask(block, period) == block) return period;
  }
  throw std::logic_error("Z_8 orbit has no period");
}

Mask canonical_orbit_representative(Mask block) {
  Mask canonical = block;
  for (int shift = 1; shift < kCycle; ++shift) {
    canonical = std::min(canonical, shift_mask(block, shift));
  }
  return canonical;
}

Coverage component_coverage(Mask base, int length) {
  if (orbit_size(base) != length) {
    throw std::logic_error("component base has the wrong orbit length");
  }
  std::array<int, kPairClasses> incidences{};
  const auto points = points_of(base);
  if (static_cast<int>(points.size()) != kBlockSize) {
    throw std::logic_error("component base is not an 8-set");
  }
  for (int i = 0; i < kBlockSize; ++i) {
    for (int j = i + 1; j < kBlockSize; ++j) {
      incidences[kPairs.class_of[points[i]][points[j]]] += length;
    }
  }
  Coverage result{};
  for (int id = 0; id < kPairClasses; ++id) {
    if (incidences[id] % kPairs.class_size[id] != 0) {
      throw std::logic_error("component incidence is not constant on a pair orbit");
    }
    result[id] =
        static_cast<std::uint8_t>(incidences[id] / kPairs.class_size[id]);
  }
  return result;
}

std::vector<Mask> enumerate_short2() {
  std::array<Mask, kLayers * 2> parity_cosets{};
  for (int layer = 0; layer < kLayers; ++layer) {
    for (int parity = 0; parity < 2; ++parity) {
      Mask coset = 0;
      for (int coordinate = parity; coordinate < kCycle; coordinate += 2) {
        coset |= point_bit(layer * kCycle + coordinate);
      }
      parity_cosets[layer * 2 + parity] = coset;
    }
  }

  std::set<Mask> representatives;
  for (int first = 0; first < kLayers * 2; ++first) {
    for (int second = first + 1; second < kLayers * 2; ++second) {
      const Mask block = parity_cosets[first] | parity_cosets[second];
      if (std::popcount(block) == kBlockSize && orbit_size(block) == 2) {
        representatives.insert(canonical_orbit_representative(block));
      }
    }
  }
  return {representatives.begin(), representatives.end()};
}

std::vector<Mask> enumerate_short4() {
  std::array<Mask, kLayers * 4> antipodal_pairs{};
  for (int layer = 0; layer < kLayers; ++layer) {
    for (int coordinate = 0; coordinate < 4; ++coordinate) {
      antipodal_pairs[layer * 4 + coordinate] =
          point_bit(layer * kCycle + coordinate) |
          point_bit(layer * kCycle + coordinate + 4);
    }
  }

  std::set<Mask> representatives;
  const int count = static_cast<int>(antipodal_pairs.size());
  for (int a = 0; a < count; ++a) {
    for (int b = a + 1; b < count; ++b) {
      for (int c = b + 1; c < count; ++c) {
        for (int d = c + 1; d < count; ++d) {
          const Mask block = antipodal_pairs[a] | antipodal_pairs[b] |
                             antipodal_pairs[c] | antipodal_pairs[d];
          if (std::popcount(block) == kBlockSize && orbit_size(block) == 4) {
            representatives.insert(canonical_orbit_representative(block));
          }
        }
      }
    }
  }
  return {representatives.begin(), representatives.end()};
}

const std::vector<Mask> kShort2 = enumerate_short2();
const std::vector<Mask> kShort4 = enumerate_short4();

struct Score {
  int uncovered_pairs = std::numeric_limits<int>::max();
  int square_excess = std::numeric_limits<int>::max();
  int maximum_multiplicity = std::numeric_limits<int>::max();

  auto key() const {
    return std::tuple{uncovered_pairs, square_excess, maximum_multiplicity};
  }
};

bool operator<(const Score& left, const Score& right) {
  return left.key() < right.key();
}

struct State {
  std::array<Mask, kFullOrbits> full{};
  int short4_index = 0;
  int short2_index = 0;
  std::array<Coverage, kFullOrbits + 2> component{};
  std::array<std::uint8_t, kPairClasses> multiplicity{};
  Score score;
};

Score score_multiplicities(
    const std::array<std::uint8_t, kPairClasses>& multiplicity) {
  Score score{0, 0, 0};
  for (int id = 0; id < kPairClasses; ++id) {
    const int value = multiplicity[id];
    if (value == 0) score.uncovered_pairs += kPairs.class_size[id];
    const int excess = std::max(0, value - 1);
    score.square_excess += kPairs.class_size[id] * excess * excess;
    score.maximum_multiplicity = std::max(score.maximum_multiplicity, value);
  }
  return score;
}

void rebuild(State& state) {
  state.multiplicity.fill(0);
  for (int index = 0; index < kFullOrbits; ++index) {
    state.component[index] = component_coverage(state.full[index], 8);
  }
  state.component[kFullOrbits] =
      component_coverage(kShort4[state.short4_index], 4);
  state.component[kFullOrbits + 1] =
      component_coverage(kShort2[state.short2_index], 2);
  for (const auto& component : state.component) {
    for (int id = 0; id < kPairClasses; ++id) {
      state.multiplicity[id] += component[id];
    }
  }
  state.score = score_multiplicities(state.multiplicity);
}

bool distinct_full_orbits(const State& state) {
  std::set<Mask> representatives;
  for (Mask block : state.full) {
    if (std::popcount(block) != kBlockSize || orbit_size(block) != 8) {
      return false;
    }
    representatives.insert(canonical_orbit_representative(block));
  }
  return static_cast<int>(representatives.size()) == kFullOrbits;
}

std::array<int, kLayers> layer_replications(const State& state) {
  std::array<int, kLayers> replication{};
  for (Mask block : state.full) {
    for (int point : points_of(block)) ++replication[point / kCycle];
  }
  for (int layer = 0; layer < kLayers; ++layer) {
    const Mask layer_mask = ((Mask{1} << kCycle) - 1) << (layer * kCycle);
    const int in_short4 =
        std::popcount(kShort4[state.short4_index] & layer_mask);
    const int in_short2 =
        std::popcount(kShort2[state.short2_index] & layer_mask);
    if (in_short4 % 2 != 0 || in_short2 % 4 != 0) {
      throw std::logic_error("short orbit has nonintegral layer replication");
    }
    replication[layer] += in_short4 / 2 + in_short2 / 4;
  }
  return replication;
}

bool acceptable_replications(const State& state, bool balanced) {
  const auto replication = layer_replications(state);
  for (int value : replication) {
    // Every point in a layer occurs value times.  Six occurrences cover at
    // most 6*7=42 of its 47 required partners, so seven is mandatory.
    if (value < 7) return false;
    if (balanced && value > 8) return false;
  }
  return true;
}

std::vector<Mask> expanded_blocks(const State& state) {
  std::vector<Mask> blocks;
  blocks.reserve(kBlocks);
  for (Mask base : state.full) {
    for (int shift = 0; shift < 8; ++shift) {
      blocks.push_back(shift_mask(base, shift));
    }
  }
  for (int shift = 0; shift < 4; ++shift) {
    blocks.push_back(shift_mask(kShort4[state.short4_index], shift));
  }
  for (int shift = 0; shift < 2; ++shift) {
    blocks.push_back(shift_mask(kShort2[state.short2_index], shift));
  }
  std::sort(blocks.begin(), blocks.end(), [](Mask left, Mask right) {
    return points_of(left) < points_of(right);
  });
  return blocks;
}

bool exact_structural_validity(const State& state) {
  if (!distinct_full_orbits(state)) return false;
  const auto blocks = expanded_blocks(state);
  if (static_cast<int>(blocks.size()) != kBlocks) return false;
  if (std::adjacent_find(blocks.begin(), blocks.end()) != blocks.end()) {
    return false;
  }
  for (Mask block : blocks) {
    if (std::popcount(block) != kBlockSize) return false;
  }
  return true;
}

Score direct_score(const State& state) {
  std::array<std::array<int, kPoints>, kPoints> pair_count{};
  for (Mask block : expanded_blocks(state)) {
    const auto points = points_of(block);
    for (int i = 0; i < kBlockSize; ++i) {
      for (int j = i + 1; j < kBlockSize; ++j) {
        ++pair_count[points[i]][points[j]];
      }
    }
  }
  int uncovered = 0;
  int square_excess = 0;
  int maximum = 0;
  for (int left = 0; left < kPoints; ++left) {
    for (int right = left + 1; right < kPoints; ++right) {
      const int value = pair_count[left][right];
      if (value == 0) ++uncovered;
      const int excess = std::max(0, value - 1);
      square_excess += excess * excess;
      maximum = std::max(maximum, value);
    }
  }
  return {uncovered, square_excess, maximum};
}

Mask random_block(std::mt19937_64& random) {
  std::array<int, kPoints> points{};
  std::iota(points.begin(), points.end(), 0);
  for (int i = 0; i < kBlockSize; ++i) {
    std::uniform_int_distribution<int> choice(i, kPoints - 1);
    std::swap(points[i], points[choice(random)]);
  }
  Mask result = 0;
  for (int i = 0; i < kBlockSize; ++i) result |= point_bit(points[i]);
  return canonical_orbit_representative(result);
}

State random_state(std::mt19937_64& random, bool balanced) {
  std::uniform_int_distribution<int> choose4(
      0, static_cast<int>(kShort4.size()) - 1);
  std::uniform_int_distribution<int> choose2(
      0, static_cast<int>(kShort2.size()) - 1);
  for (int attempt = 0; attempt < 200000; ++attempt) {
    State state;
    state.short4_index = choose4(random);
    state.short2_index = choose2(random);
    std::set<Mask> used;
    bool okay = true;
    for (Mask& block : state.full) {
      for (int block_attempt = 0; block_attempt < 1000; ++block_attempt) {
        const Mask candidate = random_block(random);
        if (orbit_size(candidate) == 8 && used.insert(candidate).second) {
          block = candidate;
          break;
        }
      }
      if (block == 0) okay = false;
    }
    if (okay && acceptable_replications(state, balanced)) {
      rebuild(state);
      return state;
    }
  }
  throw std::runtime_error("could not initialize a replication-feasible state");
}

bool update_component(State& state, int component_index, Mask new_base,
                      int orbit_length) {
  const Coverage replacement = component_coverage(new_base, orbit_length);
  for (int id = 0; id < kPairClasses; ++id) {
    const int revised = static_cast<int>(state.multiplicity[id]) -
                        state.component[component_index][id] + replacement[id];
    if (revised < 0 || revised > 255) {
      throw std::logic_error("pair multiplicity overflow");
    }
    state.multiplicity[id] = static_cast<std::uint8_t>(revised);
  }
  state.component[component_index] = replacement;
  state.score = score_multiplicities(state.multiplicity);
  return true;
}

Mask random_swap(Mask block, std::mt19937_64& random) {
  auto present = points_of(block);
  std::uniform_int_distribution<int> remove_choice(0, kBlockSize - 1);
  const int removed = present[remove_choice(random)];
  std::uniform_int_distribution<int> add_choice(0, kPoints - kBlockSize - 1);
  int rank = add_choice(random);
  int added = -1;
  for (int point = 0; point < kPoints; ++point) {
    if ((block & point_bit(point)) == 0) {
      if (rank-- == 0) {
        added = point;
        break;
      }
    }
  }
  return canonical_orbit_representative(
      (block & ~point_bit(removed)) | point_bit(added));
}

Mask inject_pair(Mask block, std::pair<int, int> pair,
                 std::mt19937_64& random) {
  std::array<int, 2> required{pair.first, pair.second};
  std::vector<int> missing;
  for (int point : required) {
    if ((block & point_bit(point)) == 0) missing.push_back(point);
  }
  if (missing.empty()) return random_swap(block, random);

  std::vector<int> removable;
  for (int point : points_of(block)) {
    if (point != required[0] && point != required[1]) {
      removable.push_back(point);
    }
  }
  std::shuffle(removable.begin(), removable.end(), random);
  for (std::size_t index = 0; index < missing.size(); ++index) {
    block &= ~point_bit(removable[index]);
    block |= point_bit(missing[index]);
  }
  return canonical_orbit_representative(block);
}

Mask greedy_ruin_block(const State& state, std::mt19937_64& random) {
  std::vector<int> uncovered;
  for (int id = 0; id < kPairClasses; ++id) {
    if (state.multiplicity[id] == 0) uncovered.push_back(id);
  }
  if (uncovered.empty()) return random_block(random);
  std::uniform_int_distribution<int> choose_class(
      0, static_cast<int>(uncovered.size()) - 1);
  const int first_class = uncovered[choose_class(random)];
  const auto& class_members = kPairs.members[first_class];
  std::uniform_int_distribution<int> choose_pair(
      0, static_cast<int>(class_members.size()) - 1);
  auto [first, second] = class_members[choose_pair(random)];
  Mask block = point_bit(first) | point_bit(second);

  while (std::popcount(block) < kBlockSize) {
    int best_gain = -1;
    std::vector<int> choices;
    for (int point = 0; point < kPoints; ++point) {
      if ((block & point_bit(point)) != 0) continue;
      int gain = 0;
      for (int current : points_of(block)) {
        const int id = kPairs.class_of[point][current];
        if (state.multiplicity[id] == 0) {
          gain += kPairs.class_size[id];
        } else if (state.multiplicity[id] == 1) {
          ++gain;
        }
      }
      if (gain > best_gain) {
        best_gain = gain;
        choices.assign(1, point);
      } else if (gain == best_gain) {
        choices.push_back(point);
      }
    }
    std::uniform_int_distribution<int> choose(0,
                                               static_cast<int>(choices.size()) - 1);
    block |= point_bit(choices[choose(random)]);
  }
  return canonical_orbit_representative(block);
}

bool mutate(State& proposal, std::mt19937_64& random, bool balanced) {
  std::uniform_int_distribution<int> percent(0, 99);
  std::uniform_int_distribution<int> choose_full(0, kFullOrbits - 1);
  const int move = percent(random);

  if (move < 54) {
    const int component = choose_full(random);
    Mask replacement;
    if (move < 34) {
      std::vector<int> uncovered;
      for (int id = 0; id < kPairClasses; ++id) {
        if (proposal.multiplicity[id] == 0) uncovered.push_back(id);
      }
      if (uncovered.empty()) return false;
      std::uniform_int_distribution<int> choose_class(
          0, static_cast<int>(uncovered.size()) - 1);
      const auto& members = kPairs.members[uncovered[choose_class(random)]];
      std::uniform_int_distribution<int> choose_pair(
          0, static_cast<int>(members.size()) - 1);
      replacement =
          inject_pair(proposal.full[component], members[choose_pair(random)], random);
    } else {
      replacement = random_swap(proposal.full[component], random);
    }
    if (orbit_size(replacement) != 8) return false;
    for (int other = 0; other < kFullOrbits; ++other) {
      if (other != component && proposal.full[other] == replacement) return false;
    }
    proposal.full[component] = replacement;
    if (!acceptable_replications(proposal, balanced)) return false;
    return update_component(proposal, component, replacement, 8);
  }

  if (move < 70) {
    const int component = choose_full(random);
    const Mask replacement = greedy_ruin_block(proposal, random);
    if (orbit_size(replacement) != 8) return false;
    for (int other = 0; other < kFullOrbits; ++other) {
      if (other != component && proposal.full[other] == replacement) return false;
    }
    proposal.full[component] = replacement;
    if (!acceptable_replications(proposal, balanced)) return false;
    return update_component(proposal, component, replacement, 8);
  }

  if (move < 88) {
    std::uniform_int_distribution<int> choose(
        0, static_cast<int>(kShort4.size()) - 1);
    const int replacement = choose(random);
    if (replacement == proposal.short4_index) return false;
    proposal.short4_index = replacement;
    if (!acceptable_replications(proposal, balanced)) return false;
    return update_component(proposal, kFullOrbits, kShort4[replacement], 4);
  }

  std::uniform_int_distribution<int> choose(
      0, static_cast<int>(kShort2.size()) - 1);
  const int replacement = choose(random);
  if (replacement == proposal.short2_index) return false;
  proposal.short2_index = replacement;
  if (!acceptable_replications(proposal, balanced)) return false;
  return update_component(proposal, kFullOrbits + 1, kShort2[replacement], 2);
}

double energy(const State& state,
              const std::array<int, kPairClasses>& uncovered_weight,
              double variance_weight) {
  double result = variance_weight * state.score.square_excess;
  for (int id = 0; id < kPairClasses; ++id) {
    if (state.multiplicity[id] == 0) result += uncovered_weight[id];
  }
  return result;
}

void write_candidate(const std::string& path, const State& state) {
  if (path.empty()) return;
  std::ofstream output(path);
  if (!output) throw std::runtime_error("cannot write candidate: " + path);
  output << "# Z_8 structured candidate: five length-8 orbits, one length-4, "
            "one length-2\n";
  output << "# uncovered_pairs=" << state.score.uncovered_pairs
         << " square_excess=" << state.score.square_excess
         << " maximum_multiplicity=" << state.score.maximum_multiplicity << "\n";
  for (Mask block : expanded_blocks(state)) {
    const auto points = points_of(block);
    for (int index = 0; index < kBlockSize; ++index) {
      if (index) output << ' ';
      output << points[index] + 1;
    }
    output << '\n';
  }
}

void write_checkpoint(const std::string& path, const State& state,
                      std::uint64_t seed, long long evaluation) {
  if (path.empty()) return;
  std::ofstream output(path);
  if (!output) throw std::runtime_error("cannot write checkpoint: " + path);
  output << "ORBIT8_CHECKPOINT 1\n";
  output << "seed " << seed << "\n";
  output << "evaluation " << evaluation << "\n";
  output << "uncovered_pairs " << state.score.uncovered_pairs << "\n";
  output << "square_excess " << state.score.square_excess << "\n";
  output << "maximum_multiplicity " << state.score.maximum_multiplicity << "\n";
  output << "short4_index " << state.short4_index << "\n";
  output << "short2_index " << state.short2_index << "\n";
  output << std::hex;
  for (Mask block : state.full) output << "full_mask " << block << "\n";
  output << "short4_mask " << kShort4[state.short4_index] << "\n";
  output << "short2_mask " << kShort2[state.short2_index] << "\n";
}

struct Options {
  std::uint64_t seed = 8101;
  long long steps = 1000000;
  int restarts = 4;
  int weight_period = 25000;
  int temperature_cycle = 200000;
  double temperature_start = 16.0;
  double temperature_end = 0.20;
  double variance_weight = 0.025;
  bool balanced = true;
  bool self_test = false;
  std::string output;
  std::string checkpoint;
  std::string log;
};

long long parse_integer(const std::string& value, const std::string& option) {
  std::size_t parsed = 0;
  const long long result = std::stoll(value, &parsed, 10);
  if (parsed != value.size()) throw std::runtime_error("bad value for " + option);
  return result;
}

double parse_real(const std::string& value, const std::string& option) {
  std::size_t parsed = 0;
  const double result = std::stod(value, &parsed);
  if (parsed != value.size()) throw std::runtime_error("bad value for " + option);
  return result;
}

Options parse_options(int argc, char** argv) {
  Options options;
  auto value_after = [&](int& index) -> std::string {
    if (++index >= argc) throw std::runtime_error("missing option value");
    return argv[index];
  };
  for (int index = 1; index < argc; ++index) {
    const std::string argument = argv[index];
    if (argument == "--seed") {
      options.seed = parse_integer(value_after(index), argument);
    } else if (argument == "--steps") {
      options.steps = parse_integer(value_after(index), argument);
    } else if (argument == "--restarts") {
      options.restarts = parse_integer(value_after(index), argument);
    } else if (argument == "--weight-period") {
      options.weight_period = parse_integer(value_after(index), argument);
    } else if (argument == "--temperature-cycle") {
      options.temperature_cycle = parse_integer(value_after(index), argument);
    } else if (argument == "--temperature-start") {
      options.temperature_start = parse_real(value_after(index), argument);
    } else if (argument == "--temperature-end") {
      options.temperature_end = parse_real(value_after(index), argument);
    } else if (argument == "--variance-weight") {
      options.variance_weight = parse_real(value_after(index), argument);
    } else if (argument == "--output") {
      options.output = value_after(index);
    } else if (argument == "--checkpoint") {
      options.checkpoint = value_after(index);
    } else if (argument == "--log") {
      options.log = value_after(index);
    } else if (argument == "--allow-unbalanced") {
      options.balanced = false;
    } else if (argument == "--self-test") {
      options.self_test = true;
    } else if (argument == "--help") {
      std::cout
          << "usage: orbit8_search [options]\n"
          << "  --seed N --steps N --restarts N\n"
          << "  --weight-period N --temperature-cycle N\n"
          << "  --temperature-start X --temperature-end X\n"
          << "  --variance-weight X --allow-unbalanced\n"
          << "  --output FILE --checkpoint FILE --log FILE --self-test\n";
      std::exit(0);
    } else {
      throw std::runtime_error("unknown option: " + argument);
    }
  }
  if (options.steps < 0 || options.restarts <= 0 || options.weight_period <= 0 ||
      options.temperature_cycle <= 0 || options.temperature_start <= 0 ||
      options.temperature_end <= 0 || options.variance_weight < 0) {
    throw std::runtime_error("numeric options are outside their valid ranges");
  }
  return options;
}

void run_self_test() {
  if (kShort2.size() != 30) {
    throw std::logic_error("expected 30 size-2 short block orbits");
  }
  if (kShort4.size() != 2640) {
    throw std::logic_error("expected 2640 size-4 short block orbits");
  }
  for (Mask block : kShort2) {
    if (std::popcount(block) != 8 || orbit_size(block) != 2) {
      throw std::logic_error("malformed size-2 orbit");
    }
    for (int layer = 0; layer < kLayers; ++layer) {
      const Mask layer_mask = ((Mask{1} << 8) - 1) << (8 * layer);
      const int count = std::popcount(block & layer_mask);
      if (count != 0 && count != 4) {
        throw std::logic_error("size-2 base is not a union of parity cosets");
      }
    }
  }
  for (Mask block : kShort4) {
    if (std::popcount(block) != 8 || orbit_size(block) != 4 ||
        shift_mask(block, 4) != block) {
      throw std::logic_error("malformed size-4 orbit");
    }
  }

  std::mt19937_64 random(123456789);
  State state = random_state(random, true);
  if (!exact_structural_validity(state)) {
    throw std::logic_error("random state failed structural validation");
  }
  const Score independent = direct_score(state);
  if (independent.key() != state.score.key()) {
    throw std::logic_error("orbit score disagrees with direct pair enumeration");
  }
  if (expanded_blocks(state).size() != kBlocks) {
    throw std::logic_error("structured candidate does not have 46 blocks");
  }
  const auto replications = layer_replications(state);
  if (std::accumulate(replications.begin(), replications.end(), 0) != kBlocks) {
    throw std::logic_error("layer replication sum is not 46");
  }
  std::cout << "self-test ok: pair_classes=" << kPairClasses
            << " short4_orbits=" << kShort4.size()
            << " short2_orbits=" << kShort2.size()
            << " random_uncovered=" << state.score.uncovered_pairs << "\n";
}

void append_log(const Options& options, const State& best, double seconds,
                long long evaluations) {
  if (options.log.empty()) return;
  bool needs_header = false;
  {
    std::ifstream existing(options.log);
    needs_header = !existing.good() || existing.peek() == std::ifstream::traits_type::eof();
  }
  std::ofstream output(options.log, std::ios::app);
  if (!output) throw std::runtime_error("cannot append log: " + options.log);
  if (needs_header) {
    output << "lane,seed,restarts,steps_per_restart,evaluations,balanced,"
              "weight_period,temp_start,temp_end,temp_cycle,variance_weight,"
              "seconds,best_uncovered,square_excess,max_multiplicity,artifact\n";
  }
  output << "orbit8," << options.seed << ',' << options.restarts << ','
         << options.steps << ',' << evaluations << ','
         << (options.balanced ? "yes" : "no") << ',' << options.weight_period << ','
         << options.temperature_start << ',' << options.temperature_end << ','
         << options.temperature_cycle << ',' << options.variance_weight << ','
         << std::fixed << std::setprecision(3) << seconds << ','
         << best.score.uncovered_pairs << ',' << best.score.square_excess << ','
         << best.score.maximum_multiplicity << ',' << options.output << '\n';
}

int run_search(const Options& options) {
  const auto started = std::chrono::steady_clock::now();
  std::mt19937_64 random(options.seed);
  State global_best;
  global_best.score = {};
  long long evaluations = 0;

  for (int restart = 0; restart < options.restarts; ++restart) {
    State current = random_state(random, options.balanced);
    State restart_best = current;
    std::array<int, kPairClasses> weights = kPairs.class_size;

    if (current.score < global_best.score) {
      global_best = current;
      write_candidate(options.output, global_best);
      write_checkpoint(options.checkpoint, global_best, options.seed, evaluations);
    }

    for (long long step = 0; step < options.steps; ++step) {
      if (step > 0 && step % options.weight_period == 0) {
        for (int id = 0; id < kPairClasses; ++id) {
          if (restart_best.multiplicity[id] == 0) {
            weights[id] += kPairs.class_size[id];
          }
        }
        current = restart_best;
      }

      const long long phase_step = step % options.temperature_cycle;
      const double phase =
          static_cast<double>(phase_step) / options.temperature_cycle;
      const double temperature =
          options.temperature_start *
          std::pow(options.temperature_end / options.temperature_start, phase);

      State proposal = current;
      if (!mutate(proposal, random, options.balanced)) continue;
      ++evaluations;
      const double old_energy =
          energy(current, weights, options.variance_weight);
      const double new_energy =
          energy(proposal, weights, options.variance_weight);
      const double delta = new_energy - old_energy;
      bool accept = delta <= 0;
      if (!accept) {
        std::uniform_real_distribution<double> unit(0.0, 1.0);
        accept = unit(random) < std::exp(-delta / temperature);
      }
      if (accept) current = proposal;

      if (proposal.score < restart_best.score) restart_best = proposal;
      if (proposal.score < global_best.score) {
        global_best = proposal;
        const Score checked = direct_score(global_best);
        if (checked.key() != global_best.score.key() ||
            !exact_structural_validity(global_best)) {
          throw std::logic_error("best state failed exact cross-check");
        }
        write_candidate(options.output, global_best);
        write_checkpoint(options.checkpoint, global_best, options.seed, evaluations);
        std::cerr << "seed=" << options.seed << " restart=" << restart
                  << " step=" << step << " evaluations=" << evaluations
                  << " uncovered=" << global_best.score.uncovered_pairs
                  << " square_excess=" << global_best.score.square_excess
                  << " max_mult=" << global_best.score.maximum_multiplicity << '\n';
        if (global_best.score.uncovered_pairs == 0) break;
      }
    }
    std::cerr << "restart=" << restart
              << " best_uncovered=" << restart_best.score.uncovered_pairs
              << " global_uncovered=" << global_best.score.uncovered_pairs << '\n';
    if (global_best.score.uncovered_pairs == 0) break;
  }

  const double seconds =
      std::chrono::duration<double>(std::chrono::steady_clock::now() - started)
          .count();
  write_candidate(options.output, global_best);
  write_checkpoint(options.checkpoint, global_best, options.seed, evaluations);
  append_log(options, global_best, seconds, evaluations);

  const auto replications = layer_replications(global_best);
  std::cout << "seed=" << options.seed << " evaluations=" << evaluations
            << " seconds=" << std::fixed << std::setprecision(3) << seconds
            << " best_uncovered=" << global_best.score.uncovered_pairs
            << " square_excess=" << global_best.score.square_excess
            << " max_multiplicity=" << global_best.score.maximum_multiplicity
            << " short4_orbits=" << kShort4.size()
            << " short2_orbits=" << kShort2.size() << " replications=";
  for (int layer = 0; layer < kLayers; ++layer) {
    if (layer) std::cout << ':';
    std::cout << replications[layer];
  }
  std::cout << '\n';
  return global_best.score.uncovered_pairs == 0 ? 0 : 1;
}

}  // namespace

int main(int argc, char** argv) {
  try {
    const Options options = parse_options(argc, argv);
    if (options.self_test) {
      run_self_test();
      return 0;
    }
    return run_search(options);
  } catch (const std::exception& error) {
    std::cerr << "error: " << error.what() << '\n';
    return 2;
  }
}
