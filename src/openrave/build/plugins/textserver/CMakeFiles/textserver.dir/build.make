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

# Include any dependencies generated for this target.
include plugins/textserver/CMakeFiles/textserver.dir/depend.make

# Include the progress variables for this target.
include plugins/textserver/CMakeFiles/textserver.dir/progress.make

# Include the compile flags for this target's objects.
include plugins/textserver/CMakeFiles/textserver.dir/flags.make

plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o: plugins/textserver/CMakeFiles/textserver.dir/flags.make
plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o: ../plugins/textserver/textserver.cpp
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --progress-dir=/home/dvrk-lite/ws_moveit_test/src/openrave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Building CXX object plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o"
	cd /home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/textserver && /usr/bin/c++   $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -o CMakeFiles/textserver.dir/textserver.cpp.o -c /home/dvrk-lite/ws_moveit_test/src/openrave/plugins/textserver/textserver.cpp

plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.i: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Preprocessing CXX source to CMakeFiles/textserver.dir/textserver.cpp.i"
	cd /home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/textserver && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -E /home/dvrk-lite/ws_moveit_test/src/openrave/plugins/textserver/textserver.cpp > CMakeFiles/textserver.dir/textserver.cpp.i

plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.s: cmake_force
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green "Compiling CXX source to assembly CMakeFiles/textserver.dir/textserver.cpp.s"
	cd /home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/textserver && /usr/bin/c++  $(CXX_DEFINES) $(CXX_INCLUDES) $(CXX_FLAGS) -S /home/dvrk-lite/ws_moveit_test/src/openrave/plugins/textserver/textserver.cpp -o CMakeFiles/textserver.dir/textserver.cpp.s

plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o.requires:

.PHONY : plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o.requires

plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o.provides: plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o.requires
	$(MAKE) -f plugins/textserver/CMakeFiles/textserver.dir/build.make plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o.provides.build
.PHONY : plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o.provides

plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o.provides.build: plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o


# Object files for target textserver
textserver_OBJECTS = \
"CMakeFiles/textserver.dir/textserver.cpp.o"

# External object files for target textserver
textserver_EXTERNAL_OBJECTS =

plugins/textserver/libtextserver.so: plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o
plugins/textserver/libtextserver.so: plugins/textserver/CMakeFiles/textserver.dir/build.make
plugins/textserver/libtextserver.so: src/libopenrave/libopenrave0.9.so.0.9.0
plugins/textserver/libtextserver.so: /usr/lib/x86_64-linux-gnu/libxml2.so
plugins/textserver/libtextserver.so: /usr/lib/x86_64-linux-gnu/libboost_date_time.so
plugins/textserver/libtextserver.so: /usr/lib/x86_64-linux-gnu/libboost_thread.so
plugins/textserver/libtextserver.so: cpp-gen-md5/libopenrave-md5.a
plugins/textserver/libtextserver.so: /usr/lib/x86_64-linux-gnu/libboost_filesystem.so
plugins/textserver/libtextserver.so: /usr/lib/x86_64-linux-gnu/libboost_system.so
plugins/textserver/libtextserver.so: 3rdparty/crlibm-1.0beta4/libcrlibm.a
plugins/textserver/libtextserver.so: 3rdparty/fparser-4.5/libfparser.a
plugins/textserver/libtextserver.so: /usr/lib/x86_64-linux-gnu/libmpfr.so
plugins/textserver/libtextserver.so: /usr/lib/x86_64-linux-gnu/libgmp.so
plugins/textserver/libtextserver.so: plugins/textserver/CMakeFiles/textserver.dir/link.txt
	@$(CMAKE_COMMAND) -E cmake_echo_color --switch=$(COLOR) --green --bold --progress-dir=/home/dvrk-lite/ws_moveit_test/src/openrave/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_2) "Linking CXX shared library libtextserver.so"
	cd /home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/textserver && $(CMAKE_COMMAND) -E cmake_link_script CMakeFiles/textserver.dir/link.txt --verbose=$(VERBOSE)

# Rule to build all files generated by this target.
plugins/textserver/CMakeFiles/textserver.dir/build: plugins/textserver/libtextserver.so

.PHONY : plugins/textserver/CMakeFiles/textserver.dir/build

plugins/textserver/CMakeFiles/textserver.dir/requires: plugins/textserver/CMakeFiles/textserver.dir/textserver.cpp.o.requires

.PHONY : plugins/textserver/CMakeFiles/textserver.dir/requires

plugins/textserver/CMakeFiles/textserver.dir/clean:
	cd /home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/textserver && $(CMAKE_COMMAND) -P CMakeFiles/textserver.dir/cmake_clean.cmake
.PHONY : plugins/textserver/CMakeFiles/textserver.dir/clean

plugins/textserver/CMakeFiles/textserver.dir/depend:
	cd /home/dvrk-lite/ws_moveit_test/src/openrave/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/dvrk-lite/ws_moveit_test/src/openrave /home/dvrk-lite/ws_moveit_test/src/openrave/plugins/textserver /home/dvrk-lite/ws_moveit_test/src/openrave/build /home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/textserver /home/dvrk-lite/ws_moveit_test/src/openrave/build/plugins/textserver/CMakeFiles/textserver.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : plugins/textserver/CMakeFiles/textserver.dir/depend

