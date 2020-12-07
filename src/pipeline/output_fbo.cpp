#include <cassert>
#include <string>

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"
#include "GL/glext.h"

#include "pipeline/output_fbo.hpp"
#include "pipeline/camera_parameters.hpp"
#include "utils/opengl_objects.hpp"

#include "logger.hpp"


using namespace ve;
using namespace ve::pipeline;

//-----------------------------------------------------------------------------
// OutputFBOParameters
//-----------------------------------------------------------------------------
size_t OutputFBOParameters::getTextureWidth() const
{
    return pixels_width;
}
size_t OutputFBOParameters::getTextureHeight() const
{
    return pixels_height;
}
size_t OutputFBOParameters::getLayers() const
{
    return gridXSize*gridYSize;
}
size_t OutputFBOParameters::getGridSizeX() const
{
    return gridXSize;
}
size_t OutputFBOParameters::getGridSizeY() const
{
    return gridYSize;
}
//-----------------------------------------------------------------------------
// OutputFBO
//-----------------------------------------------------------------------------
OutputFBO::~OutputFBO()
{
    deinitialize();
}

void OutputFBO::initialize(OutputFBOParameters params)
{
    // store params
    m_Params = params;

    auto countOfLayers = params.getLayers();
    for(size_t i = 0; i < countOfLayers; i++)
        m_proxyFBO.push_back(std::move(ve::utils::FBORAII(0)));

    glGenFramebuffers(1, &m_FBOId); 
    assert(glGetError() == GL_NO_ERROR);
    glBindFramebuffer(GL_FRAMEBUFFER,m_FBOId);
    assert(glGetError() == GL_NO_ERROR);

    glGenTextures(1, &m_LayeredColorBuffer);
    assert(glGetError() == GL_NO_ERROR);
    glGenTextures(1, &m_LayeredDepthStencilBuffer);
    assert(glGetError() == GL_NO_ERROR);


    glBindTexture(GL_TEXTURE_2D_ARRAY, m_LayeredColorBuffer);
    assert(glGetError() == GL_NO_ERROR);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_RGBA8, params.getTextureWidth(),params.getTextureHeight(), countOfLayers);
    assert(glGetError() == GL_NO_ERROR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_LayeredColorBuffer, 0);
    auto error = glGetError();
    if(error != GL_NO_ERROR)
    {
        Logger::log("[Repeater] error: {}\n", error);
    }
    assert(error == GL_NO_ERROR);

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_LayeredDepthStencilBuffer);
    glTexStorage3D(GL_TEXTURE_2D_ARRAY, 1, GL_DEPTH24_STENCIL8, params.getTextureWidth(),params.getTextureHeight(), countOfLayers);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    assert(glGetError() == GL_NO_ERROR);
    glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, m_LayeredDepthStencilBuffer, 0);
    assert(glGetError() == GL_NO_ERROR);
    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);

    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE)
    {
        Logger::log("[Repeater] Failed to create FBO for layered rendering: Status: glEnum {}\n", status);
    }
    assert(status == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER,0);


    /*
     * Create shader program for displaying layered color buffer
     */

    auto VS = std::string(R"(
        #version 440 core
        layout (location = 0) in vec3 position;

        out vec2 uv;
        void main()
        {
            gl_Position = vec4(position,1.0);
            uv = (position.xy+vec2(1.0))/2.0f;
        }
    )");

    auto FS = std::string(R"(
        #version 440 core
        uniform sampler2DArray enhancer_layeredScreen;
        uniform int gridXSize = 3;
        uniform int gridYSize = 3;
        in vec2 uv;
        out vec4 color;

        void main()
        {
            vec2 newUv = mod(vec2(gridXSize*uv.x, gridYSize*uv.y), 1.0);
            ivec2 indices = ivec2(int(uv.x*float(gridXSize)),int(uv.y*float(gridYSize)));
            int layer = (gridYSize-indices.y-1)*gridXSize+indices.x;

            color = texture(enhancer_layeredScreen, vec3(newUv, layer));
            color.w = 1.0;
            //color.z = float(layer)/float(gridXSize*gridYSize);
        }
    )");

    auto fs = ve::utils::glShader(FS, GL_FRAGMENT_SHADER);
    assert(fs.getID() != 0);
    auto vs = ve::utils::glShader(VS, GL_VERTEX_SHADER);
    assert(vs.getID() != 0);

    auto program = ve::utils::glProgram(std::move(vs),std::move(fs));
    assert(program.getID() != 0);
    m_ViewerProgram = program.releaseID();

    m_VAO = std::make_shared<ve::utils::glFullscreenVAO>();
}

