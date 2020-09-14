cmake_minimum_required(VERSION 3.10)
project(enhancer)

###############################################################################
# Decide bits (32 or 64)?
###############################################################################
option(ENHANCER_32 "Force 32-bit compilation" OFF)

# Stores 32 or 64 into ENHANCER_BITS
set(ENHANCER_BITS 64)

if(${ENHANCER_32})
    message(STATUS "Forcing 32-bit compilation")
    set(ENHANCER_BITS 32)
endif()

set(PROJECT_SUFFIX, "_64")
if(${ENHANCER_32})
    set(PROJECT_SUFFIX "_32")
endif()

set(ENHANCER_PROJECT_NAME "enhancer${PROJECT_SUFFIX}")
message(STATUS "Using project name: ${ENHANCER_PROJECT_NAME}")

###############################################################################
# Find subhook & 3rd parties
###############################################################################
find_file (SUBHOOK_SO32 NAMES libsubhook.so PATHS /usr/local/lib/ REQUIRED)
find_file (SUBHOOK_SO64 NAMES libsubhook.so PATHS /usr/local/lib64/ REQUIRED)
find_file (SUBHOOK_H NAMES subhook.h REQUIRED)

set(SUBHOOK_SO ${SUBHOOK_SO${ENHANCER_BITS}})

get_filename_component(SUBHOOK_INCL ${SUBHOOK_H} DIRECTORY)
###############################################################################

find_package(glm REQUIRED)
###############################################################################

###############################################################################
# Create base library (enhancer)
###############################################################################

add_library(${ENHANCER_PROJECT_NAME} INTERFACE 
)

target_sources(${ENHANCER_PROJECT_NAME} INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/opengl_redirector_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/startup_enhancer.cpp
    )

target_link_libraries(${ENHANCER_PROJECT_NAME} INTERFACE ${SUBHOOK_SO} dl X11 glm)

###############################################################################
# Define function for applications, based on redirector (enhancer)
###############################################################################
macro(createEnhancerApplication name)
    message ("ARGN: ${ARGN}")
    message ("Suffix: ${PROJECT_SUFFIX}")
    set(APPLICATION_NAME "${name}${PROJECT_SUFFIX}")
    message ("Appname: ${APPLICATION_NAME}")

    add_library(${APPLICATION_NAME} SHARED
        ${ARGN}
    )
    target_link_libraries(${APPLICATION_NAME} PRIVATE ${ENHANCER_PROJECT_NAME})

    if(${ENHANCER_32})
        set_target_properties(${APPLICATION_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    endif()
endmacro()


###############################################################################
# Create apps
###############################################################################
createEnhancerApplication(repeater src/startup_repeater.cpp src/repeater.cpp)

###############################################################################
# Create test app
###############################################################################
add_executable(app 
    src/main.cpp
)
if(${ENHANCER_32})
    set_target_properties(app PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()
target_link_libraries(app PRIVATE repeater${PROJECT_SUFFIX} dl)
