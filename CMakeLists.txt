cmake_minimum_required(VERSION 3.10)
project(enhancer)

option(ENHANCER_32 "Force 32-bit compilation" OFF)

# Stores 32 or 64 into ENHANCER_BITS
set(ENHANCER_BITS 64)

if(${ENHANCER_32})
    message(STATUS "Forcing 32-bit compilation")
    set(ENHANCER_BITS 32)
endif()

set(PROJECT_SUFFIX, "")
if(${ENHANCER_32})
    set(PROJECT_SUFFIX "_32")
endif()
###############################################################################
# Find subhook
###############################################################################
find_file (SUBHOOK_SO32 NAMES libsubhook.so PATHS /usr/local/lib/ REQUIRED)
find_file (SUBHOOK_SO64 NAMES libsubhook.so PATHS /usr/local/lib64/ REQUIRED)
find_file (SUBHOOK_H NAMES subhook.h REQUIRED)

set(SUBHOOK_SO ${SUBHOOK_SO${ENHANCER_BITS}})

get_filename_component(SUBHOOK_INCL ${SUBHOOK_H} DIRECTORY)
###############################################################################

set(ENHANCER_PROJECT_NAME "enhancer${PROJECT_SUFFIX}")
message(STATUS "Using project name: ${ENHANCER_PROJECT_NAME}")
add_library(${ENHANCER_PROJECT_NAME} SHARED
    src/opengl_redirector_base.cpp
    src/repeater.cpp
    src/startup.cpp
)

target_link_libraries(${ENHANCER_PROJECT_NAME} PRIVATE ${SUBHOOK_SO} dl)

if(${ENHANCER_32})
    set_target_properties(${ENHANCER_PROJECT_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()

add_executable(app 
    src/main.cpp
)
if(${ENHANCER_32})
    set_target_properties(app PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()
target_link_libraries(app PRIVATE ${ENHANCER_PROJECT_NAME} dl)
