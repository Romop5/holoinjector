#include <cassert>
#include <string>

#define GL_GLEXT_PROTOTYPES 1
#include "GL/gl.h"

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
        Logger::log("[Repeater] error:", error);
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
        Logger::log("[Repeater] Failed to create FBO for layered rendering: Status: glEnum: ", status);
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
        uniform bool shouldDisplayGrid = false;
        uniform bool shouldSingleViewQuilt= false;
        uniform int singleViewID = 0;
        in vec2 uv;
        out vec4 color;

        void renderGrid()
        {
            if(shouldSingleViewQuilt)
            {
                color = texture(enhancer_layeredScreen, vec3(uv, singleViewID));
            } else {
                vec2 newUv = mod(vec2(gridXSize*uv.x, gridYSize*uv.y), 1.0);
                vec2 indicesuv = vec2(uv.x, 1.0-uv.y);
                ivec2 indices = ivec2(int(indicesuv.x*float(gridXSize)),int(indicesuv.y*float(gridYSize)));
                int layer = (gridYSize-indices.y-1)*gridXSize+indices.x;

                color = texture(enhancer_layeredScreen, vec3(newUv, layer));
            }
            color.w = 1.0;
        }

        //-----------------------------------------------
         // HoloPlay values
          uniform float pitch = 354.677;
          uniform float tilt = -0.113949f;
          uniform float center = -0.400272;
          uniform float subp = 0.00013f;
          uniform vec4 viewPortion = vec4(0.99976f, 0.99976f, 0.00f, 0.00f);
          uniform uint drawOnlyOneImage = 0;
          uniform int ri = 0;
          uniform int bi = 2;

          vec3 texArr(vec3 uvz)
          {
              int layersCount = gridXSize*gridYSize;
              // decide which section to take from based on the z.
              float z = floor(uvz.z * layersCount);
              float x = (mod(z, gridXSize) + uvz.x) / gridXSize;
              float y = (floor(z / gridYSize) + uvz.y) / gridYSize;
              vec2 finalXY = vec2(x, y) * viewPortion.xy;
              //return vec3(finalXY, z);
              return vec3(uvz.x,uvz.y,z);
          }
     
          
          void renderLookingGlass()
          {
                int countOfLayers = gridXSize*gridYSize;
                vec2 texCoords = uv;

                vec3 nuv = vec3(texCoords.xy, 0.0);
          
                vec4 rgb[3];
                for (int i=0; i < 3; i++)
                {
                    nuv.z = (texCoords.x + i * subp + texCoords.y * tilt) * pitch - center;
                    nuv.z = fract(nuv.z);
                    nuv.z = (1.0 - nuv.z);
                    vec3 quiltTextureCoords = texArr(nuv);
                    if(shouldSingleViewQuilt && quiltTextureCoords.z != singleViewID)
                    {
                        rgb[i] = vec4(0.0);
                    }
                    else {
                        rgb[i] = texture(enhancer_layeredScreen, quiltTextureCoords);
                    }
                }

                color = vec4(rgb[ri].r, rgb[1].g, rgb[bi].b, 1.0);
          }

        //-----------------------------------------------
        void main()
        {
            if(shouldDisplayGrid)
            {
                renderGrid();
            } else {
                renderLookingGlass();
            }
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

    setHoloDisplayParameters(HoloDisplayParameters{});
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

void OutputFBO::toggleGridView()
{
    shouldDisplayGrid = !shouldDisplayGrid;
}

void OutputFBO::toggleSingleViewGridView()
{
    shouldDisplayOnlySingleQuiltImage = !shouldDisplayOnlySingleQuiltImage;
}


void OutputFBO::setOnlyQuiltImageID(size_t id)
{
    m_OnlyQuiltImageID = id;
}

const HoloDisplayParameters OutputFBO::getHoloDisplayParameters() const
{
    return m_HoloParameters;
}

void OutputFBO::setHoloDisplayParameters(const HoloDisplayParameters params)
{
    if(m_HoloParameters.m_Pitch != params.m_Pitch)
    {
        glUniform1f(glGetUniformLocation(m_ViewerProgram,"pitch"),params.m_Pitch);
    }

    if(m_HoloParameters.m_Tilt != params.m_Tilt)
    {
        glUniform1f(glGetUniformLocation(m_ViewerProgram,"til"),params.m_Tilt);
    }

    if(m_HoloParameters.m_Center != params.m_Center)
    {
        glUniform1f(glGetUniformLocation(m_ViewerProgram,"center"),params.m_Center);
    }

    if(m_HoloParameters.m_SubpixelSize != params.m_SubpixelSize)
    {
        glUniform1f(glGetUniformLocation(m_ViewerProgram,"subp"),params.m_SubpixelSize);
    }

    m_HoloParameters = params;
}

void OutputFBO::renderGridLayout()
{
    glUseProgram(m_ViewerProgram);
    auto gridXLocation = glGetUniformLocation(m_ViewerProgram, "gridXSize");
    glUniform1i(gridXLocation, m_Params.getGridSizeX());

    auto gridYLocation = glGetUniformLocation(m_ViewerProgram, "gridYSize");
    glUniform1i(gridYLocation, m_Params.getGridSizeY());

    auto gridToggleLocation = glGetUniformLocation(m_ViewerProgram, "shouldDisplayGrid");
    glUniform1i(gridToggleLocation, shouldDisplayGrid);

    auto singleViewQuilt = glGetUniformLocation(m_ViewerProgram, "shouldSingleViewQuilt");
    glUniform1i(singleViewQuilt, shouldDisplayOnlySingleQuiltImage);

    auto quiltSingle = glGetUniformLocation(m_ViewerProgram,"singleViewID");
    glUniform1i(quiltSingle, m_OnlyQuiltImageID);

    auto& params = m_HoloParameters;
    glUniform1f(glGetUniformLocation(m_ViewerProgram,"pitch"),params.m_Pitch);
    glUniform1f(glGetUniformLocation(m_ViewerProgram,"tilt"),params.m_Tilt);
    glUniform1f(glGetUniformLocation(m_ViewerProgram,"center"),params.m_Center);

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
