#define GL_GLEXT_PROTOTYPES 1
#include <GL/gl.h>

#include "framebuffer_manager.hpp"
#include "context.hpp"
#include "trackers/framebuffer_tracker.hpp"
#include "pipeline/output_fbo.hpp"
#include "pipeline/viewport_area.hpp"

#include "utils/opengl_utils.hpp"
#include "utils/opengl_state.hpp"

using namespace ve;
using namespace ve::managers;

void FramebufferManager::clear(Context& context, GLbitfield mask)
{
    if(context.m_IsMultiviewActivated && context.getFBOTracker().isFBODefault() &&  context.getOutputFBO().hasImage())
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(context.getCurrentViewport().getX(), context.getCurrentViewport().getY(),
                    context.getCurrentViewport().getWidth(), context.getCurrentViewport().getHeight());
        context.getOutputFBO().renderToBackbuffer(context.getCameraParameters());
        context.getOutputFBO().clearBuffers();
    }

    glClear(mask);
}

void FramebufferManager::bindFramebuffer (Context& context, GLenum target, GLuint framebuffer)
{
    context.getFBOTracker().bind(framebuffer);
    if(framebuffer == 0)
    {
        if(context.m_IsMultiviewActivated)
        {
            glBindFramebuffer(target, context.getOutputFBO().getFBOId());
            glViewport(0,0,context.getOutputFBO().getParams().getTextureWidth(), context.getOutputFBO().getParams().getTextureHeight());
        } else {
            glBindFramebuffer(target, 0);
            glViewport(context.getCurrentViewport().getX(), context.getCurrentViewport().getY(),
                    context.getCurrentViewport().getWidth(), context.getCurrentViewport().getHeight());
        }
    } else {
        if(context.m_IsMultiviewActivated)
        {
            auto id = framebuffer;
            auto fbo = context.getFBOTracker().getBound();
            /*
             * Only create & bind shadow FBO when original FBO is complete (thus has any attachment)
             */
            if(fbo->hasAnyAttachment())
            {
                if(!fbo->hasShadowFBO() && context.getFBOTracker().isSuitableForRepeating())
                {
                    fbo->createShadowedFBO(context.getOutputFBO().getParams().getLayers());
                    if(!fbo->hasShadowFBO())
                    {
                        Logger::logError("[Repeater] Failed to create shawow FBO for FBO: ",id, ENHANCER_POS);
                    }
                }
                // Creation of shadow FBO should never fail
                //assert(fbo->hasShadowFBO());
                id = fbo->getShadowFBO();
            }

            glBindFramebuffer(target,id);
            glViewport(context.getCurrentViewport().getX(), context.getCurrentViewport().getY(),
                    context.getCurrentViewport().getWidth(), context.getCurrentViewport().getHeight());

            // TODO: shadowed textures are the same size as OutputFBO
            /* if(context.m_IsMultiviewActivated) */
            /* { */
            /*     glViewport(0,0,context.getOutputFBO().getParams().getTextureWidth(), context.getOutputFBO().getParams().getTextureHeight()); */
            /* } */
        } else {
            glBindFramebuffer(target, framebuffer);
        }
    }
}

void FramebufferManager::swapBuffers(Context& context, std::function<void(void)> swapit,std::function<void(void)> onOverlayRender)
{
    ve::utils::restoreStateFunctor({GL_CULL_FACE, GL_DEPTH_TEST, GL_SCISSOR_TEST},[this, &context]()
    {
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_SCISSOR_TEST);
        glDisable(GL_STENCIL_TEST);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
        if(context.m_IsMultiviewActivated && context.getOutputFBO().hasImage())
        {
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glViewport(context.getCurrentViewport().getX(), context.getCurrentViewport().getY(),
            context.getCurrentViewport().getWidth(), context.getCurrentViewport().getHeight());
            context.getOutputFBO().renderToBackbuffer(context.getCameraParameters());
            context.getOutputFBO().clearBuffers();
        }
    });

    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
    onOverlayRender();
    // Physically do swap buffer
    swapit();
}
