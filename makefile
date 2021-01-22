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
PROFBIN := voxel_renderer_profile

# libraries 
LIBS := -lm -lSDL2 -lSDL2_gfx

# flags 
FLAGS := 

#------------------------------------------------------------------------------#
# other variables                                                              #
#------------------------------------------------------------------------------#

# commands 
CC := gcc -Wall -Wextra
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

# compile for profiling
compile_profile: $(PROFBIN)

# compile and run project 
run: $(BIN)
	./$<

# compile and profile project 
profile: $(PROFBIN)
	./$<
	gprof $(PROFBIN) gmon.out > profile.output
	gprof2dot profile.output -o profile.dot

# clean project 
clean:
	$(RM) $(BIN)
	$(RM) $(PROFBIN)
	$(RM) $(BUILDDIR)
	$(RM) gmon.out
	$(RM) profile.dot
	$(RM) profile.output

#------------------------------------------------------------------------------#
# build process                                                                #
#------------------------------------------------------------------------------#

$(BIN): $(OBJS)
	$(CC) $(FLAGS) -L $(LIBDIR) $(OBJS) $(LIBS) -o $(BIN)

$(PROFBIN): $(OBJS)
	$(CC) -pg $(FLAGS) -L $(LIBDIR) $(OBJS) $(LIBS) -o $(PROFBIN)

$(BUILDDIR)/%.o: $(SRCDIR)/%.c $(BUILDDIR)
	# $(CC) -pg -c $< -o $@
	$(CC) -c $< -o $@

$(BUILDDIR):
	$(MKDIR) $(BUILDDIR)