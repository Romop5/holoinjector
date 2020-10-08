#include "gtest/gtest.h"
#include "uniform_block_tracing.hpp"

using namespace ve;

namespace
{
TEST(UniformBlockTracing, Basics) {
    UniformBlockTracing ut;

    // Get random buffer => all should have binding to 0 by default
    ASSERT_FALSE(ut.hasBufferBindingIndex(666));
    ut.setUniformBinding(666,0);
    ASSERT_EQ(ut.getBufferBindingIndex(666),0);
}
}