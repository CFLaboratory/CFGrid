# Find kaHIP using env variable kaHIP__DIR

set(kaHIP_ROOT_DIR "$ENV{kaHIP_DIR}")

find_path(kaHIP_INCLUDE_DIR
        NAMES parhip_interface.h
        PATHS ${kaHIP_ROOT_DIR}/include
          )

find_library(kaHIP_LIBRARY
             NAMES parhip_interface
             PATHS ${kaHIP_ROOT_DIR}/lib
              )

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(kaHIP DEFAULT_MSG
kaHIP_LIBRARY kaHIP_INCLUDE_DIR)
 