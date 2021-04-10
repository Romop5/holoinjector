/*****************************************************************************
*
*  PROJECT:     HoloInjector - https://github.com/Romop5/holoinjector
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        paralax/mapping.cpp
*
*****************************************************************************/

#include "paralax/mapping.hpp"
#include "utils/opengl_objects.hpp"

void ve::paralax::Mapping::initializeResources()
{
    auto vs = utils::glShader(R"(
        #version 330 core
        layout(location = 0) in vec3 normal;
        layout(location = 1) in vec2 uvIn;
        uniform bool isFBO = false;

        out vec2 uv;
        void main()
        {
            uv = uvIn;
            float z = uvIn.x;
            if(!isFBO)
                z = 0.0;
            gl_Position = vec4(normal.xy, z, 1.0);
        }
        )",
        GL_VERTEX_SHADER);

    auto fs = utils::glShader(R"(
        #version 430 core
        out vec4 FragColor;
        in vec2 uv;

        uniform sampler2D tex;
        uniform sampler2D texDepth;

        uniform bool isFBO = false;
        uniform float time = 0.0;


        float getDepth(vec2 pos)
        {
            float depth = texture(texDepth, pos).x;

            if(depth > 1.0)
                return 1.0;
            if(depth < 0.0)
                return 0.0;


            // Linearize
            const float C = 0.01;
            depth = (exp(depth * log(C + 1.0)) - 1.0) / C;

            return depth;
        }
        vec3 paralax(vec2 start, float disparity, float center)
        {
            int maxSteps = 60;
            const float GapOffset = 01.0;
            float stepSize = 1.0/maxSteps;
            // when disparity is 0, depth map is projected orthogonally,
            // thus ray should have zero horizontal movement when stepping
            // on the other hand, when disparity increases, viewer see
            // scene from the angle.
            float direction = stepSize*disparity;

            vec2 currentPos = start-vec2(disparity*center,0.0);
            float currentDepth = 0.0;
            float depth = getDepth(currentPos);

            while(currentDepth < depth)
            {
                currentPos.x += direction;
                depth = getDepth(currentPos);
                currentDepth += stepSize;
            }

            vec2 positionBeforeLastStep = currentPos - vec2(direction,0.0);
            float depthBeforeLastStep = getDepth(positionBeforeLastStep);
            float before =  depthBeforeLastStep - (currentDepth - stepSize);
            float after = currentDepth-depth;
            float DepthDifference = depthBeforeLastStep - depth;

            float weight = before / (after+before);
            currentPos = weight*currentPos + (1-weight)*positionBeforeLastStep;

            const float pixelSize = 1.0/1024.0;
            // Apply gap masking (by JMF)
            DepthDifference *= GapOffset * disparity * 100.0;
            DepthDifference *= pixelSize; // Replace function
            currentPos.x += DepthDifference;

            //return vec2(weight, 0.0);
            //return currentPos;
            return vec3(currentPos, weight);
        }
       

        uniform float centerRatio = 1.0;
        uniform float disparityRatio = 0.1;
        uniform int gridXSize = 1;
        uniform int gridYSize = 1;
        void main()
        {
            vec2 newUv = mod(vec2(gridXSize*uv.x, gridYSize*uv.y), 1.0);
            ivec2 indices = ivec2(int(uv.x*float(gridXSize)),int(uv.y*float(gridYSize)));
            int layer = (gridYSize-indices.y-1)*gridXSize+indices.x;
            int maxLayers = gridXSize*gridYSize;
            float disparityOffset = 2.0*(((layer+1.0)/(maxLayers+1.0))-0.5);
            float disparity = disparityOffset*disparityRatio;

            vec4 color = texture(tex, newUv);
            vec4 depth = texture(texDepth, newUv);
            FragColor = vec4(1.0);
            vec3 paraUv = paralax(newUv, -disparity, centerRatio);
            FragColor.xyz = texture(tex, paraUv.xy).xyz;
            //FragColor.x = paraUv.z;
        }
    )",
        GL_FRAGMENT_SHADER);

    m_program = std::make_unique<ve::utils::glProgram>(std::move(vs), std::move(fs));
    m_VAO = std::make_shared<ve::utils::glFullscreenVAO>();
}

void ve::paralax::Mapping::bindInputDepthBuffer(size_t bufferID)
{
    setUniform1i("texDepth", bufferID);
}

void ve::paralax::Mapping::bindInputColorBuffer(size_t bufferID)
{
    setUniform1i("tex", bufferID);
}

void ve::paralax::Mapping::draw(size_t gridXSize, size_t gridYSize, float disparityRatio, float centerRatio)
{
    setUniform1i("gridXSize", gridXSize);
    setUniform1i("gridYSize", gridYSize);
    setUniform1f("disparityRatio", disparityRatio);
    setUniform1f("centerRatio", centerRatio);
    m_VAO->draw();
}

void ve::paralax::Mapping::setUniform1i(const std::string& name, size_t value)
{
    auto progID = m_program->getID();
    size_t texLocation = glGetUniformLocation(progID, name.c_str());
    glUseProgram(progID);
    glUniform1i(texLocation, value);
}

void ve::paralax::Mapping::setUniform1f(const std::string& name, float value)
{
    auto progID = m_program->getID();
    size_t texLocation = glGetUniformLocation(progID, name.c_str());
    glUseProgram(progID);
    glUniform1f(texLocation, value);
}
