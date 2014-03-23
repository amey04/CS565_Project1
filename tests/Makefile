# Makefile for tests; generates unoptimized .ll IR for all tests

# Test files
FILES = test1.bc test2.bc test3.bc test4.bc

##
LLVM_ROOT = ../../..
CLANG = $(LLVM_ROOT)/bin/clang
CLANG_FLAGS = -emit-llvm -O0
DIS = $(LLVM_ROOT)/bin/llvm-dis
LLC = $(LLVM_ROOT)/bin/llc

all: $(FILES)

clean:
	rm -f *.bc *.ll *.s $(FILES:%.bc=%)

%.bc: %.c
	$(CLANG) $(CLANG_FLAGS) -c $(@:%.bc=%.c) -o $@
	$(DIS) -f $@
	$(LLC) $@
	gcc $(@:%.bc=%.s) -o $(@:%.bc=%)
