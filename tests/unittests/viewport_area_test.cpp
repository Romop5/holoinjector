#include "gtest/gtest.h"
#include "viewport_area.hpp"

using namespace ve;

namespace {
TEST(ViewportArea, Basic) {
    ve::ViewportArea area(1,2,3,4);
    EXPECT_EQ(area.getX(), 1);
    EXPECT_EQ(area[0], 1);
    EXPECT_EQ(area.getY(), 2);
    EXPECT_EQ(area[1], 2);
    EXPECT_EQ(area.getWidth(), 3);
    EXPECT_EQ(area[2], 3);
    EXPECT_EQ(area.getHeight(), 4);
    EXPECT_EQ(area[3], 4);


    area.set(4,5,6,7);
    EXPECT_EQ(area[0], 4);
    EXPECT_EQ(area[1], 5);
    EXPECT_EQ(area[2], 6);
    EXPECT_EQ(area[3], 7);
}

TEST(ViewportArea, RawPtr) {
    ViewportArea area;
    area.getDataPtr()[0] = 666;
    EXPECT_EQ(area[0],666);
}
}
