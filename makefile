#==============================================================================#
# basic makefile for binary creation                                           #
#==============================================================================#

#------------------------------------------------------------------------------#
# basic projec structure                                                       #
#------------------------------------------------------------------------------#

# project
# ├── src
# │   ├── a.c
# │   ├── b.c
# │   └── etc...
# ├── include
# │   ├── a.h
# │   ├── b.h
# │   └── etc...
# ├── lib
# │   ├── lib_a.a
# │   ├── lib_b.so
# │   └── etc...
# ├── makefile
# ├── binary
# └── etc...

#------------------------------------------------------------------------------#
# important variables                                                          #
#------------------------------------------------------------------------------#

# binary name 
BIN := voxel_renderer

# libraries 
LIBS := -lm -lSDL2 -lSDL2_gfx

# flags 
FLAGS := -Wall -Wextra

#------------------------------------------------------------------------------#
# other variables                                                              #
#------------------------------------------------------------------------------#

# commands 
CC := gcc
MV := mv
RM := rm -rf
CP := cp
MKDIR := mkdir
ECHO := @echo

# folders 
BUILDDIR := build
SRCDIR := src
LIBDIR := lib

# source files 
SRCS := $(wildcard $(SRCDIR)/*.c)
OBJS := $(patsubst $(SRCDIR)/%.c,$(BUILDDIR)/%.o,$(SRCS))

#------------------------------------------------------------------------------#
# main rules                                                                   #
#------------------------------------------------------------------------------#

.PHONY: all run clean

# compile project (default) 
compile: $(BIN)

# compile and run project 
run: $(BIN)
	./$<

# compile for profiling
compile_profile: FLAGS += -pg
compile_profile: $(BIN)

# compile and profile project
profile: FLAGS += -pg
profile: $(BIN)
	./$<
	gprof $(BIN) gmon.out > profile.output
	gprof2dot profile.output -o profile.dot

# clean project 
clean:
	$(RM) $(BIN)
	$(RM) $(BUILDDIR)
	$(RM) gmon.out
	$(RM) profile.dot
	$(RM) profile.output

#------------------------------------------------------------------------------#
# build process                                                                #
#------------------------------------------------------------------------------#

$(BIN): $(OBJS)
	$(CC) $(FLAGS) -L $(LIBDIR) $(OBJS) $(LIBS) -o $(BIN)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR)
	$(CC) $(FLAGS) -c $< -o $@

$(BUILDDIR):
	$(MKDIR) $(BUILDDIR)