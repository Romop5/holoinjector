cmake_minimum_required(VERSION 3.10)
project(enhancer)

option(ENHANCER_BUILD_TESTS "Should also build unittests?" ON)
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
# Create core library (enhancer)
###############################################################################

add_library(enhancer_utils INTERFACE 
)

target_sources(enhancer_utils INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/projection_estimator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/projection_estimator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_inspector.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_inspector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/shader_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framebuffer_tracker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/framebuffer_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/opengl_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/opengl_utils.cpp
    )

target_link_libraries(enhancer_utils INTERFACE glm)
target_include_directories(enhancer_utils INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/src)


###############################################################################
# Create utils library (enhancer_utils)
###############################################################################
add_library(${ENHANCER_PROJECT_NAME} INTERFACE 
)

target_sources(${ENHANCER_PROJECT_NAME} INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/opengl_redirector_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/startup_enhancer.cpp
    )

target_link_libraries(${ENHANCER_PROJECT_NAME} INTERFACE ${SUBHOOK_SO} dl X11 glm enhancer_utils)


###############################################################################
# Define function for applications, based on redirector (enhancer)
###############################################################################
macro(createEnhancerApplication name)
    set(APPLICATION_NAME "${name}${PROJECT_SUFFIX}")

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
createEnhancerApplication(shaderDumper src/startup_dumper.cpp src/shader_dumper.cpp)

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

###############################################################################
# Create tests
###############################################################################

if(${ENHANCER_BUILD_TESTS})
    enable_testing()
    find_package(GTest REQUIRED)

    set(GTEST_ROOT "${CMAKE_BINARY_DIR}/")

    #TODO: reorganize unit_test hierarchy later. Tip: have a look at Dormon's repos :D 
    add_executable(unit_tests 
        tests/unittests/projection_estimator_test.cpp
        tests/unittests/shader_inspector_test.cpp
    )
    find_package(Threads)
    target_link_libraries(unit_tests ${GTEST_BOTH_LIBRARIES} enhancer_utils Threads::Threads)

    add_test(NAME AllTests COMMAND "$<TARGET_FILE:unit_tests>")

endif()
