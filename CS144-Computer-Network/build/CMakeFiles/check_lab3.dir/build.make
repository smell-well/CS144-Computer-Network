# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.27

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:

#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:

# Disable VCS-based implicit rules.
% : %,v

# Disable VCS-based implicit rules.
% : RCS/%

# Disable VCS-based implicit rules.
% : RCS/%,v

# Disable VCS-based implicit rules.
% : SCCS/s.%

# Disable VCS-based implicit rules.
% : s.%

.SUFFIXES: .hpux_make_needs_suffix_list

# Command-line flag to silence nested $(MAKE).
$(VERBOSE)MAKESILENT = -s

#Suppress display of executed commands.
$(VERBOSE).SILENT:

# A target that is always out of date.
cmake_force:
.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /opt/cmake-3.27.1-linux-x86_64/bin/cmake

# The command to remove a file.
RM = /opt/cmake-3.27.1-linux-x86_64/bin/cmake -E rm -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /home/ymk/proj/cs144/CS144-Computer-Network

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /home/ymk/proj/cs144/CS144-Computer-Network/build

# Utility rule file for check_lab3.

# Include any custom commands dependencies for this target.
include CMakeFiles/check_lab3.dir/compiler_depend.make

# Include the progress variables for this target.
include CMakeFiles/check_lab3.dir/progress.make

CMakeFiles/check_lab3:
	@$(CMAKE_COMMAND) -E cmake_echo_color "--switch=$(COLOR)" --blue --bold --progress-dir=/home/ymk/proj/cs144/CS144-Computer-Network/build/CMakeFiles --progress-num=$(CMAKE_PROGRESS_1) "Testing the TCP sender..."
	/opt/cmake-3.27.1-linux-x86_64/bin/ctest --output-on-failure --timeout 10 -R 't_send_|t_recv_|t_wrapping_|t_strm_reassem_|t_byte_stream|_dt'

check_lab3: CMakeFiles/check_lab3
check_lab3: CMakeFiles/check_lab3.dir/build.make
.PHONY : check_lab3

# Rule to build all files generated by this target.
CMakeFiles/check_lab3.dir/build: check_lab3
.PHONY : CMakeFiles/check_lab3.dir/build

CMakeFiles/check_lab3.dir/clean:
	$(CMAKE_COMMAND) -P CMakeFiles/check_lab3.dir/cmake_clean.cmake
.PHONY : CMakeFiles/check_lab3.dir/clean

CMakeFiles/check_lab3.dir/depend:
	cd /home/ymk/proj/cs144/CS144-Computer-Network/build && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /home/ymk/proj/cs144/CS144-Computer-Network /home/ymk/proj/cs144/CS144-Computer-Network /home/ymk/proj/cs144/CS144-Computer-Network/build /home/ymk/proj/cs144/CS144-Computer-Network/build /home/ymk/proj/cs144/CS144-Computer-Network/build/CMakeFiles/check_lab3.dir/DependInfo.cmake "--color=$(COLOR)"
.PHONY : CMakeFiles/check_lab3.dir/depend
