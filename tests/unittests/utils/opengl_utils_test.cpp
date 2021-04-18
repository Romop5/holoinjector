#include "gtest/gtest.h"
#include "utils/opengl_utils.hpp"
#include <cstdlib>

using namespace hi;
using namespace hi::opengl_utils;

namespace {
TEST(opengl_utils, createMatrixFromRawGL) {
    std::vector<float> rawValues = {
        0.0f, 1.0f, 2.0f, 3.0f,
        4.0f,5.0f,6.0f,7.0f,
        8.0f,9.0f,10.0f,11.0f,
        12.0f,13.0f,14.0f,15.0f
    };
    auto matrix = createMatrixFromRawGL(rawValues.data());
    EXPECT_EQ(matrix[0][0], 0.0f);
    EXPECT_EQ(matrix[0][1], 1.0f);
    EXPECT_EQ(matrix[0][2], 2.0f);
    EXPECT_EQ(matrix[0][3], 3.0f);

    EXPECT_EQ(matrix[1][0], 4.0f);
    EXPECT_EQ(matrix[1][1], 5.0f);
    EXPECT_EQ(matrix[1][2], 6.0f);
    EXPECT_EQ(matrix[1][3], 7.0f);

    EXPECT_EQ(matrix[2][0],  8.0f);
    EXPECT_EQ(matrix[2][1],  9.0f);
    EXPECT_EQ(matrix[2][2], 10.0f);
    EXPECT_EQ(matrix[2][3], 11.0f);

    EXPECT_EQ(matrix[3][0], 12.0f);
    EXPECT_EQ(matrix[3][1], 13.0f);
    EXPECT_EQ(matrix[3][2], 14.0f);
    EXPECT_EQ(matrix[3][3], 15.0f);
}

TEST(opengl_utils, createMatrixFromRawGLDouble) {
    std::vector<double> rawValues = {
        0.0, 1.0, 2.0, 3.0,
        4.0,5.0,6.0,7.0,
        8.0,9.0,10.0,11.0,
        12.0,13.0,14.0,15.0
    };
    auto matrix = createMatrixFromRawGL(rawValues.data());
    EXPECT_EQ(matrix[0][0], 0.0);
    EXPECT_EQ(matrix[0][1], 1.0);
    EXPECT_EQ(matrix[0][2], 2.0);
    EXPECT_EQ(matrix[0][3], 3.0);

    EXPECT_EQ(matrix[1][0], 4.0);
    EXPECT_EQ(matrix[1][1], 5.0);
    EXPECT_EQ(matrix[1][2], 6.0);
    EXPECT_EQ(matrix[1][3], 7.0);

    EXPECT_EQ(matrix[2][0],  8.0);
    EXPECT_EQ(matrix[2][1],  9.0);
    EXPECT_EQ(matrix[2][2], 10.0);
    EXPECT_EQ(matrix[2][3], 11.0);

    EXPECT_EQ(matrix[3][0], 12.0);
    EXPECT_EQ(matrix[3][1], 13.0);
    EXPECT_EQ(matrix[3][2], 14.0);
    EXPECT_EQ(matrix[3][3], 15.0);
}
}
