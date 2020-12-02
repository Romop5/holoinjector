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
            vs_uv = uvIn;
            float z = uvIn.x;
            if(!isFBO)
                z = 0.0;
            gl_Position = vec4(normal.xy, z, 1.0);
        }
        )",GL_VERTEX_SHADER);

    auto fs = utils::glShader(R"(
        #version 430 core
        out vec4 FragColor;
        in vec2 uv;

        uniform sampler2D tex;
        uniform sampler2D texDepth;

        uniform bool isFBO = false;
        uniform float time = 0.0;

        
        vec2 paralax(vec2 start, float disparity, float center)
        {
            int maxSteps = 60;
            float stepSize = 1.0/maxSteps;
            // when disparity is 0, depth map is projected orthogonally,
            // thus ray should have zero horizontal movement when stepping
            // on the other hand, when disparity increases, viewer see
            // scene from the angle.
            float direction = stepSize*disparity;
            //float direction = -(start.x-0.5)*stepSize*disparity;

            vec2 currentPos = start-vec2(disparity*center,0.0);
            float currentDepth = 0.0;
            float depth = texture(texDepth, currentPos).x;

            while(currentDepth < depth)
            {
                currentPos.x += direction;
                depth = texture(texDepth, currentPos).x;
                currentDepth += stepSize;
            }

            float positionBeforeLastStep = currentPos.x - direction;
            float depthBeforeLastStep = texture(texDepth, positionBeforeLastStep).x;
            // how much depth changed when we moved along X axis in depth map
            float depthStep = depth-depthBeforeLastStep;
            // how much ray traced depth differ from last depth
            float depthDifference = currentDepth-depthBeforeLastStep;
            // ratio for traced depth vs step
            float weight = depthDifference/depthStep;
            if(currentPos.x > 1.0)
                currentPos.x = 1.0;
            if(currentPos.x < 0.0)
                currentPos.x = 0.0;
            return currentPos;
        }
       

        uniform float disparity = 0.0;
        void main()
        {
            vec4 color = texture(tex, uv);
            vec4 depth = texture(texDepth,uv);
            if(isFBO)
            {
                color.x = 0.2*gl_Layer;
            }
            FragColor = vec4(color.xyz, 0.5);
            if(!isFBO)
            {
                FragColor.z = depth.x*1.0;
                float center = time;
                //float center = 0.0;
                FragColor = texture(tex, paralax(uv, disparity, center));
            }
        }
    )", GL_FRAGMENT_SHADER);

    m_program = std::make_unique<ve::utils::glProgram>(std::move(vs), std::move(fs));
}

void ve::paralax::Mapping::bindInputDepthBuffer(size_t bufferID)
{
    setUniform1i("texDepth", bufferID);
}

void ve::paralax::Mapping::bindInputColorBuffer(size_t bufferID)
{
    setUniform1i("tex", bufferID);
}

void ve::paralax::Mapping::draw(size_t windowsX, size_t windowsY, float disparityRatio, float centerRatio)
{
    setUniform1i("windowsX", windowsX); 
    setUniform1i("windowsY", windowsY); 
    setUniform1i("disparityRatio", disparityRatio); 
    setUniform1i("centerRatio", centerRatio); 
    m_VAO->draw();
}

void ve::paralax::Mapping::setUniform1i(const std::string& name, size_t value)
{
    auto progID = m_program->getID();
    size_t texLocation = glGetUniformLocation(progID, name.c_str());
    glUseProgram(progID);
    glUniform1i(texLocation, value);
}
