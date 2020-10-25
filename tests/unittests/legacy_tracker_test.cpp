#include "gtest/gtest.h"
#include "trackers/legacy_tracker.hpp"

#include <glm/gtx/transform.hpp>
#include <glm/gtx/string_cast.hpp>

using namespace ve;
namespace helper {
    static bool areMatricesSame(const glm::mat4& a, const glm::mat4& b, float eps = 1e-6)
    {
        for(size_t i = 0; i < 4; i++)
        {
            for(size_t j = 0; j < 4; j++)
            {
                if(std::abs(a[i][j]-b[i][j]) > eps)
                    return false;
            }
        }
        return true;
    }
}

namespace {
TEST(LegacyTracker, Basic) {
    LegacyTracker tracker;
    ASSERT_FALSE(tracker.isLegacyNeeded());

    tracker.matrixMode(GL_PROJECTION);
    ASSERT_EQ(tracker.getMatrixMode(), GL_PROJECTION);

    tracker.matrixMode(GL_MODELVIEW);
    ASSERT_NE(tracker.getMatrixMode(), GL_PROJECTION);

    ASSERT_EQ(tracker.getProjection(), glm::mat4(1.0));

    tracker.matrixMode(GL_PROJECTION);
    auto rotationMatrix = glm::rotate(glm::radians(90.0f), glm::vec3(1.0,0.0,0.0));
    std::cout << "Rotation 90 degrees: " << glm::to_string(rotationMatrix) << std::endl;
    auto rotationMatrix180 = glm::rotate(glm::radians(180.0f), glm::vec3(1.0,0.0,0.0));
    tracker.multMatrix(rotationMatrix);
    ASSERT_NE(tracker.getProjection(), glm::mat4(1.0));
    ASSERT_EQ(tracker.getProjection(), rotationMatrix);


    tracker.multMatrix(rotationMatrix);
    std::cout << "Projection: " << glm::to_string(tracker.getProjection()) << std::endl;
    std::cout << "Rotation180: " << glm::to_string(rotationMatrix180) << std::endl;
    ASSERT_TRUE(helper::areMatricesSame(tracker.getProjection(), rotationMatrix180));

    tracker.multMatrix(rotationMatrix);
    tracker.multMatrix(rotationMatrix);

    // 4 times rotation around 90 degrees => should be identity right now
    ASSERT_TRUE(helper::areMatricesSame(tracker.getProjection(), glm::mat4(1.0)));
    tracker.loadMatrix(tracker.getProjection());
    ASSERT_TRUE(helper::areMatricesSame(tracker.getProjection(), glm::mat4(1.0)));

    // Should be orthogonal
    ASSERT_TRUE(tracker.isOrthogonalProjection());
}
}
