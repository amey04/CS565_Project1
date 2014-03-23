# Makefile for hello pass

# Path to top level of LLVM hierarchy
LEVEL = ../..

# Name of the library to build
LIBRARYNAME = cs565opt

# Make the shared library become a loadable module so the tools can
# dlopen/dlsym on the resulting library.
LOADABLE_MODULE = 1

# Might need to change this to .so in LINUX
LIB_EXT = .dylib

TEST_PRINT = $(LEVEL)/../bin/opt -load $(LEVEL)/../lib/$(LIBRARYNAME)$(LIB_EXT) -prettyPrint <
TEST_NAIVE = $(LEVEL)/../bin/opt -load $(LEVEL)/../lib/$(LIBRARYNAME)$(LIB_EXT) -cfgNaive < 

# Include the makefile implementation stuff
include $(LEVEL)/Makefile.common

testPrint:
	$(TEST_PRINT) ../tests/test1.bc > /dev/null

testDead:
	$(TEST_PRINT) ../tests/test2.opt.bc > /dev/null


testNaive: 
	$(TEST_PRINT) ../tests/test2.bc > /dev/null
	$(TEST_NAIVE) ../tests/test2.bc > ../tests/test2.opt.bc
	$(TEST_PRINT) ../tests/test2.opt.bc > /dev/null

testNaive4: 
	@$(TEST_PRINT) ../tests/test4.bc > /dev/null
	@$(TEST_NAIVE) ../tests/test4.bc > ../tests/test4.opt.bc
	@$(TEST_PRINT) ../tests/test4.opt.bc > /dev/null

tar:
	tar cvf Project1_LLvm.tar Makefile *.cpp *.h
