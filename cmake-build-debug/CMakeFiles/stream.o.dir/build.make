# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.16

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/mac/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/navintamilchelvampro/c/ftp_protocol

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/navintamilchelvampro/c/ftp_protocol/cmake-build-debug

# Include any dependencies generated for this target.
include CMakeFiles/stream.o.dir/depend.make

# Include the progress variables for this target.
include CMakeFiles/stream.o.dir/progress.make

# Include the compile flags for this target's objects.
include CMakeFiles/stream.o.dir/flags.make

CMakeFiles/stream.o.dir/stream.c.o: CMakeFiles/stream.o.dir/flags.make
CMakeFiles/stream.o.dir/stream.c.o: ../stream.c
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/Users/navintamilchelvampro/c/ftp_protocol/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building C object CMakeFiles/stream.o.dir/stream.c.o"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -o CMakeFiles/stream.o.dir/stream.c.o   -c /Users/navintamilchelvampro/c/ftp_protocol/stream.c

CMakeFiles/stream.o.dir/stream.c.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing C source to CMakeFiles/stream.o.dir/stream.c.i"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -E /Users/navintamilchelvampro/c/ftp_protocol/stream.c > CMakeFiles/stream.o.dir/stream.c.i

CMakeFiles/stream.o.dir/stream.c.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling C source to assembly CMakeFiles/stream.o.dir/stream.c.s"
	/Library/Developer/CommandLineTools/usr/bin/cc $(C_DEFINES) $(C_INCLUDES) $(C_FLAGS) -S /Users/navintamilchelvampro/c/ftp_protocol/stream.c -o CMakeFiles/stream.o.dir/stream.c.s

# Object files for target stream.o
stream_o_OBJECTS = \
"CMakeFiles/stream.o.dir/stream.c.o"

# External object files for target stream.o
stream_o_EXTERNAL_OBJECTS =

stream.o: CMakeFiles/stream.o.dir/stream.c.o
stream.o: CMakeFiles/stream.o.dir/build.make
stream.o: CMakeFiles/stream.o.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/Users/navintamilchelvampro/c/ftp_protocol/cmake-build-debug/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking C executable stream.o"
	$(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/stream.o.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
CMakeFiles/stream.o.dir/build: stream.o

.PHONY : CMakeFiles/stream.o.dir/build

CMakeFiles/stream.o.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/stream.o.dir/cmake_clean.cmake
.PHONY : CMakeFiles/stream.o.dir/clean

CMakeFiles/stream.o.dir/depend:
	cd /Users/navintamilchelvampro/c/ftp_protocol/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/navintamilchelvampro/c/ftp_protocol /Users/navintamilchelvampro/c/ftp_protocol /Users/navintamilchelvampro/c/ftp_protocol/cmake-build-debug /Users/navintamilchelvampro/c/ftp_protocol/cmake-build-debug /Users/navintamilchelvampro/c/ftp_protocol/cmake-build-debug/CMakeFiles/stream.o.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : CMakeFiles/stream.o.dir/depend
