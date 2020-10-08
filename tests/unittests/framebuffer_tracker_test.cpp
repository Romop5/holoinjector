#include "gtest/gtest.h"
#include "framebuffer_tracker.hpp"

using namespace ve;

namespace {
TEST(FramebufferTracker, Basic) {
    FramebufferTracker ft;
    /*
     * By default, default program should be bounded
     */
    ASSERT_EQ(ft.getCurrentFrameBuffer(),0);
    ASSERT_TRUE(ft.isFBODefault());
    ASSERT_FALSE(ft.isFBOshadowMap());

    ASSERT_FALSE(ft.hasFramebuffer(1));

    /*
     * Create new FBO and only attach depth buffer
     */
    ft.addFramebuffer(1);
    // Not binded yet, still default
    ASSERT_TRUE(ft.isFBODefault());
    // Bind new FBO
    ft.bind(1);
    ASSERT_EQ(ft.getCurrentFrameBuffer(),1);
    ASSERT_TRUE(!ft.isFBODefault());
    ft.attach(GL_DEPTH_ATTACHMENT, 0);
    /*
     * New FBO only has depth attached => should be shadow map
     */
    ASSERT_TRUE(ft.isFBOshadowMap());
    ft.attach(GL_COLOR_ATTACHMENT0, 1);
    ASSERT_FALSE(ft.isFBOshadowMap());

    /*
     * Rebind default (unbind)
     */
    ft.bind(0);
    ASSERT_EQ(ft.getCurrentFrameBuffer(),0);

    ASSERT_TRUE(ft.isFBODefault());
    ASSERT_FALSE(ft.isFBOshadowMap());
}

TEST(FramebufferTracker, NonDefaultAttachment) {
    FramebufferTracker ft;
    for(size_t i = 0; i < 8; i++)
    {
        ft.addFramebuffer(i+1);
        ASSERT_EQ(ft.getCurrentFrameBuffer(),i);
        ft.bind(i+1);
        ASSERT_EQ(ft.getCurrentFrameBuffer(),i+1);
        ft.attach(GL_DEPTH_ATTACHMENT, 0);
        ASSERT_TRUE(ft.isFBOshadowMap());
        ft.attach(GL_COLOR_ATTACHMENT0+i, i+1);
        ASSERT_FALSE(ft.isFBOshadowMap());
    }
}
TEST(FramebufferTracker, Stencil) {
    FramebufferTracker ft;
    ft.addFramebuffer(1);
    ft.bind(1);
    ft.attach(GL_STENCIL_ATTACHMENT, 0);
    ASSERT_FALSE(ft.isFBOshadowMap());
}
TEST(FramebufferTracker, DepthStencil) {
    FramebufferTracker ft;
    ft.addFramebuffer(1);
    ft.bind(1);
    ft.attach(GL_DEPTH_STENCIL_ATTACHMENT, 0);
    ASSERT_TRUE(ft.isFBOshadowMap());
}

TEST(FramebufferTracker, DeleteFBO) {
    FramebufferTracker ft;
    ft.addFramebuffer(1);
    ft.bind(1);
    ft.attach(GL_DEPTH_ATTACHMENT, 0);
    ASSERT_TRUE(ft.isFBOshadowMap());
    ASSERT_TRUE(ft.hasFramebuffer(1));

    ft.deleteFramebuffer(1);
    ASSERT_FALSE(ft.hasFramebuffer(1));
    ASSERT_EQ(ft.getCurrentFrameBuffer(),0);
    ASSERT_FALSE(ft.isFBOshadowMap());
}
}
