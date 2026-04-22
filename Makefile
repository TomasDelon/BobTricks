CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -g
INCLUDES := -Isrc -Ivendor/imgui $(shell sdl2-config --cflags) $(shell pkg-config --cflags SDL2_mixer)
LIBS     := $(shell sdl2-config --libs) $(shell pkg-config --libs SDL2_mixer) -lGL

SRC_DIRS := src src/app src/config \
            src/core/math src/core/character src/core/locomotion \
            src/core/runtime src/core/simulation src/core/telemetry src/core/terrain \
            src/render src/debug

SRCS     := $(wildcard $(addsuffix /*.cpp, $(SRC_DIRS)))
IMGUI    := vendor/imgui/imgui.cpp \
            vendor/imgui/imgui_draw.cpp \
            vendor/imgui/imgui_tables.cpp \
            vendor/imgui/imgui_widgets.cpp \
            vendor/imgui/imgui_impl_sdl2.cpp \
            vendor/imgui/imgui_impl_sdlrenderer2.cpp

ALL_SRCS := $(SRCS) $(IMGUI)
OBJS     := $(patsubst src/%.cpp,     build/%.o,        $(SRCS)) \
            $(patsubst vendor/%.cpp,  build/vendor/%.o, $(IMGUI))
TARGET   := build/bobtricks_v4

# ── Headless binary ───────────────────────────────────────────────────────────
# No SDL, no ImGui, no Camera2D.  Links only core + config + headless.
HEADLESS_DIRS := src/config \
                 src/core/character src/core/locomotion \
                 src/core/simulation src/core/telemetry src/core/terrain \
                 src/headless
HEADLESS_SRCS := $(wildcard $(addsuffix /*.cpp, $(HEADLESS_DIRS)))
HEADLESS_BIN  := build/bobtricks_headless

TEST_CORE_SRCS := src/core/character/CharacterState.cpp \
                  src/core/character/ArmController.cpp \
                  src/core/character/HeadController.cpp \
                  src/core/locomotion/BalanceComputer.cpp \
                  src/core/locomotion/LegIK.cpp \
                  src/core/locomotion/StandingController.cpp \
                  src/core/simulation/GroundReference.cpp \
                  src/core/simulation/SimVerbosity.cpp \
                  src/core/simulation/SimulationCore.cpp \
                  src/core/simulation/SimulationCoreLifecycle.cpp \
                  src/core/simulation/SimulationCoreLocomotion.cpp \
                  src/core/telemetry/TelemetryRecorder.cpp \
                  src/core/terrain/Terrain.cpp
UNIT_TEST_BIN := build/tests/unit/core_unit_tests
REGRESSION_TEST_BIN := build/tests/regression/headless_regression_tests

# ── ASan/UBSan binary (headless only) ────────────────────────────────────────
ASAN_BIN := build/bobtricks_headless_asan
ARCH_CHECK := scripts/check_architecture.sh

build_asan: $(ASAN_BIN)

$(ASAN_BIN): $(HEADLESS_SRCS)
	@mkdir -p $(@D)
	$(CXX) -std=c++20 -Wall -Wextra -g -fsanitize=address,undefined -Isrc $^ -lm -o $@
	@echo "ASan build OK → $(ASAN_BIN)"

test_asan: $(ASAN_BIN)
	@$(ASAN_BIN) --all --quiet

# ── Memory check (Valgrind, headless only) ────────────────────────────────────
test_mem: $(HEADLESS_BIN)
	valgrind --leak-check=full --error-exitcode=1 $(HEADLESS_BIN) --all --quiet

.PHONY: all build build_headless build_asan run test test_headless test_unit \
        test_regression test_asan test_mem check_architecture docs clean help FORCE

all: build

build: $(TARGET)
	@echo "make pass"

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@
	@echo "Build OK → $(TARGET)"

build/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

build/vendor/%.o: vendor/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d)

run: build
	./$(TARGET)

build_headless: $(HEADLESS_BIN)
	@echo "make pass"

test: test_unit test_regression test_headless
	@echo "make pass"

test_headless: $(HEADLESS_BIN)
	@$(HEADLESS_BIN) --all --quiet

test_unit: $(UNIT_TEST_BIN)
	@./$(UNIT_TEST_BIN)

test_regression: $(REGRESSION_TEST_BIN)
	@./$(REGRESSION_TEST_BIN)

check_architecture:
	@./$(ARCH_CHECK)

$(HEADLESS_BIN): $(HEADLESS_SRCS)
	@mkdir -p $(@D)
	$(CXX) -std=c++20 -Wall -Wextra -O2 -Isrc $^ -lm -o $@
	@echo "Headless OK → $(HEADLESS_BIN)"

$(UNIT_TEST_BIN): tests/unit/test_core_math.cpp tests/TestSupport.h \
                  src/core/math/Bezier.cpp \
                  src/core/math/StrokePath.cpp \
                  src/core/character/ArmController.cpp \
                  src/core/character/CharacterState.cpp \
                  src/core/character/HeadController.cpp \
                  src/core/locomotion/BalanceComputer.cpp \
                  src/core/locomotion/LegIK.cpp \
                  src/core/locomotion/StandingController.cpp \
                  src/core/terrain/Terrain.cpp
	@mkdir -p $(@D)
	$(CXX) -std=c++20 -Wall -Wextra -O2 -Isrc -I. \
	    tests/unit/test_core_math.cpp \
	    src/core/math/Bezier.cpp \
	    src/core/math/StrokePath.cpp \
	    src/core/character/ArmController.cpp \
	    src/core/character/CharacterState.cpp \
	    src/core/character/HeadController.cpp \
	    src/core/locomotion/BalanceComputer.cpp \
	    src/core/locomotion/LegIK.cpp \
	    src/core/locomotion/StandingController.cpp \
	    src/core/terrain/Terrain.cpp -lm -o $@

$(REGRESSION_TEST_BIN): tests/regression/test_headless_scenarios.cpp \
                        tests/TestSupport.h src/headless/ScenarioLibrary.cpp \
                        src/headless/ScenarioRunner.cpp $(TEST_CORE_SRCS)
	@mkdir -p $(@D)
	$(CXX) -std=c++20 -Wall -Wextra -O2 -Isrc -I. \
	    tests/regression/test_headless_scenarios.cpp \
	    src/headless/ScenarioLibrary.cpp src/headless/ScenarioRunner.cpp \
	    $(TEST_CORE_SRCS) -lm -o $@

# ── Documentation Doxygen ─────────────────────────────────────────────────────
docs:
	doxygen Doxyfile
	@echo "Docs OK → doc/doxygen/html/index.html"

clean:
	rm -rf build/

help:
	@echo "make build                   — compile SDL app"
	@echo "make run                     — compile + run SDL app"
	@echo "make build_headless          — compile headless binary (no SDL)"
	@echo "make test                    — run unit + regression + headless scenario tests"
	@echo "make test_unit               — run low-level unit tests"
	@echo "make test_regression         — run regression tests on named scenarios"
	@echo "make test_headless           — run all headless scenarios"
	@echo "make build_asan              — compile headless with ASan + UBSan"
	@echo "make test_asan               — build_asan + run all scenarios"
	@echo "make test_mem                — Valgrind on headless binary"
	@echo "make check_architecture      — verify repo invariants (core SDL-free, etc.)"
	@echo "make docs                    — générer la doc Doxygen dans doc/doxygen/html/"
	@echo "make clean                   — remove build/"
