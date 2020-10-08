#include "gtest/gtest.h"
#include <glm/gtx/transform.hpp>  // glm::translate etc.
#include <glm/glm.hpp>
#include "projection_estimator.hpp"
#include <glm/gtx/string_cast.hpp>

// Uncomment to get more information about transformations being processed
#define SHOULD_BE_VERBOSE 

#ifdef SHOULD_BE_VERBOSE
#define OUTPUTTER std::cout 
#else
#define OUTPUTTER emptyOut
#endif

#define ASSERT_NEAR_RELATIVE(a,b,c) ASSERT_NEAR(a,b,glm::abs(c*a))

struct EmptyOutputter
{
    template<typename T>
    EmptyOutputter& operator<<(T object)
    {
        static_cast<void>(object);
        return (*this);
    }
} emptyOut;


bool areFloatsSame(float a, float b, float eps = 1e-3)
{
    return (abs(a-b) < eps);
}

float calculateFX(float deg, float aspectRatio = 1.0)
{
    return (1.0f/(tan(glm::radians(deg)/2.0)*aspectRatio));
}

#define INVERSE_TAN_DEGREES(deg) (1.0f/tan(glm::radians(


#define TEST_PERSPECTIVE(angleDegrees, aspect, near, far)\
{\
    auto estimation = ve::estimatePerspectiveProjection(mvp);\
    ASSERT_NEAR_RELATIVE(estimation.fy, calculateFX(angleDegrees),1e-3);\
    ASSERT_NEAR_RELATIVE(estimation.fx, calculateFX(angleDegrees, aspect),1e-3);\
\
    ASSERT_NEAR_RELATIVE(estimation.A, projection[2][2],1e-3);\
    ASSERT_NEAR_RELATIVE(estimation.B, projection[3][2],1e-3);\
\
    ASSERT_NEAR_RELATIVE(estimation.nearPlane, near,1e-3);\
    ASSERT_NEAR_RELATIVE(estimation.farPlane, far,1e-1);\
    ASSERT_EQ(estimation.isPerspective, true);\
\
    OUTPUTTER << "Params: " << angleDegrees << ", " << aspect << ", " << near << ", " << far << "\n";\
    OUTPUTTER << "MVP: " << glm::to_string(mvp) << "\n";\
    OUTPUTTER << "PROJ: " << glm::to_string(projection) << "\n";\
    OUTPUTTER << "Estimated params: " << estimation.fx << "," << estimation.fy << ", " << estimation.A << ", " << estimation.B << ", " << estimation.nearPlane << ", " << estimation.farPlane << "\n";\
}

#define MAKE_PERSPECTIVE(angleDegrees, aspect, near, far)\
    glm::mat4 projection = glm::perspective(glm::radians(angleDegrees), aspect, near,far);\

#define TEST_MV_PERSPECTIVE(mvMatrixName, angleDegrees, aspect, near, far)\
{\
    OUTPUTTER << "===" << "\n";\
    OUTPUTTER << "Running test: " << "" # mvMatrixName << "\n";\
    MAKE_PERSPECTIVE(angleDegrees, aspect, near, far)\
    auto mvp = projection*mvMatrixName;\
    TEST_PERSPECTIVE(angleDegrees, aspect, near, far)\
    OUTPUTTER << "===" << "\n";\
}

glm::mat4 ROT_X(float angle) { return glm::rotate(angle, glm::vec3(1.0,0.0,0.0)); }
glm::mat4 ROT_Y(float angle) { return glm::rotate(angle, glm::vec3(0.0,1.0,0.0)); }
glm::mat4 ROT_Z(float angle) { return glm::rotate(angle, glm::vec3(0.0,0.0,1.0)); }

glm::mat4 TRANSL(float x,float y, float z){ return glm::translate(glm::vec3(x,y,z)); }
glm::mat4 SCALE(float s) { return glm::scale(glm::vec3(s)); }

