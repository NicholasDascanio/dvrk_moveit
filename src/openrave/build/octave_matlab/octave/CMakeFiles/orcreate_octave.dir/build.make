# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.5

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
CMAKE_COMMAND = /usr/bin/cmake

# The command to remove a file.
RM = /usr/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/dvrk-lite/ws_moveit_test/src/openrave

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/dvrk-lite/ws_moveit_test/src/openrave/build

# Utility rule file for orcreate_octave.

# Include the progress variables for this target.
include octave_matlab/octave/CMakeFiles/orcreate_octave.dir/progress.make

octave_matlab/octave/CMakeFiles/orcreate_octave: octave_matlab/octave/orcreate.mex


octave_matlab/octave/orcreate.mex: ../octave_matlab/orcreate.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --blue --bold --progress-dir=/home/dvrk-lite/ws_moveit_test/src/openrave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Generating orcreate.mex"
	cd /home/dvrk-lite/ws_moveit_test/src/openrave/build/octave_matlab/octave && /usr/bin/mkoctfile --mex -I/home/dvrk-lite/ws_moveit_test/src/openrave --strip -o "/home/dvrk-lite/ws_moveit_test/src/openrave/build/octave_matlab/octave/orcreate.mex" "/home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/orcreate.cpp"

orcreate_octave: octave_matlab/octave/CMakeFiles/orcreate_octave
orcreate_octave: octave_matlab/octave/orcreate.mex
orcreate_octave: octave_matlab/octave/CMakeFiles/orcreate_octave.dir/build.make

.PHONY : orcreate_octave

# Rule to build all files generated by this target.
octave_matlab/octave/CMakeFiles/orcreate_octave.dir/build: orcreate_octave

.PHONY : octave_matlab/octave/CMakeFiles/orcreate_octave.dir/build

octave_matlab/octave/CMakeFiles/orcreate_octave.dir/clean:
	cd /home/dvrk-lite/ws_moveit_test/src/openrave/build/octave_matlab/octave && $(CMAKE_COMMAND) -P CMakeFiles/orcreate_octave.dir/cmake_clean.cmake
.PHONY : octave_matlab/octave/CMakeFiles/orcreate_octave.dir/clean

octave_matlab/octave/CMakeFiles/orcreate_octave.dir/depend:
	cd /home/dvrk-lite/ws_moveit_test/src/openrave/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dvrk-lite/ws_moveit_test/src/openrave /home/dvrk-lite/ws_moveit_test/src/openrave/octave_matlab/octave /home/dvrk-lite/ws_moveit_test/src/openrave/build /home/dvrk-lite/ws_moveit_test/src/openrave/build/octave_matlab/octave /home/dvrk-lite/ws_moveit_test/src/openrave/build/octave_matlab/octave/CMakeFiles/orcreate_octave.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : octave_matlab/octave/CMakeFiles/orcreate_octave.dir/depend

