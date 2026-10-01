# host tests, for environments that have gcc or clang. on the windows machine this was developed
# on there is no unix toolchain, so build.ps1 does the same thing with MSVC, and the
# motorecopico_tests target runs the very same tests on a Pico.
#
#   make            build and run the tests

CC      ?= cc
CFLAGS  ?= -std=c11 -Wall -Wextra -O1 -I../src
BUILD   := build

# the modules with no hardware in them. keep in step with the motorecopico_tests target
# in ../CMakeLists.txt
TEST_SRC := test_nmea.c ../src/nmea.c ../src/record.c ../src/power.c ../src/settings.c \
            ../src/wallclock.c

.PHONY: all test clean

all: test

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/test_nmea: $(BUILD) $(TEST_SRC)
	$(CC) $(CFLAGS) -o $@ $(TEST_SRC) -lm

test: $(BUILD)/test_nmea
	$(BUILD)/test_nmea

clean:
	rm -rf $(BUILD)
