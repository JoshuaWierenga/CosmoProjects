#-*-mode:makefile-gmake;indent-tabs-mode:t;tab-width:8;coding:utf-8-*-┐
#───vi: set et ft=make ts=8 tw=8 fenc=utf-8 :vi───────────────────────┘

PKGS += THIRD_PARTY_XASM

THIRD_PARTY_XASM_ARTIFACTS += THIRD_PARTY_XASM_A
THIRD_PARTY_XASM = $(THIRD_PARTY_XASM_A_DEPS) $(THIRD_PARTY_XASM_A)
THIRD_PARTY_XASM_A = o/$(MODE)/third_party/xasm/xasm.a
THIRD_PARTY_XASM_A_FILES :=                         \
    $(wildcard third_party/xasm/*)
THIRD_PARTY_XASM_A_HDRS = $(filter %.h,$(THIRD_PARTY_XASM_A_FILES))
THIRD_PARTY_XASM_A_SRCS = $(filter %.c,$(THIRD_PARTY_XASM_A_FILES))
THIRD_PARTY_XASM_A_OBJS = $(THIRD_PARTY_XASM_A_SRCS:%.c=o/$(MODE)/%.o)

THIRD_PARTY_XASM_A_DIRECTDEPS =                     \
        LIBC_NEXGEN32E                              \
        LIBC_STDIO                                  \
        LIBC_STR                                    \
        THIRD_PARTY_GETOPT

THIRD_PARTY_XASM_A_DEPS :=                          \
        $(call uniq,$(foreach x,$(THIRD_PARTY_XASM_A_DIRECTDEPS),$($(x))))

THIRD_PARTY_XASM_A_CHECKS =                         \
        $(THIRD_PARTY_XASM_A).pkg

$(THIRD_PARTY_XASM_A):                              \
        third_party/xasm/                           \
        $(THIRD_PARTY_XASM_A).pkg                   \
        $(THIRD_PARTY_XASM_A_OBJS)

$(THIRD_PARTY_XASM_A).pkg:                                               	\
        $(THIRD_PARTY_XASM_A_OBJS)                                      	\
        $(foreach x,$(THIRD_PARTY_XASM_A_DIRECTDEPS),$($(x)_A).pkg)

o/$(MODE)/third_party/xasm/xasm.com.dbg:            \
        $(THIRD_PARTY_XASM)                         \
        o/$(MODE)/third_party/xasm/xasm.o           \
        $(CRT)                                      \
        $(APE_NO_MODIFY_SELF)
	@$(APELINK)

THIRD_PARTY_XASM_COMS = o/$(MODE)/third_party/xasm/xasm.com
THIRD_PARTY_XASM_BINS = $(THIRD_PARTY_XASM_COMS) $(THIRD_PARTY_XASM_COMS:%=%.dbg)
THIRD_PARTY_XASM_LIBS = $(foreach x,$(THIRD_PARTY_XASM_ARTIFACTS),$($(x)))
THIRD_PARTY_XASM_SRCS = $(foreach x,$(THIRD_PARTY_XASM_ARTIFACTS),$($(x)_SRCS))
THIRD_PARTY_XASM_HDRS = $(foreach x,$(THIRD_PARTY_XASM_ARTIFACTS),$($(x)_HDRS))
THIRD_PARTY_XASM_CHECKS = $(foreach x,$(THIRD_PARTY_XASM_ARTIFACTS),$($(x)_CHECKS))
THIRD_PARTY_XASM_OBJS = $(foreach x,$(THIRD_PARTY_XASM_ARTIFACTS),$($(x)_OBJS))
$(THIRD_PARTY_XASM_OBJS): third_party/xasm/xasm.mk

.PHONY: o/$(MODE)/third_party/xasm
o/$(MODE)/third_party/xasm:                         \
		$(THIRD_PARTY_XASM_BINS)                    \
		$(THIRD_PARTY_XASM_CHECKS)
