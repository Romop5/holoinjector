cmake_minimum_required(VERSION 3.10)
project(enhancer)

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

###############################################################################
# Get options from user
###############################################################################
option(ENHANCER_BUILD_TESTS "Should also build unittests?" ON)
option(ENHANCER_BUILD_TESTS_COVERAGE "Should also enable code coverage?" OFF)
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

find_file (SIMPLECPP_SO32 NAMES libsimplecpp.so PATHS /usr/local/lib/ REQUIRED)
find_file (SIMPLECPP_SO64 NAMES libsimplecpp.so PATHS /usr/local/lib/ REQUIRED)
find_file (SIMPLECPP_H NAMES simplecpp.h REQUIRED)

set(SIMPLECPP_SO ${SIMPLECPP_SO${ENHANCER_BITS}})

message("Found simplecpp.so: ${SIMPLECPP_SO}")

get_filename_component(SIMPLECPP_INCL ${SIMPLECPP_H} DIRECTORY)

###############################################################################

find_package(glm REQUIRED)
find_package(FreeImage REQUIRED)
###############################################################################

###############################################################################
# Create core library (enhancer)
###############################################################################

add_library(enhancer_core 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/shader_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/shader_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/framebuffer_tracker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/framebuffer_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/legacy_tracker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/legacy_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/uniform_block_tracing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/uniform_block_tracing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/texture_tracker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/texture_tracker.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/projection_estimator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/projection_estimator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/shader_inspector.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/shader_inspector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/viewport_area.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/viewport_area.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/virtual_cameras.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/virtual_cameras.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/shader_parser.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/shader_parser.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/pipeline_injector.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/pipeline_injector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/program_metadata.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/program_metadata.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/output_fbo.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/output_fbo.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/diagnostics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/diagnostics.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_utils.cpp
)

# Hack: we would like a static library to be linked into shared library later
# => Position Independent Code is thus required
target_compile_options(enhancer_core PRIVATE "-fPIC")

target_link_libraries(enhancer_core PUBLIC GL glm ${FREEIMAGE_LIBRARIES})
target_include_directories(enhancer_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FREEIMAGE_INCLUDE_DIR})
target_compile_features(enhancer_core PUBLIC cxx_std_17)



###############################################################################
# Create utils library (enhancer_core)
###############################################################################
add_library(${ENHANCER_PROJECT_NAME} INTERFACE 
)

target_sources(${ENHANCER_PROJECT_NAME} INTERFACE 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hooking/opengl_redirector_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hooking/opengl_redirector_base.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/startup_enhancer.cpp
    )

target_link_libraries(${ENHANCER_PROJECT_NAME} INTERFACE ${SUBHOOK_SO} dl X11 glm enhancer_core)


###############################################################################
# Define function for applications, based on redirector (enhancer)
###############################################################################
macro(createEnhancerApplication name)
    set(APPLICATION_NAME "${name}${PROJECT_SUFFIX}")

    add_library(${APPLICATION_NAME} SHARED
        ${ARGN}
    )
    target_link_libraries(${APPLICATION_NAME} PRIVATE ${ENHANCER_PROJECT_NAME})
    target_link_libraries(${APPLICATION_NAME} PUBLIC ${SIMPLECPP_SO})

    if(${ENHANCER_32})
        set_target_properties(${APPLICATION_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    endif()
endmacro()


###############################################################################
# Create apps
###############################################################################
createEnhancerApplication(repeater src/startup_repeater.cpp src/repeater.cpp)
#createEnhancerApplication(shaderDumper src/startup_dumper.cpp src/shader_dumper.cpp)

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
target_link_libraries(enhancer_core PUBLIC ${SIMPLECPP_SO})

###############################################################################
# Create tests
###############################################################################

if(${ENHANCER_BUILD_TESTS})
    enable_testing()
    find_package(GTest REQUIRED)
    find_package(Threads)

    #===========================================================================
    set(GTEST_ROOT "${CMAKE_BINARY_DIR}/")
    #===========================================================================
    file(GLOB UNITTEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/*.cpp)
    add_executable(unittests 
        ${UNITTEST_FILES}
    )
    target_link_libraries(unittests ${GTEST_BOTH_LIBRARIES} enhancer_core Threads::Threads)
    set(TARGET unittests PROPERTY CXX_STANDARD 17)

    #===========================================================================
    add_executable(opengl-unittest
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/opengl/opengl_test_context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/opengl/output_fbo_test.cpp
    )
    target_link_libraries(opengl-unittest ${GTEST_BOTH_LIBRARIES} enhancer_core GL X11)
    set(TARGET opengl-unittest PROPERTY CXX_STANDARD 17)

    #===========================================================================
    add_test(NAME AllTests COMMAND "$<TARGET_FILE:unittests>")
    if(${ENHANCER_BUILD_TESTS_COVERAGE})
        include(CodeCoverage)
        append_coverage_compiler_flags()

        setup_target_for_coverage_lcov(NAME unittest-cc EXECUTABLE unittests DEPENDENCIES unittests
            EXCLUDE "/usr/*" "${CMAKE_CURRENT_SOURCE_DIR}/src/opengl_redirector*")
    endif()


endif()
