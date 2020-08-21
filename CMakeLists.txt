cmake_minimum_required(VERSION 3.10)
project(enhancer)

option(ENHANCER_32 "Force 32-bit compilation" OFF)


if(${ENHANCER_32})
    message(STATUS "Forcing 32-bit compilation")
endif()

set(PROJECT_SUFFIX, "")
if(${ENHANCER_32})
    set(PROJECT_SUFFIX "_32")
endif()
###############################################################################
# Find subhook
###############################################################################
find_file (SUBHOOK_SO NAMES libsubhook.so PATHS /usr/local/lib/ REQUIRED)
find_file (SUBHOOK_H NAMES subhook.h REQUIRED)

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
