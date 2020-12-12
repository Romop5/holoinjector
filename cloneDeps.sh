#!/bin/bash 
# Author: Roman Dobias
# Purpose: Clone all necessary 3rd party libraries to '3rd' directory
#          compile & install into '3rd/install' and generate CMake conf
#          in build 

mkdir -p 3rd
mkdir -p 3rd/install

INSTALL_DIR=`realpath 3rd/install`
echo "Install dir: ${INSTALL_DIR}"

function prepareRepo {
    REPO_NAME=$1
    REPO_URL=$2
    REPO_ADDITIONAL_CMAKE_DEFINITIONS=$3
    START_PATH=`pwd`

    echo "${REPO_NAME} : ${REPO_URL}"
    cd 3rd
    git clone ${REPO_URL} ${REPO_NAME}
    cd ${REPO_NAME}
    mkdir -p build && cd build
    cmake ../ -DCMAKE_INSTALL_PREFIX=${INSTALL_DIR} ${REPO_ADDITIONAL_CMAKE_DEFINITIONS} 
    make -j4
    make install
    cd ${START_PATH}
}

###############################################################################
# I. Clone deps
###############################################################################
prepareRepo "simplecpp" "https://github.com/Romop5/simplecpp" 
prepareRepo "subhook" "https://github.com/Romop5/subhook" 
prepareRepo "fmt" "https://github.com/fmtlib/fmt" 
prepareRepo "yaml-cpp" "https://github.com/jbeder/yaml-cpp" "-DYAML_BUILD_SHARED_LIBS=ON" 
prepareRepo "imgui" "https://github.com/ocornut/imgui" 

###############################################################################
# II. Add imgui configuration
###############################################################################
echo "
cmake_minimum_required(VERSION 3.10)
project(imgui)
add_library(imgui SHARED 
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_tables.cpp
    imgui_widgets.cpp
    imconfig.h
    imgui.h
    imgui_internal.h
    imstb_rectpack.h
    imstb_textedit.h
    imstb_truetype.h

    backends/imgui_impl_opengl3.h
    backends/imgui_impl_opengl3.cpp
)
target_include_directories(imgui PUBLIC \${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(imgui PRIVATE GLEW)
install(TARGETS imgui DESTINATION lib)
INSTALL(FILES "imgui.h imconfig.h backends/imgui_impl_opengl3.h" DESTINATION include)
" > 3rd/imgui/CMakeLists.txt
prepareRepo "imgui" "https://github.com/ocornut/imgui" 

###############################################################################
# III. Generate configuration
###############################################################################

cd build
CMAKE_PREFIX_PATH=${CMAKE_PREFIX_PATH}:${INSTALL_DIR} cmake ../