void OutputFBO::deinitialize()
{
    if(m_FBOId)
    {
        glDeleteFramebuffers(1,&m_FBOId);
        m_FBOId = 0;
    }
    if(m_LayeredColorBuffer)
    {
        glDeleteTextures(1,&m_LayeredColorBuffer);
        m_LayeredColorBuffer = 0;
    }

    if(m_LayeredDepthStencilBuffer)
    {
        glDeleteTextures(1,&m_LayeredDepthStencilBuffer);
        m_LayeredDepthStencilBuffer = 0;
    }

    if(m_ViewerProgram)
    {
        glDeleteProgram(m_ViewerProgram);
        m_ViewerProgram = 0;
    }
}
void OutputFBO::renderToBackbuffer(const CameraParameters& params)
{
    // Mark FBO as clean
    clearImageFlag();

    GLint oldProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &oldProgram);

    bool result = true;
    if(result)
    {
        renderGridLayout();
    } else {
        renderParalax(params);
    }
    glUseProgram(oldProgram);
}

void OutputFBO::clearBuffers()
{
    glBindFramebuffer(GL_FRAMEBUFFER,m_FBOId);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

GLuint OutputFBO::getFBOId()
{
    return m_FBOId;
}

const OutputFBOParameters& OutputFBO::getParams()
{
    return m_Params;
}

void OutputFBO::setContainsImageFlag()
{
    m_ContainsImageFlag = true;
}

bool OutputFBO::hasImage() const
{
    return m_ContainsImageFlag;
}

void OutputFBO::clearImageFlag()
{
    m_ContainsImageFlag = false;
}

GLuint OutputFBO::createProxyFBO(size_t layer)
{
    // Use cache
    if(layer < m_proxyFBO.size() && m_proxyFBO[layer].getID() != 0)
    {
        return m_proxyFBO[layer].getID();
    }

    // Assert that OutputFBO has already been initialized
    assert(m_FBOId != 0);
    assert(m_LayeredColorBuffer != 0);
    assert(m_LayeredDepthStencilBuffer != 0);

    GLuint proxyFBO;
    glGenFramebuffers(1,&proxyFBO);
    // Hack: OpenGL require at least one bind before attaching
    glBindFramebuffer(GL_FRAMEBUFFER,proxyFBO);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, m_LayeredColorBuffer,0, layer);

    glBindTexture(GL_TEXTURE_2D_ARRAY, m_LayeredDepthStencilBuffer);
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,m_LayeredDepthStencilBuffer,0,layer);
    auto status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    glBindFramebuffer(GL_FRAMEBUFFER,0);
    assert(status == GL_FRAMEBUFFER_COMPLETE);

    m_proxyFBO[layer] = std::move(ve::utils::FBORAII(proxyFBO));
    assert(m_proxyFBO[layer].getID() == proxyFBO);
    return m_proxyFBO[layer].getID();
}

void OutputFBO::renderGridLayout()
{
    glUseProgram(m_ViewerProgram);
    auto gridXLocation = glGetUniformLocation(m_ViewerProgram, "gridXSize");
    glUniform1i(gridXLocation, m_Params.getGridSizeX());

    auto gridYLocation = glGetUniformLocation(m_ViewerProgram, "gridYSize");
    glUniform1i(gridYLocation, m_Params.getGridSizeY());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D_ARRAY, m_LayeredColorBuffer);
    m_VAO->draw();
    glBindFramebuffer(GL_FRAMEBUFFER,m_FBOId);

};

void OutputFBO::renderParalax(const CameraParameters& params)
{
    static GLuint m_colorBuffer = 0;
    static GLuint m_depthBuffer = 0;
    if(!m_colorBuffer || !m_depthBuffer)
    {
        size_t layers = m_Params.gridXSize*m_Params.gridYSize;
        size_t zeroLayer = layers/2;
        glGenTextures(1, &m_colorBuffer);
        glTextureView(m_colorBuffer, GL_TEXTURE_2D, m_LayeredColorBuffer, GL_RGBA8,0,1,zeroLayer,1);

        glGenTextures(1, &m_depthBuffer);
        glTextureView(m_depthBuffer, GL_TEXTURE_2D, m_LayeredDepthStencilBuffer, GL_DEPTH24_STENCIL8,0,1,zeroLayer,1);
        m_Pm.initializeResources();
    }

    /*
     * TODO: make sure that TU 78/79 are not used by native app
     */
    glActiveTexture(GL_TEXTURE0+79);
    glBindTexture(GL_TEXTURE_2D, m_colorBuffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glActiveTexture(GL_TEXTURE0+78);
    glBindTexture(GL_TEXTURE_2D, m_depthBuffer);
    // Hack: needed for DEPTH+STENCIL texture
    glTexParameteri (GL_TEXTURE_2D, GL_DEPTH_STENCIL_TEXTURE_MODE, GL_DEPTH_COMPONENT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glActiveTexture(GL_TEXTURE0);

    m_Pm.bindInputColorBuffer(79);
    m_Pm.bindInputDepthBuffer(78);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    const auto disparityRatio = std::max(0.0,1.0-1.0/params.m_XShiftMultiplier);
    const auto centerRatio = std::max(0.0,1.0-1.0/params.m_frontOpticalAxisCentreDistance);
    m_Pm.draw(m_Params.getGridSizeX(), m_Params.getGridSizeY(), disparityRatio, centerRatio);
    glBindFramebuffer(GL_FRAMEBUFFER,m_FBOId);
}
