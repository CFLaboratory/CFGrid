# CMakeLists.txt
#
# The CMake configuration for the CFGrid project source.
#
# SPDX-License-Identifier: Apache-2.0

add_library(objparser OBJECT
  _node_parser.cpp node_parser.cpp
  topo_parser.cpp
)
target_include_directories(objparser PUBLIC ${CMAKE_SOURCE_DIR}/include)
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

add_library(objmesh OBJECT
  gmsh.cpp
)
target_include_directories(objmesh PUBLIC ${CMAKE_SOURCE_DIR}/include)
target_link_libraries(objmesh objparser)

add_library(libcfg
  $<TARGET_OBJECTS:objparser>
  $<TARGET_OBJECTS:objmesh>
)
target_include_directories(libcfg PUBLIC ${CMAKE_SOURCE_DIR}/include)
set_target_properties(libcfg PROPERTIES OUTPUT_NAME "cfg") # Prevents building "liblibcfg.x"

add_executable(cfgrid main.cpp)
target_link_libraries(cfgrid libcfg)

find_package(MPI REQUIRED)
target_link_libraries(cfgrid MPI::MPI_CXX)
