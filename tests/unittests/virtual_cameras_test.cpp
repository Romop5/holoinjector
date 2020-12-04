#include "gtest/gtest.h"
#include "pipeline/virtual_cameras.hpp"

using namespace ve;
using namespace ve::pipeline;

namespace {
TEST(VirtualCameras, Setup) {
    VirtualCameras cameras;
    cameras.setupWindows(1,1);
    ASSERT_EQ(cameras.getCameras().size(),1);
    auto initialViewport = ViewportArea(0,0,100,100);
    cameras.updateViewports(initialViewport);
    auto& camera = cameras.getCameras()[0];
    ASSERT_EQ(camera.getViewport(), initialViewport);

    cameras.setupWindows(2,2);
    ASSERT_EQ(cameras.getCameras().size(), 2);
}

TEST(VirtualCameras, MultipleCameras) {
    VirtualCameras cameras;
    // Create 2x2 grid
    cameras.setupWindows(4,2);
    ASSERT_EQ(cameras.getCameras().size(),4);

    auto initialViewport = ViewportArea(0,0,100,200);
    cameras.updateViewports(initialViewport);
    // Vertical axis are mirrored due to OpenGL starting in left-down corner
    ASSERT_EQ(cameras.getCameras()[0].getViewport(), ViewportArea(0,100,50,100));
    ASSERT_EQ(cameras.getCameras()[1].getViewport(), ViewportArea(50,100,50,100));
    ASSERT_EQ(cameras.getCameras()[2].getViewport(), ViewportArea(0,0,50,100));
    ASSERT_EQ(cameras.getCameras()[3].getViewport(), ViewportArea(50,0,50,100));

    /*
     * Use zero angle, which implies that all cameras will have same matrix
     */
    CameraParameters params;
    params.m_XShiftMultiplier = 0.0;
    cameras.updateParamaters(params);

    /*
     * Now, verify that transformations are the same
     */

    ASSERT_EQ(cameras.getCameras()[0].getViewMatrixRotational(), glm::mat4(1.0));
    ASSERT_NE(cameras.getCameras()[0].getViewMatrixRotational(), glm::mat4(1.1));

    for(const auto& camera: cameras.getCameras())
    {
        ASSERT_EQ(camera.getViewMatrixRotational(), glm::mat4(1.0));
        // Because angleMult == 0, all views are same and distance does not matter
        ASSERT_EQ(camera.getViewMatrixRotational(), camera.getViewMatrix());
    }
}
}
