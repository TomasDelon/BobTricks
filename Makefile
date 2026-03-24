CXX      := g++
CXXFLAGS := -std=c++20 -Wall -Wextra -g
INCLUDES := -Isrc -Ithird_party/imgui $(shell sdl2-config --cflags)
LIBS     := $(shell sdl2-config --libs) -lGL

SRC_DIRS := src src/app src/config \
            src/core/math src/core/character src/core/locomotion \
            src/core/runtime src/core/simulation src/core/telemetry src/core/terrain \
            src/render src/debug

SRCS     := $(wildcard $(addsuffix /*.cpp, $(SRC_DIRS)))
IMGUI    := third_party/imgui/imgui.cpp \
            third_party/imgui/imgui_draw.cpp \
            third_party/imgui/imgui_tables.cpp \
            third_party/imgui/imgui_widgets.cpp \
            third_party/imgui/imgui_impl_sdl2.cpp \
            third_party/imgui/imgui_impl_sdlrenderer2.cpp

ALL_SRCS := $(SRCS) $(IMGUI)
OBJS     := $(patsubst src/%.cpp,          build/%.o,             $(SRCS)) \
            $(patsubst third_party/%.cpp,  build/third_party/%.o, $(IMGUI))
TARGET   := build/bobtricks_v4

# ── Headless binary ───────────────────────────────────────────────────────────
# No SDL, no ImGui, no Camera2D.  Links only core + config + headless.
HEADLESS_DIRS := src/config \
                 src/core/character src/core/locomotion \
                 src/core/simulation src/core/telemetry src/core/terrain \
                 src/headless
HEADLESS_SRCS := $(wildcard $(addsuffix /*.cpp, $(HEADLESS_DIRS)))
HEADLESS_BIN  := build/bobtricks_headless

# ── ASan/UBSan binary (headless only) ────────────────────────────────────────
ASAN_BIN := build/bobtricks_headless_asan

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

.PHONY: all build build_headless build_asan run test test_asan test_mem clean help

all: build

build: $(TARGET)

$(TARGET): $(OBJS)
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $^ $(LIBS) -o $@
	@echo "Build OK → $(TARGET)"

build/%.o: src/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

build/third_party/%.o: third_party/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -MMD -MP -c $< -o $@

-include $(OBJS:.o=.d)

run: build
	./$(TARGET)

build_headless: $(HEADLESS_BIN)

test: $(HEADLESS_BIN)
	@$(HEADLESS_BIN) --all --quiet

$(HEADLESS_BIN): $(HEADLESS_SRCS)
	@mkdir -p $(@D)
	$(CXX) -std=c++20 -Wall -Wextra -O2 -Isrc $^ -lm -o $@
	@echo "Headless OK → $(HEADLESS_BIN)"

# ── Headless analysis tools ───────────────────────────────────────────────────
# No SDL, no ImGui. Shares headers from src/ via -Isrc.
# Usage:  make analysis/test_ip_dynamics  &&  ./analysis/test_ip_dynamics > out.csv
analysis/%: analysis/%.cpp
	$(CXX) -std=c++20 -O2 -Isrc $< -lm -o $@

clean:
	rm -rf build/

help:
	@echo "make build                   — compile SDL app"
	@echo "make run                     — compile + run SDL app"
	@echo "make build_headless          — compile headless binary (no SDL)"
	@echo "make test                    — run all scenarios, exit 0 if all PASS"
	@echo "make build_asan              — compile headless with ASan + UBSan"
	@echo "make test_asan               — build_asan + run all scenarios"
	@echo "make test_mem                — Valgrind on headless binary"
	@echo "make analysis/<name>         — compile headless analysis tool"
	@echo "make clean                   — remove build/"
