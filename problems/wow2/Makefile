CC ?= cc
CFLAGS ?= -std=c17 -O3 -Wall -Wextra -Wpedantic -Werror
BUILD_DIR ?= /tmp/math-eval-wow2

COMMON = graph.c graph.h conjectures.c conjectures.h
REDUCTION61_COMMON = reduction61.c reduction61.h
GENERATOR = $(BUILD_DIR)/wow2-generate
SWEEPER = $(BUILD_DIR)/wow2-sweep
SEARCH = $(BUILD_DIR)/wow2-search
PROOF61 = $(BUILD_DIR)/wow2-proof61
MAXINE61 = $(BUILD_DIR)/wow2-maxine61
REDUCTION_SEARCH61 = $(BUILD_DIR)/wow2-reduction-search61
FINISH61 = $(BUILD_DIR)/wow2-finish61

.PHONY: all test check-counts-9 exhaustive-61 exhaustive-59 families-61 \
	check-critical-61 check-maxine-61 check-finish-61 clean

all: $(GENERATOR) $(SWEEPER) $(SEARCH) $(PROOF61) $(MAXINE61) \
	$(REDUCTION_SEARCH61) $(FINISH61)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(GENERATOR): graph.c graph.h generate.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) graph.c generate.c -o $@

$(SWEEPER): $(COMMON) sweep.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) graph.c conjectures.c sweep.c -o $@

$(SEARCH): $(COMMON) search.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) graph.c conjectures.c search.c -o $@

$(PROOF61): $(COMMON) $(REDUCTION61_COMMON) proof61.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) graph.c conjectures.c reduction61.c proof61.c -o $@

$(MAXINE61): graph.c graph.h maxine61.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) graph.c maxine61.c -o $@

$(REDUCTION_SEARCH61): graph.c graph.h $(REDUCTION61_COMMON) \
		reduction_search61.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) graph.c reduction61.c reduction_search61.c -o $@

$(FINISH61): graph.c graph.h $(REDUCTION61_COMMON) finish61.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) graph.c reduction61.c finish61.c -o $@

test: all
	$(GENERATOR) --self-test
	$(SWEEPER) --self-test
	$(PROOF61) --self-test
	$(GENERATOR) 6 | $(MAXINE61)
	$(REDUCTION_SEARCH61) --self-test
	$(FINISH61) --self-test
	python3 verify.py --self-test
	python3 critical61.py --self-test

check-counts-9: $(GENERATOR)
	$(GENERATOR) --count-only 9

exhaustive-61: $(GENERATOR) $(SWEEPER)
	$(GENERATOR) 9 | $(SWEEPER) --mode 61 --scan

exhaustive-59: $(GENERATOR) $(SWEEPER)
	$(GENERATOR) 9 | $(SWEEPER) --mode 59 --scan

families-61: $(SWEEPER)
	python3 families.py --max-vertices 20 | $(SWEEPER) --mode 61 --scan

check-critical-61: $(GENERATOR) $(PROOF61)
	$(GENERATOR) 9 | $(PROOF61) --scan --expect-critical 196 \
		--expect-induction-critical 0

check-maxine-61: $(GENERATOR) $(MAXINE61)
	$(GENERATOR) 9 | $(MAXINE61)

check-finish-61: $(GENERATOR) $(FINISH61)
	$(GENERATOR) 9 | $(FINISH61)

clean:
	rm -f $(GENERATOR) $(SWEEPER) $(SEARCH) $(PROOF61) $(MAXINE61) \
		$(REDUCTION_SEARCH61)
