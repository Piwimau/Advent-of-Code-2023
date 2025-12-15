CC = gcc
CFLAGS = -std=c23 -Wall -Wextra -Wpedantic -Wundef -Wshadow -Wconversion \
	-Wdouble-promotion -Wcast-align -Wcast-qual -Wlogical-op -Wpointer-arith \
	-Wformat=2 -Wwrite-strings -Wmissing-declarations -Wmissing-include-dirs \
	-Wmissing-prototypes -Wduplicated-cond -Wduplicated-branches
CPPFLAGS = ${INCS} ${DEPFLAGS}
DEPFLAGS = -MMD -MP
LDFLAGS = -Llib
LDLIBS = -lscu${LIB_SUFFIX}

CONFIG ?= debug
ifeq (${CONFIG}, debug)
	CFLAGS += -g3 -Og
	LIB_SUFFIX = d
else ifeq (${CONFIG}, release)
	CFLAGS += -g0 -O3
	CPPFLAGS += -DNDEBUG
	LIB_SUFFIX =
endif

ifdef DAY
	DAYDIR = ${firstword ${wildcard Day-${DAY}-*}}
else
	DAYDIR = ${firstword ${wildcard Day-*}}
endif

ifdef NATIVE
	CFLAGS += -march=native -mtune=native
endif

ifdef V
	Q =
else
	Q = @
endif

SRC = ${DAYDIR}/src
BUILD = ${DAYDIR}/build

INCS = -Iinclude $(if $(wildcard ${DAYDIR}/include), -I${DAYDIR}/include)
SRCS = ${shell find ${SRC} -type f -name '*.c'}
OBJS = ${patsubst ${SRC}/%.c, ${BUILD}/${CONFIG}/%.o, ${SRCS}}
DEPS = ${patsubst ${SRC}/%.c, ${BUILD}/${CONFIG}/%.d, ${SRCS}}

BIN = ${BUILD}/${CONFIG}/main.exe

.PHONY: all run clean help

all: ${BIN}

run: all
	${Q}${BIN} < ${DAYDIR}/resources/input.txt

${BIN}: ${OBJS}
	${Q}mkdir -p ${dir $@}
	${Q}${CC} $^ ${LDFLAGS} ${LDLIBS} -o $@

${BUILD}/${CONFIG}/%.o: ${SRC}/%.c
	${Q}mkdir -p ${dir $@}
	${Q}${CC} ${CFLAGS} ${CPPFLAGS} -c $< -o $@

clean:
	${Q}rm -rf ${BUILD}

help:
	@echo "Usage: make [TARGET]... [VARIABLE]..."
	@echo ""
	@echo "Targets:"
	@echo "  all    Build the selected day (default)."
	@echo "  run    Build and run the selected day."
	@echo "  clean  Remove all build artifacts of the selected day."
	@echo "  help   Display this help and exit."
	@echo ""
	@echo "Variables:"
	@echo "  CONFIG={debug|release}  Set the build configuration (default: debug)."
	@echo "  DAY=N                   Select the day by its two-digit number N (default: first day found)."
	@echo "  NATIVE=1                Enable machine-specific optimizations."
	@echo "  V                       Enable verbose build output."

-include ${DEPS}