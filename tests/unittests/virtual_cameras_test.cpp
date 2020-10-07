#include "gtest/gtest.h"
#include "virtual_cameras.hpp"

using namespace ve;

namespace {
TEST(VirtualCameras, Setup) {
    VirtualCameras cameras;
    cameras.setupWindows(1,1);
    EXPECT_EQ(cameras.getCameras().size(),1);
    auto initialViewport = ViewportArea(0,0,100,100);
    cameras.updateViewports(initialViewport);
    auto& camera = cameras.getCameras()[0];
    EXPECT_EQ(camera.getViewport(), initialViewport);

    cameras.setupWindows(2,2);
    EXPECT_EQ(cameras.getCameras().size(), 2);
}

TEST(VirtualCameras, MultipleCameras) {
    VirtualCameras cameras;
    // Create 2x2 grid
    cameras.setupWindows(4,2);
    EXPECT_EQ(cameras.getCameras().size(),4);

    auto initialViewport = ViewportArea(0,0,100,200);
    cameras.updateViewports(initialViewport);
    // Vertical axis are mirrored due to OpenGL starting in left-down corner
    EXPECT_EQ(cameras.getCameras()[0].getViewport(), ViewportArea(0,100,50,100));
    EXPECT_EQ(cameras.getCameras()[1].getViewport(), ViewportArea(50,100,50,100));
    EXPECT_EQ(cameras.getCameras()[2].getViewport(), ViewportArea(0,0,50,100));
    EXPECT_EQ(cameras.getCameras()[3].getViewport(), ViewportArea(50,0,50,100));

    /*
     * Use zero angle, which implies that all cameras will have same matrix
     */
    CameraParameters params;
    params.m_angleMultiplier = 0.0;
    cameras.updateParamaters(params);

    /*
     * Now, verify that transformations are the same
     */

    EXPECT_EQ(cameras.getCameras()[0].getViewMatrixRotational(), glm::mat4(1.0));
    EXPECT_NE(cameras.getCameras()[0].getViewMatrixRotational(), glm::mat4(1.1));

    for(const auto& camera: cameras.getCameras())
    {
        EXPECT_EQ(camera.getViewMatrixRotational(), glm::mat4(1.0));
        // Because angleMult == 0, all views are same and distance does not matter
        EXPECT_EQ(camera.getViewMatrixRotational(), camera.getViewMatrix());
    }
}
}
