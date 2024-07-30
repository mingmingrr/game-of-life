cmake_minimum_required(VERSION 3.15)

project(life CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include(FetchContent)

FetchContent_Declare(dynamic_bitset
	GIT_REPOSITORY https://github.com/pinam45/dynamic_bitset.git
	GIT_TAG        ac60c9e6c534db7457ca6af02fdedbe74ad60968)
FetchContent_MakeAvailable(dynamic_bitset)

find_package(Curses REQUIRED)
include_directories(${CURSES_INCLUDE_DIR})

add_executable(life
	life.cpp)
target_link_libraries(life
	dynamic_bitset
	${CURSES_LIBRARIES})
# target_include_directories(life
	# PRIVATE src/pyrsistent_extras)

