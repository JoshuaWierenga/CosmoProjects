#-*-mode:makefile-gmake;indent-tabs-mode:t;tab-width:8;coding:utf-8-*-┐
#───vi: set et ft=make ts=8 tw=8 fenc=utf-8 :vi───────────────────────┘

PKGS += THIRD_PARTY_AOC

THIRD_PARTY_AOC_ARTIFACTS += THIRD_PARTY_AOC_A
THIRD_PARTY_AOC = $(THIRD_PARTY_AOC_A_DEPS) $(THIRD_PARTY_AOC_A)
THIRD_PARTY_AOC_A = o/$(MODE)/third_party/aoc/aoc.a
THIRD_PARTY_AOC_A_FILES :=                      \
    $(wildcard third_party/aoc/*)               \
    third_party/aoc/utilities/hashmap/src/cmap.c
THIRD_PARTY_AOC_A_HDRS = $(filter %.h,$(THIRD_PARTY_AOC_A_FILES))
THIRD_PARTY_AOC_A_SRCS = $(filter %.c,$(THIRD_PARTY_AOC_A_FILES))
THIRD_PARTY_AOC_A_OBJS = $(THIRD_PARTY_AOC_A_SRCS:%.c=o/$(MODE)/%.o)

THIRD_PARTY_AOC_A_DIRECTDEPS =							\
        LIBC_CALLS                          \
        LIBC_FMT                            \
        LIBC_INTRIN                         \
        LIBC_LOG                            \
        LIBC_MEM                            \
        LIBC_NEXGEN32E                      \
        LIBC_RUNTIME                        \
        LIBC_STDIO                          \
        LIBC_STR                            \
        LIBC_SYSV                           \
        LIBC_X

THIRD_PARTY_AOC_A_DEPS :=							\
        $(call uniq,$(foreach x,$(THIRD_PARTY_AOC_A_DIRECTDEPS),$($(x))))

THIRD_PARTY_AOC_A_CHECKS = 							\
        $(THIRD_PARTY_AOC_A).pkg

$(THIRD_PARTY_AOC_A):            					     	\
        	third_party/aoc/                				\
        	$(THIRD_PARTY_AOC_A).pkg        				\
        	$(THIRD_PARTY_AOC_A_OBJS)

$(THIRD_PARTY_AOC_A).pkg:                                               	\
        	$(THIRD_PARTY_AOC_A_OBJS)                                      	\
        	$(foreach x,$(THIRD_PARTY_AOC_A_DIRECTDEPS),$($(x)_A).pkg)

o/$(MODE)/third_party/aoc/aoc.com.dbg:						\
		$(THIRD_PARTY_AOC)						\
		o/$(MODE)/third_party/aoc/aoc.o					\
		$(CRT)								\
		$(APE_NO_MODIFY_SELF)
	@$(APELINK)

THIRD_PARTY_AOC_COMS = o/$(MODE)/third_party/aoc/aoc.com
THIRD_PARTY_AOC_BINS = $(THIRD_PARTY_AOC_COMS) $(THIRD_PARTY_AOC_COMS:%=%.dbg)
THIRD_PARTY_AOC_LIBS = $(foreach x,$(THIRD_PARTY_AOC_ARTIFACTS),$($(x)))
THIRD_PARTY_AOC_SRCS = $(foreach x,$(THIRD_PARTY_AOC_ARTIFACTS),$($(x)_SRCS))
THIRD_PARTY_AOC_HDRS = $(foreach x,$(THIRD_PARTY_AOC_ARTIFACTS),$($(x)_HDRS))
THIRD_PARTY_AOC_CHECKS = $(foreach x,$(THIRD_PARTY_AOC_ARTIFACTS),$($(x)_CHECKS))
THIRD_PARTY_AOC_OBJS = $(foreach x,$(THIRD_PARTY_AOC_ARTIFACTS),$($(x)_OBJS))
$(THIRD_PARTY_AOC_OBJS): third_party/aoc/aoc.mk

.PHONY: o/$(MODE)/third_party/aoc
o/$(MODE)/third_party/aoc: 							\
		$(THIRD_PARTY_AOC_BINS)						\
		$(THIRD_PARTY_AOC_CHECKS)
