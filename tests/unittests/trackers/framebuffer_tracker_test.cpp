#include "gtest/gtest.h"
#include "trackers/framebuffer_tracker.hpp"
#include "trackers/texture_tracker.hpp"

using namespace ve;
using namespace ve::trackers;

namespace {
TEST(FramebufferTracker, Basic) {
    FramebufferTracker ft;
    /*
     * By default, default program should be bounded
     */
    EXPECT_EQ(ft.getBoundId(),0);
    EXPECT_TRUE(ft.isFBODefault());
    EXPECT_TRUE(ft.isSuitableForRepeating());

    EXPECT_FALSE(ft.has(1));

    /*
     * Create new FBO and only attach depth buffer
     */
    ft.add(1, std::make_shared<FramebufferMetadata>(1));
    // Not binded yet, still default
    EXPECT_TRUE(ft.isFBODefault());
    // Bind new FBO
    ft.bind(1);
    EXPECT_EQ(ft.getBoundId(),1);
    EXPECT_TRUE(!ft.isFBODefault());


    TextureTracker tt;
    auto depthTexture = std::make_shared<TextureMetadata>(0);
    depthTexture->setStorage(GL_TEXTURE_2D,100,100,1,0, GL_DEPTH_COMPONENT24);
    tt.add(0, depthTexture);

    ft.get(1)->attach(GL_DEPTH_ATTACHMENT, depthTexture);
    /*
     * New FBO only has depth attached => should be shadow map
     */
    EXPECT_TRUE(ft.get(1)->isShadowMapFBO());

    auto colorTexture= std::make_shared<TextureMetadata>(1);
    colorTexture->setStorage(GL_TEXTURE_2D,100,100,1,0, GL_RGBA8);
    tt.add(1, colorTexture);

    ft.get(1)->attach(GL_COLOR_ATTACHMENT0, colorTexture);
    EXPECT_FALSE(ft.get(1)->isShadowMapFBO());

    /*
     * Rebind default (unbind)
     */
    ft.bind(0);
    EXPECT_EQ(ft.getBoundId(),0);

    EXPECT_TRUE(ft.isFBODefault());
    EXPECT_TRUE(ft.isSuitableForRepeating());
}

TEST(FramebufferTracker, NonDefaultAttachment) {
    FramebufferTracker ft;
    TextureTracker tt;

    auto depthTexture = std::make_shared<TextureMetadata>(1);
    depthTexture->setStorage(GL_TEXTURE_2D,100,100,1,0, GL_DEPTH_COMPONENT16); 
    tt.add(1, depthTexture);

    auto colorTexture= std::make_shared<TextureMetadata>(2);
    colorTexture->setStorage(GL_TEXTURE_2D,100,100,1,0, GL_RGBA8); 
    tt.add(2, colorTexture);

    for(size_t i = 0; i < 8; i++)
    {
        ft.add(i+1, std::make_shared<FramebufferMetadata>(i+1));
        EXPECT_EQ(ft.getBoundId(),i);
        ft.bind(i+1);
        EXPECT_EQ(ft.getBoundId(),i+1);
        ft.getBound()->attach(GL_DEPTH_ATTACHMENT, depthTexture);
        EXPECT_TRUE(ft.getBound()->isShadowMapFBO());
        ft.getBound()->attach(GL_COLOR_ATTACHMENT0+i, colorTexture);
        EXPECT_FALSE(ft.getBound()->isShadowMapFBO());
    }
}
TEST(FramebufferTracker, Stencil) {
    FramebufferTracker ft;
    ft.add(1, std::make_shared<FramebufferMetadata>(1));
    ft.bind(1);
    ft.getBound()->attach(GL_STENCIL_ATTACHMENT, 0);
    EXPECT_FALSE(ft.getBound()->isShadowMapFBO());
}
TEST(FramebufferTracker, DepthStencil) {
    FramebufferTracker ft;
    ft.add(1, std::make_shared<FramebufferMetadata>(1));
    ft.bind(1);
    ft.getBound()->attach(GL_DEPTH_STENCIL_ATTACHMENT, 0);
    EXPECT_TRUE(ft.getBound()->isShadowMapFBO());
}

TEST(FramebufferTracker, DeleteFBO) {
    FramebufferTracker ft;
    ft.add(1, std::make_shared<FramebufferMetadata>(1));
    ft.bind(1);
    ft.getBound()->attach(GL_DEPTH_ATTACHMENT, 0);
    EXPECT_TRUE(ft.getBound()->isShadowMapFBO());
    EXPECT_TRUE(ft.has(1));

    ft.remove(1);
    EXPECT_FALSE(ft.has(1));
    EXPECT_EQ(ft.getBoundId(),0);
    EXPECT_TRUE(ft.isSuitableForRepeating());
}
}