namespace {
TEST(ProjectionEstimator, AsVector) {
    MAKE_PERSPECTIVE(90.0, 2.0, 0.1,100.0);
    auto params = ve::estimatePerspectiveProjection(projection);

    auto paramsVec = params.asVector();
    ASSERT_NEAR_RELATIVE(paramsVec[1], calculateFX(90.0),1e-3);
    ASSERT_NEAR_RELATIVE(paramsVec[0], calculateFX(90.0, 2.0),1e-3);
    ASSERT_NEAR_RELATIVE(paramsVec[2], 0.1,1e-3);
    ASSERT_NEAR_RELATIVE(paramsVec[3], 100,1e-3);

}
TEST(ProjectionEstimator, Identity) {
    const auto IDENTITY = glm::identity<glm::mat4>();

    TEST_MV_PERSPECTIVE(IDENTITY,20.0,1.5,0.1,1000.0);
    TEST_MV_PERSPECTIVE(IDENTITY,25.0,1.5,0.1,1000.0);
    TEST_MV_PERSPECTIVE(IDENTITY,25.0,1.5,0.1,5.0);
    TEST_MV_PERSPECTIVE(IDENTITY,56.0,4.5,0.1,500.0);
    TEST_MV_PERSPECTIVE(IDENTITY,16.0,4.5,0.1,500.0);
    TEST_MV_PERSPECTIVE(IDENTITY,0.1,4.5,0.1,500.0);

}
TEST(ProjectionEstimator, Rotation) {
    TEST_MV_PERSPECTIVE(ROT_X(1.0)*ROT_Y(3.0),20.0,1.5,0.5,100.0);

    TEST_MV_PERSPECTIVE(ROT_X(5)*ROT_Y(2.0),20.0,1.5,0.1,1000.0);
    TEST_MV_PERSPECTIVE(ROT_X(5)*ROT_Y(2.0),25.0,1.5,0.1,1000.0);
    TEST_MV_PERSPECTIVE(ROT_X(5)*ROT_Y(2.0),25.0,1.5,0.1,5.0);
    TEST_MV_PERSPECTIVE(ROT_X(5)*ROT_Y(2.0),56.0,4.5,0.1,500.0);
    TEST_MV_PERSPECTIVE(ROT_X(5)*ROT_Y(2.0),16.0,4.5,0.1,500.0);
    TEST_MV_PERSPECTIVE(ROT_X(5)*ROT_Y(2.0),0.1,4.5,0.1,500.0);

}
TEST(ProjectionEstimator, Scale) {

    TEST_MV_PERSPECTIVE(SCALE(3.0)*ROT_X(5)*ROT_Y(2.0),0.1,4.5,0.1,500.0);
    TEST_MV_PERSPECTIVE(ROT_Y(2.130)*SCALE(3.0)*ROT_X(5)*ROT_Y(2.0),0.1,4.5,0.1,500.0);

    TEST_MV_PERSPECTIVE(ROT_Y(2.130)*SCALE(3.0)*ROT_X(5)*ROT_Y(2.0),20.0,1.5,0.1,1000.0);
    TEST_MV_PERSPECTIVE(ROT_Y(2.130)*SCALE(3.0)*ROT_X(5)*ROT_Y(2.0),25.0,1.5,0.1,1000.0);
    TEST_MV_PERSPECTIVE(ROT_Y(2.130)*SCALE(3.0)*ROT_X(5)*ROT_Y(2.0),25.0,1.5,0.1,5.0);
    TEST_MV_PERSPECTIVE(ROT_Y(2.130)*SCALE(3.0)*ROT_X(5)*ROT_Y(2.0),56.0,4.5,0.1,500.0);
    TEST_MV_PERSPECTIVE(ROT_Y(2.130)*SCALE(3.0)*ROT_X(5)*ROT_Y(2.0),16.0,4.5,0.1,500.0);
    TEST_MV_PERSPECTIVE(ROT_Y(2.130)*SCALE(3.0)*ROT_X(5)*ROT_Y(2.0),0.1,4.5,0.1,500.0);

}

TEST(ProjectionEstimator, OrthogonalProjection) {
    auto ortho = glm::mat4(glm::vec4(1.0,0.0,0.0,0.0), glm::vec4(0.0,1.0,0.0,0.0), glm::vec4(0.0,0.0,5.0,0.0), glm::vec4(0.0,0.0,1.0,1.0)); 

    auto projection = ve::estimatePerspectiveProjection(ortho);
    ASSERT_EQ(projection.isPerspective, false);
    ASSERT_EQ(projection.fx, 1.0);
    ASSERT_EQ(projection.fy, 1.0);
}
}



