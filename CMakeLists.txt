cmake_minimum_required(VERSION 3.10)

project(holoinjector
    VERSION 1.0
    DESCRIPTION "A conversion layer for legacy OpenGL apps to multiview, wide-angle 3D display"
    HOMEPAGE_URL "https://github.com/Romop5/holoinjector")

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")

###############################################################################
# Compiler-specific
###############################################################################
# Clang-only
#add_compile_options("-ftime-trace")

option (FORCE_COLORED_OUTPUT "Always produce ANSI-colored output (GNU/Clang only)." FALSE)
if (${FORCE_COLORED_OUTPUT})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
       add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()

###############################################################################
# Get options from user
###############################################################################
option(HOLOINJECTOR_BUILD_TESTS          "Should also build unittests?" ON)
option(HOLOINJECTOR_BUILD_DOCS           "Should also build Doxygen docs?" OFF)
option(HOLOINJECTOR_BUILD_TESTS_COVERAGE "Should also enable code coverage?" OFF)
option(HOLOINJECTOR_32                   "Force 32-bit compilation" OFF)

# Stores 32 or 64 into HOLOINJECTOR_BITS
set(HOLOINJECTOR_BITS 64)

if(${HOLOINJECTOR_32})
    message(STATUS "Forcing 32-bit compilation")
    set(HOLOINJECTOR_BITS 32)
endif()

set(PROJECT_SUFFIX, "_64")
if(${HOLOINJECTOR_32})
    set(PROJECT_SUFFIX "_32")
endif()

set(HOLOINJECTOR_PROJECT_NAME "HoloInjector${PROJECT_SUFFIX}")
message(STATUS "Using project name: ${HOLOINJECTOR_PROJECT_NAME}")

###############################################################################
# Find subhook & 3rd parties
###############################################################################
#find_file (SUBHOOK_SO32 NAMES libsubhook.so PATHS /usr/local/lib/ REQUIRED)
#find_file (SUBHOOK_SO64 NAMES libsubhook.so PATHS /usr/local/lib64/ REQUIRED)
#find_file (SUBHOOK_H NAMES subhook.h REQUIRED)

#set(SUBHOOK_SO ${SUBHOOK_SO${HOLOINJECTOR_BITS}})
#get_filename_component(SUBHOOK_INCL ${SUBHOOK_H} DIRECTORY)

find_file (SIMPLECPP_SO32 NAMES libsimplecpp.so PATHS /usr/local/lib/ REQUIRED)
find_file (SIMPLECPP_SO64 NAMES libsimplecpp.so PATHS /usr/local/lib/ REQUIRED)
find_file (SIMPLECPP_H NAMES simplecpp.h REQUIRED)

set(SIMPLECPP_SO ${SIMPLECPP_SO${HOLOINJECTOR_BITS}})

message("Found simplecpp.so: ${SIMPLECPP_SO}")

get_filename_component(SIMPLECPP_INCL ${SIMPLECPP_H} DIRECTORY)

###############################################################################

find_package(glm REQUIRED)
find_package(FreeImage REQUIRED)
#find_package(fmt REQUIRED)
find_package(yaml-cpp REQUIRED)

find_library(IMGUI_LIB libimgui.so PATHS lib lib64 REQUIRED)

