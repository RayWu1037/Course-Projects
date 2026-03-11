CXX = clang++
# Unused: warn, but annoying to block compilation on
# Sign compare: noisy
# Command line arg: noisy, not relevant to students
CXXFLAGS = \
	-Wall -Wextra -Werror \
	-Wno-error=unused-function \
	-Wno-error=unused-parameter \
	-Wno-error=unused-variable \
	-Wno-error=unused-but-set-variable \
	-Wno-error=unused-value \
	-Wno-sign-compare \
	-Wno-unused-command-line-argument \
	-std=c++2a -I. -g -fno-omit-frame-pointer \
	-fsanitize=address,undefined

# On Ubuntu and WSL, googletest is installed to /usr/include or
# /usr/local/include, which are used by default.

# On Mac, we need to manually include them in our path. Brew installs to
# different locations on Intel/Silicon, so ask brew where things live.
UNAME := $(shell uname)
ifeq ($(UNAME), Darwin)
	GTEST_PREFIX := $(shell brew --prefix googletest)
	LLVM_PREFIX := $(shell brew --prefix llvm)
	CXX := $(LLVM_PREFIX)/bin/clang++
	CXXFLAGS += -I$(GTEST_PREFIX)/include
	CXXFLAGS += -L$(GTEST_PREFIX)/lib
	CXXFLAGS += -L$(LLVM_PREFIX)/lib/c++
	CXXFLAGS += -Wno-character-conversion
endif

ENV_VARS = \
	ASAN_OPTIONS=abort_on_error=1:detect_leaks=1 \
	LSAN_OPTIONS=suppressions=suppr.txt:print_suppressions=false

# Test content won't change, so saving object files helps a lot with
# googletest macro processing time
build/%.o: tests/%.cpp
	mkdir -p build && $(CXX) $(CXXFLAGS) -c $< -o $@

TEST_NAMES := $(basename $(notdir $(wildcard tests/*.cpp)))
TEST_OBJS := $(addprefix build/,$(addsuffix .o,$(TEST_NAMES)))

# All public tests: ours and the students'
lyrics_tests: $(TEST_OBJS) lyrics_tests.cpp lyrics.cpp
	$(CXX) $(CXXFLAGS) $^ -lgtest -lgmock -lgtest_main -o $@

test_clean_token: lyrics_tests
	$(ENV_VARS) ./$< --gtest_color=yes --gtest_filter="CleanToken*"

test_gather_tokens: lyrics_tests
	$(ENV_VARS) ./$< --gtest_color=yes --gtest_filter="GatherTokens*"

test_build_indices: lyrics_tests
	$(ENV_VARS) ./$< --gtest_color=yes --gtest_filter="BuildIndices*"

test_find_query_matches: lyrics_tests
	$(ENV_VARS) ./$< --gtest_color=yes --gtest_filter="FindQueryMatches*"

test_search_engine: lyrics_tests
	$(ENV_VARS) ./$< --gtest_color=yes --gtest_filter="SearchEngine*"

test_all: lyrics_tests
	$(ENV_VARS) ./$< --gtest_color=yes

lyrics_main: lyrics.cpp lyrics_main.cpp
	$(CXX) $(CXXFLAGS) $^ -o $@

run_lyrics: lyrics_main
	$(ENV_VARS) ./$<

clean:
	rm -f lyrics_tests lyrics_main build/*
	# MacOS symbol cleanup
	rm -rf *.dSYM

.PHONY: clean run_lyrics test_all test_clean_token test_gather_tokens test_build_indices test_find_query_matches test_search_engine test_all
