# CMakeLists.txt
#
# The CMake configuration for the CFGrid project source.
#
# SPDX-License-Identifier: Apache-2.0

# Integration of KaHIP/parhip
Include(FetchContent)

FetchContent_Declare(
  KaHIP
  GIT_REPOSITORY https://github.com/KaHIP/KaHIP.git
  GIT_TAG        v3.18
)
FetchContent_MakeAvailable(KaHIP)

# remove the -Wpedantic flag as parhip doesn't handle it
target_compile_options(parhip PRIVATE "-Wno-pedantic")


add_library(objreader OBJECT reader.cpp)
target_include_directories(objreader PUBLIC ${CMAKE_SOURCE_DIR}/include)

add_library(objnode_parser OBJECT _node_parser.cpp node_parser.cpp)
target_include_directories(objnode_parser PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(objnode_parser objreader)

add_library(libcfg $<TARGET_OBJECTS:objreader> $<TARGET_OBJECTS:objnode_parser>)
target_include_directories(libcfg PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(libcfg PROPERTIES OUTPUT_NAME "cfg") # Prevents building "liblibcfg.x"

add_executable(cfgrid main.cpp)
target_link_libraries(cfgrid libcfg)

find_package(MPI REQUIRED)
target_link_libraries(cfgrid MPI::MPI_CXX)