if(${HOLOINJECTOR_BUILD_DOCS})
    find_package(Doxygen
                 REQUIRED dot
                 OPTIONAL_COMPONENTS mscgen dia)

    file(GLOB DOXY_FILES ${CMAKE_CURRENT_SOURCE_DIR}/src/*)
    set(DOXYGEN_HAVE_DOT TRUE)
    doxygen_add_docs(Doxygen "${CMAKE_CURRENT_SOURCE_DIR}/README.md"
        ${DOXY_FILES} ALL)
endif()
###############################################################################

###############################################################################
# Create core library (holoinjector)
###############################################################################

add_library(injector_core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/shader_tracker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/shader_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/framebuffer_tracker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/framebuffer_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/legacy_tracker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/legacy_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/uniform_block_tracing.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/uniform_block_tracing.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/texture_tracker.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/texture_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/renderbuffer_tracker.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/trackers/renderbuffer_tracker.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/projection_estimator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/projection_estimator.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/shader_inspector.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/shader_inspector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/viewport_area.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/viewport_area.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/camera_parameters.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/camera_parameters.cpp
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
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pipeline/shader_profile.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/diagnostics.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/diagnostics.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/context.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/context.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/logger.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_adapter.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/imgui_adapter.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/draw_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/draw_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/shader_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/shader_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/ui_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/ui_manager.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/framebuffer_manager.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/managers/framebuffer_manager.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/paralax/mapping.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/paralax/mapping.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_objects.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_objects.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_state.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_state.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_raii.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_raii.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/enviroment.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/enviroment.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/glsl_preprocess.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/glsl_preprocess.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/string_utils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/string_utils.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_debug.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/utils/opengl_debug.hpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/x11_sniffer.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/x11_sniffer.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/settings_widget.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/settings_widget.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/inspector_widget.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ui/inspector_widget.cpp
)

# Hack: we would like a static library to be linked into shared library later
# => Position Independent Code is thus required
target_compile_options(injector_core PRIVATE "-fPIC")
set_target_properties(injector_core PROPERTIES CXX_VISIBILITY_PRESET "hidden")
set_target_properties(injector_core PROPERTIES VISIBILITY_INLINES_HIDDEN ON)

target_link_libraries(injector_core PUBLIC GL GLU GLEW glm ${FREEIMAGE_LIBRARIES}
  yaml-cpp ${IMGUI_LIB})
target_include_directories(injector_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${FREEIMAGE_INCLUDE_DIR})
target_compile_features(injector_core PUBLIC cxx_std_17)

###############################################################################
# Create utils library (injector_core)
###############################################################################
add_library(${HOLOINJECTOR_PROJECT_NAME} INTERFACE 
)

target_sources(${HOLOINJECTOR_PROJECT_NAME} INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hooking/opengl_redirector_base.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/hooking/opengl_redirector_base.hpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/startup_injector.cpp
    )

target_link_libraries(${HOLOINJECTOR_PROJECT_NAME} INTERFACE ${SUBHOOK_SO} dl X11 glm injector_core)


###############################################################################
# Define function for applications, based on redirector (holoinjector)
###############################################################################
macro(createHoloInjectorApplication name)
    set(APPLICATION_NAME "${name}${PROJECT_SUFFIX}")

    add_library(${APPLICATION_NAME} SHARED
        ${ARGN}
    )
    target_link_libraries(${APPLICATION_NAME} PRIVATE ${HOLOINJECTOR_PROJECT_NAME})
    target_link_libraries(${APPLICATION_NAME} PUBLIC ${SIMPLECPP_SO} ${IMGUI_LIB})
    set_target_properties(${APPLICATION_NAME} PROPERTIES CXX_VISIBILITY_PRESET "hidden")
    set_target_properties(${APPLICATION_NAME} PROPERTIES VISIBILITY_INLINES_HIDDEN ON)

    if(${HOLOINJECTOR_32})
        set_target_properties(${APPLICATION_NAME} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
    endif()
endmacro()


###############################################################################
# Create apps
###############################################################################
createHoloInjectorApplication(holoInjector src/startup_main.cpp src/dispatcher.cpp)
#createHoloInjectorApplication(shaderDumper src/startup_dumper.cpp src/shader_dumper.cpp)

###############################################################################
# Create test app
###############################################################################
add_executable(app 
    src/main.cpp
)
if(${HOLOINJECTOR_32})
    set_target_properties(app PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
endif()
target_link_libraries(app PRIVATE holoInjector${PROJECT_SUFFIX} dl)
target_link_libraries(injector_core PUBLIC ${SIMPLECPP_SO})

###############################################################################
# Create tests
###############################################################################

if(${HOLOINJECTOR_BUILD_TESTS})
    enable_testing()
    find_package(GTest REQUIRED)
    find_package(Threads)

    #===========================================================================
    set(GTEST_ROOT "${CMAKE_BINARY_DIR}/")
    #===========================================================================
    file(GLOB_RECURSE UNITTEST_FILES ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/*
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/unittests/*/*)
    add_executable(unittests 
        ${UNITTEST_FILES}
    )
    target_link_libraries(unittests ${GTEST_BOTH_LIBRARIES} injector_core Threads::Threads)
    set(TARGET unittests PROPERTY CXX_STANDARD 17)

    #===========================================================================
    add_executable(opengl-unittest
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/opengl/opengl_test_context.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/opengl/output_fbo_test.cpp
    )
    target_link_libraries(opengl-unittest ${GTEST_BOTH_LIBRARIES} injector_core GL X11)
    set(TARGET opengl-unittest PROPERTY CXX_STANDARD 17)

    #===========================================================================
    add_test(NAME AllTests COMMAND "$<TARGET_FILE:unittests>")
    if(${HOLOINJECTOR_BUILD_TESTS_COVERAGE})
        include(CodeCoverage)
        append_coverage_compiler_flags()

        setup_target_for_coverage_lcov(NAME coverage EXECUTABLE unittests DEPENDENCIES unittests
            EXCLUDE "/usr/*" "src/opengl_redirector*" "3rd/*" "tests/*" "src/hooking")
    endif()


endif()
