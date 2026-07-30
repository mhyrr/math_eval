PYTHON ?= python3
CXX ?= c++
BUILD_DIR ?= /tmp/math-eval-covering-48-8-2
CXXFLAGS ?= -O3 -Wall -Wextra -Wpedantic

.PHONY: test python-test cpp-test verify-best clean-room-48

test: python-test cpp-test verify-best

python-test:
	$(PYTHON) -m unittest discover -v

cpp-test:
	mkdir -p $(BUILD_DIR)
	$(CXX) -std=c++17 $(CXXFLAGS) local_search.cpp -o $(BUILD_DIR)/local_search
	$(CXX) -std=c++17 $(CXXFLAGS) block_search.cpp -o $(BUILD_DIR)/block_search
	$(CXX) -std=c++20 $(CXXFLAGS) orbit8_search.cpp -o $(BUILD_DIR)/orbit8_search
	$(BUILD_DIR)/local_search --self-test
	$(BUILD_DIR)/block_search --self-test
	$(BUILD_DIR)/orbit8_search --self-test

verify-best:
	$(PYTHON) verifier.py structured_artifacts/structured_cyclic_48_seed480802_valid_5886a11e9550990b.txt
	$(PYTHON) independent_verify.py structured_artifacts/structured_cyclic_48_seed480802_valid_5886a11e9550990b.txt

clean-room-48:
	$(PYTHON) structured_search.py --mode cyclic --blocks 48 --seed 480802 \
		--cyclic-evaluations 20000 --output-dir $(BUILD_DIR)/reproduction --json
