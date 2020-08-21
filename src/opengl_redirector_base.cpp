/*
 * BEWARE !!!
 *
 * Following .cpp file is heavily overusing macros. 
 * In order not to loose your head, use following sequence of commands
 * to get expanded C++ code in reasonable format:
 *
 * g++ -E opengl_redirector_base.cpp | clang-format 
 */

#include <unordered_map>
#include "opengl_redirector_base.hpp"
#include "opengl_redirector_impl_macros.hpp"

/*
 * To redirect API call to our class method, a static method with same type must be
 * defined, which then access singleton of OpenglRedirectorBase, and routes the
 * call to overloaded function.
 */

using namespace ve;

static OpenglRedirectorBase* g_OpenGLRedirector = nullptr;

OpenglRedirectorBase::OpenglRedirectorBase()
{
    g_OpenGLRedirector = this;
}

namespace helper
{
    static std::unordered_map<std::string, void*> definedAPIFunctions;
    /// Register corresponding OpenGL API rediction into definedAPIFunctions
    class RegisterAPIFunction
    {
        public:
        RegisterAPIFunction(const std::string& name, void* address)
        {
            definedAPIFunctions[name] = address;
        }
    };

    void log_api_call(std::string apiName, ...)
    {
        printf("[Enhancer API CALL: %s]\n", apiName.c_str());
    }
}

/*
 * OPENGL_FORWARD does following:
 *  - a single static function with name implglXYZ, calling OpenglRedirectorBase::XYZ on call
 *  with g_OpenGLRedirector as instance.
 *  - a single method of class OpenglRedirectorBase for class method XYZ
 *  - an instance of RegisterAPIFunction with XYZ and address of implglXYZ
 */
OPENGL_FORWARD(void,glClearIndex,GLfloat,c);
OPENGL_FORWARD(void,glClearColor,GLclampf,red,GLclampf,green,GLclampf,blue,GLclampf,alpha);
OPENGL_FORWARD(void,glClear,GLbitfield,mask);
OPENGL_FORWARD(void,glIndexMask,GLuint,mask);
OPENGL_FORWARD(void,glColorMask,GLboolean,red,GLboolean,green,GLboolean,blue,GLboolean,alpha);
OPENGL_FORWARD(void,glAlphaFunc,GLenum,func,GLclampf,ref);
OPENGL_FORWARD(void,glBlendFunc,GLenum,sfactor,GLenum,dfactor);
OPENGL_FORWARD(void,glLogicOp,GLenum,opcode);
OPENGL_FORWARD(void,glCullFace,GLenum,mode);
OPENGL_FORWARD(void,glFrontFace,GLenum,mode);
OPENGL_FORWARD(void,glPointSize,GLfloat,size);
OPENGL_FORWARD(void,glLineWidth,GLfloat,width);
OPENGL_FORWARD(void,glLineStipple,GLint,factor,GLushort,pattern);
OPENGL_FORWARD(void,glPolygonMode,GLenum,face,GLenum,mode);
OPENGL_FORWARD(void,glPolygonOffset,GLfloat,factor,GLfloat,units);
OPENGL_FORWARD(void,glPolygonStipple,const GLubyte*,mask);
OPENGL_FORWARD(void,glGetPolygonStipple,GLubyte*,mask);
OPENGL_FORWARD(void,glEdgeFlag,GLboolean,flag);
OPENGL_FORWARD(void,glEdgeFlagv,const GLboolean*,flag);
OPENGL_FORWARD(void,glScissor,GLint,x,GLint,y,GLsizei,width,GLsizei,height);
OPENGL_FORWARD(void,glClipPlane,GLenum,plane,const GLdouble*,equation);
OPENGL_FORWARD(void,glGetClipPlane,GLenum,plane,GLdouble*,equation);
OPENGL_FORWARD(void,glDrawBuffer,GLenum,mode);
OPENGL_FORWARD(void,glReadBuffer,GLenum,mode);
OPENGL_FORWARD(void,glEnable,GLenum,cap);
OPENGL_FORWARD(void,glDisable,GLenum,cap);
OPENGL_FORWARD(GLboolean,glIsEnabled,GLenum,cap);
OPENGL_FORWARD(void,glEnableClientState,GLenum,cap);
OPENGL_FORWARD(void,glDisableClientState,GLenum,cap);
OPENGL_FORWARD(void,glGetBooleanv,GLenum,pname,GLboolean*,params);
OPENGL_FORWARD(void,glGetDoublev,GLenum,pname,GLdouble*,params);
OPENGL_FORWARD(void,glGetFloatv,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetIntegerv,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glPushAttrib,GLbitfield,mask);
OPENGL_FORWARD(void,glPopAttrib,void,);
OPENGL_FORWARD(void,glPushClientAttrib,GLbitfield,mask);
OPENGL_FORWARD(void,glPopClientAttrib,void,);
OPENGL_FORWARD(GLint,glRenderMode,GLenum,mode);
OPENGL_FORWARD(GLenum,glGetError,void,);
OPENGL_FORWARD(const GLubyte*,glGetString,GLenum,name);
OPENGL_FORWARD(void,glFinish,void,);
OPENGL_FORWARD(void,glFlush,void,);
OPENGL_FORWARD(void,glHint,GLenum,target,GLenum,mode);
OPENGL_FORWARD(void,glClearDepth,GLclampd,depth);
OPENGL_FORWARD(void,glDepthFunc,GLenum,func);
OPENGL_FORWARD(void,glDepthMask,GLboolean,flag);
OPENGL_FORWARD(void,glDepthRange,GLclampd,near_val,GLclampd,far_val);
OPENGL_FORWARD(void,glClearAccum,GLfloat,red,GLfloat,green,GLfloat,blue,GLfloat,alpha);
OPENGL_FORWARD(void,glAccum,GLenum,op,GLfloat,value);
OPENGL_FORWARD(void,glMatrixMode,GLenum,mode);
OPENGL_FORWARD(void,glOrtho,GLdouble,left,GLdouble,right,GLdouble,bottom,GLdouble,top,GLdouble,near_val,GLdouble,far_val);
OPENGL_FORWARD(void,glFrustum,GLdouble,left,GLdouble,right,GLdouble,bottom,GLdouble,top,GLdouble,near_val,GLdouble,far_val);
OPENGL_FORWARD(void,glViewport,GLint,x,GLint,y,GLsizei,width,GLsizei,height);
OPENGL_FORWARD(void,glPushMatrix,void,);
OPENGL_FORWARD(void,glPopMatrix,void,);
OPENGL_FORWARD(void,glLoadIdentity,void,);
OPENGL_FORWARD(void,glLoadMatrixd,const GLdouble*,m);
OPENGL_FORWARD(void,glLoadMatrixf,const GLfloat*,m);
OPENGL_FORWARD(void,glMultMatrixd,const GLdouble*,m);
OPENGL_FORWARD(void,glMultMatrixf,const GLfloat*,m);
OPENGL_FORWARD(void,glRotated,GLdouble,angle,GLdouble,x,GLdouble,y,GLdouble,z);
OPENGL_FORWARD(void,glRotatef,GLfloat,angle,GLfloat,x,GLfloat,y,GLfloat,z);
OPENGL_FORWARD(void,glScaled,GLdouble,x,GLdouble,y,GLdouble,z);
OPENGL_FORWARD(void,glScalef,GLfloat,x,GLfloat,y,GLfloat,z);
OPENGL_FORWARD(void,glTranslated,GLdouble,x,GLdouble,y,GLdouble,z);
OPENGL_FORWARD(void,glTranslatef,GLfloat,x,GLfloat,y,GLfloat,z);
OPENGL_FORWARD(GLboolean,glIsList,GLuint,list);
OPENGL_FORWARD(void,glDeleteLists,GLuint,list,GLsizei,range);
OPENGL_FORWARD(GLuint,glGenLists,GLsizei,range);
OPENGL_FORWARD(void,glNewList,GLuint,list,GLenum,mode);
OPENGL_FORWARD(void,glEndList,void,);
OPENGL_FORWARD(void,glCallList,GLuint,list);
OPENGL_FORWARD(void,glCallLists,GLsizei,n,GLenum,type,const GLvoid*,lists);
OPENGL_FORWARD(void,glListBase,GLuint,base);
OPENGL_FORWARD(void,glBegin,GLenum,mode);
OPENGL_FORWARD(void,glEnd,void,);
OPENGL_FORWARD(void,glVertex2d,GLdouble,x,GLdouble,y);
OPENGL_FORWARD(void,glVertex2f,GLfloat,x,GLfloat,y);
OPENGL_FORWARD(void,glVertex2i,GLint,x,GLint,y);
OPENGL_FORWARD(void,glVertex2s,GLshort,x,GLshort,y);
OPENGL_FORWARD(void,glVertex3d,GLdouble,x,GLdouble,y,GLdouble,z);
OPENGL_FORWARD(void,glVertex3f,GLfloat,x,GLfloat,y,GLfloat,z);
OPENGL_FORWARD(void,glVertex3i,GLint,x,GLint,y,GLint,z);
OPENGL_FORWARD(void,glVertex3s,GLshort,x,GLshort,y,GLshort,z);
OPENGL_FORWARD(void,glVertex4d,GLdouble,x,GLdouble,y,GLdouble,z,GLdouble,w);
OPENGL_FORWARD(void,glVertex4f,GLfloat,x,GLfloat,y,GLfloat,z,GLfloat,w);
OPENGL_FORWARD(void,glVertex4i,GLint,x,GLint,y,GLint,z,GLint,w);
OPENGL_FORWARD(void,glVertex4s,GLshort,x,GLshort,y,GLshort,z,GLshort,w);
OPENGL_FORWARD(void,glVertex2dv,const GLdouble*,v);
OPENGL_FORWARD(void,glVertex2fv,const GLfloat*,v);
OPENGL_FORWARD(void,glVertex2iv,const GLint*,v);
OPENGL_FORWARD(void,glVertex2sv,const GLshort*,v);
OPENGL_FORWARD(void,glVertex3dv,const GLdouble*,v);
OPENGL_FORWARD(void,glVertex3fv,const GLfloat*,v);
OPENGL_FORWARD(void,glVertex3iv,const GLint*,v);
OPENGL_FORWARD(void,glVertex3sv,const GLshort*,v);
OPENGL_FORWARD(void,glVertex4dv,const GLdouble*,v);
OPENGL_FORWARD(void,glVertex4fv,const GLfloat*,v);
OPENGL_FORWARD(void,glVertex4iv,const GLint*,v);
OPENGL_FORWARD(void,glVertex4sv,const GLshort*,v);
OPENGL_FORWARD(void,glNormal3b,GLbyte,nx,GLbyte,ny,GLbyte,nz);
OPENGL_FORWARD(void,glNormal3d,GLdouble,nx,GLdouble,ny,GLdouble,nz);
OPENGL_FORWARD(void,glNormal3f,GLfloat,nx,GLfloat,ny,GLfloat,nz);
OPENGL_FORWARD(void,glNormal3i,GLint,nx,GLint,ny,GLint,nz);
OPENGL_FORWARD(void,glNormal3s,GLshort,nx,GLshort,ny,GLshort,nz);
OPENGL_FORWARD(void,glNormal3bv,const GLbyte*,v);
OPENGL_FORWARD(void,glNormal3dv,const GLdouble*,v);
OPENGL_FORWARD(void,glNormal3fv,const GLfloat*,v);
OPENGL_FORWARD(void,glNormal3iv,const GLint*,v);
OPENGL_FORWARD(void,glNormal3sv,const GLshort*,v);
OPENGL_FORWARD(void,glIndexd,GLdouble,c);
OPENGL_FORWARD(void,glIndexf,GLfloat,c);
OPENGL_FORWARD(void,glIndexi,GLint,c);
OPENGL_FORWARD(void,glIndexs,GLshort,c);
OPENGL_FORWARD(void,glIndexub,GLubyte,c);
OPENGL_FORWARD(void,glIndexdv,const GLdouble*,c);
OPENGL_FORWARD(void,glIndexfv,const GLfloat*,c);
OPENGL_FORWARD(void,glIndexiv,const GLint*,c);
OPENGL_FORWARD(void,glIndexsv,const GLshort*,c);
OPENGL_FORWARD(void,glIndexubv,const GLubyte*,c);
OPENGL_FORWARD(void,glColor3b,GLbyte,red,GLbyte,green,GLbyte,blue);
OPENGL_FORWARD(void,glColor3d,GLdouble,red,GLdouble,green,GLdouble,blue);
OPENGL_FORWARD(void,glColor3f,GLfloat,red,GLfloat,green,GLfloat,blue);
OPENGL_FORWARD(void,glColor3i,GLint,red,GLint,green,GLint,blue);
OPENGL_FORWARD(void,glColor3s,GLshort,red,GLshort,green,GLshort,blue);
OPENGL_FORWARD(void,glColor3ub,GLubyte,red,GLubyte,green,GLubyte,blue);
OPENGL_FORWARD(void,glColor3ui,GLuint,red,GLuint,green,GLuint,blue);
OPENGL_FORWARD(void,glColor3us,GLushort,red,GLushort,green,GLushort,blue);
OPENGL_FORWARD(void,glColor4b,GLbyte,red,GLbyte,green,GLbyte,blue,GLbyte,alpha);
OPENGL_FORWARD(void,glColor4d,GLdouble,red,GLdouble,green,GLdouble,blue,GLdouble,alpha);
OPENGL_FORWARD(void,glColor4f,GLfloat,red,GLfloat,green,GLfloat,blue,GLfloat,alpha);
OPENGL_FORWARD(void,glColor4i,GLint,red,GLint,green,GLint,blue,GLint,alpha);
OPENGL_FORWARD(void,glColor4s,GLshort,red,GLshort,green,GLshort,blue,GLshort,alpha);
OPENGL_FORWARD(void,glColor4ub,GLubyte,red,GLubyte,green,GLubyte,blue,GLubyte,alpha);
OPENGL_FORWARD(void,glColor4ui,GLuint,red,GLuint,green,GLuint,blue,GLuint,alpha);
OPENGL_FORWARD(void,glColor4us,GLushort,red,GLushort,green,GLushort,blue,GLushort,alpha);
OPENGL_FORWARD(void,glColor3bv,const GLbyte*,v);
OPENGL_FORWARD(void,glColor3dv,const GLdouble*,v);
OPENGL_FORWARD(void,glColor3fv,const GLfloat*,v);
OPENGL_FORWARD(void,glColor3iv,const GLint*,v);
OPENGL_FORWARD(void,glColor3sv,const GLshort*,v);
OPENGL_FORWARD(void,glColor3ubv,const GLubyte*,v);
OPENGL_FORWARD(void,glColor3uiv,const GLuint*,v);
OPENGL_FORWARD(void,glColor3usv,const GLushort*,v);
OPENGL_FORWARD(void,glColor4bv,const GLbyte*,v);
OPENGL_FORWARD(void,glColor4dv,const GLdouble*,v);
OPENGL_FORWARD(void,glColor4fv,const GLfloat*,v);
OPENGL_FORWARD(void,glColor4iv,const GLint*,v);
OPENGL_FORWARD(void,glColor4sv,const GLshort*,v);
OPENGL_FORWARD(void,glColor4ubv,const GLubyte*,v);
OPENGL_FORWARD(void,glColor4uiv,const GLuint*,v);
OPENGL_FORWARD(void,glColor4usv,const GLushort*,v);
OPENGL_FORWARD(void,glTexCoord1d,GLdouble,s);
OPENGL_FORWARD(void,glTexCoord1f,GLfloat,s);
OPENGL_FORWARD(void,glTexCoord1i,GLint,s);
OPENGL_FORWARD(void,glTexCoord1s,GLshort,s);
OPENGL_FORWARD(void,glTexCoord2d,GLdouble,s,GLdouble,t);
OPENGL_FORWARD(void,glTexCoord2f,GLfloat,s,GLfloat,t);
OPENGL_FORWARD(void,glTexCoord2i,GLint,s,GLint,t);
OPENGL_FORWARD(void,glTexCoord2s,GLshort,s,GLshort,t);
OPENGL_FORWARD(void,glTexCoord3d,GLdouble,s,GLdouble,t,GLdouble,r);
OPENGL_FORWARD(void,glTexCoord3f,GLfloat,s,GLfloat,t,GLfloat,r);
OPENGL_FORWARD(void,glTexCoord3i,GLint,s,GLint,t,GLint,r);
OPENGL_FORWARD(void,glTexCoord3s,GLshort,s,GLshort,t,GLshort,r);
OPENGL_FORWARD(void,glTexCoord4d,GLdouble,s,GLdouble,t,GLdouble,r,GLdouble,q);
OPENGL_FORWARD(void,glTexCoord4f,GLfloat,s,GLfloat,t,GLfloat,r,GLfloat,q);
OPENGL_FORWARD(void,glTexCoord4i,GLint,s,GLint,t,GLint,r,GLint,q);
OPENGL_FORWARD(void,glTexCoord4s,GLshort,s,GLshort,t,GLshort,r,GLshort,q);
OPENGL_FORWARD(void,glTexCoord1dv,const GLdouble*,v);
OPENGL_FORWARD(void,glTexCoord1fv,const GLfloat*,v);
OPENGL_FORWARD(void,glTexCoord1iv,const GLint*,v);
OPENGL_FORWARD(void,glTexCoord1sv,const GLshort*,v);
OPENGL_FORWARD(void,glTexCoord2dv,const GLdouble*,v);
OPENGL_FORWARD(void,glTexCoord2fv,const GLfloat*,v);
OPENGL_FORWARD(void,glTexCoord2iv,const GLint*,v);
OPENGL_FORWARD(void,glTexCoord2sv,const GLshort*,v);
OPENGL_FORWARD(void,glTexCoord3dv,const GLdouble*,v);
OPENGL_FORWARD(void,glTexCoord3fv,const GLfloat*,v);
OPENGL_FORWARD(void,glTexCoord3iv,const GLint*,v);
OPENGL_FORWARD(void,glTexCoord3sv,const GLshort*,v);
OPENGL_FORWARD(void,glTexCoord4dv,const GLdouble*,v);
OPENGL_FORWARD(void,glTexCoord4fv,const GLfloat*,v);
OPENGL_FORWARD(void,glTexCoord4iv,const GLint*,v);
OPENGL_FORWARD(void,glTexCoord4sv,const GLshort*,v);
OPENGL_FORWARD(void,glRasterPos2d,GLdouble,x,GLdouble,y);
OPENGL_FORWARD(void,glRasterPos2f,GLfloat,x,GLfloat,y);
OPENGL_FORWARD(void,glRasterPos2i,GLint,x,GLint,y);
OPENGL_FORWARD(void,glRasterPos2s,GLshort,x,GLshort,y);
OPENGL_FORWARD(void,glRasterPos3d,GLdouble,x,GLdouble,y,GLdouble,z);
OPENGL_FORWARD(void,glRasterPos3f,GLfloat,x,GLfloat,y,GLfloat,z);
OPENGL_FORWARD(void,glRasterPos3i,GLint,x,GLint,y,GLint,z);
OPENGL_FORWARD(void,glRasterPos3s,GLshort,x,GLshort,y,GLshort,z);
OPENGL_FORWARD(void,glRasterPos4d,GLdouble,x,GLdouble,y,GLdouble,z,GLdouble,w);
OPENGL_FORWARD(void,glRasterPos4f,GLfloat,x,GLfloat,y,GLfloat,z,GLfloat,w);
OPENGL_FORWARD(void,glRasterPos4i,GLint,x,GLint,y,GLint,z,GLint,w);
OPENGL_FORWARD(void,glRasterPos4s,GLshort,x,GLshort,y,GLshort,z,GLshort,w);
OPENGL_FORWARD(void,glRasterPos2dv,const GLdouble*,v);
OPENGL_FORWARD(void,glRasterPos2fv,const GLfloat*,v);
OPENGL_FORWARD(void,glRasterPos2iv,const GLint*,v);
OPENGL_FORWARD(void,glRasterPos2sv,const GLshort*,v);
OPENGL_FORWARD(void,glRasterPos3dv,const GLdouble*,v);
OPENGL_FORWARD(void,glRasterPos3fv,const GLfloat*,v);
OPENGL_FORWARD(void,glRasterPos3iv,const GLint*,v);
OPENGL_FORWARD(void,glRasterPos3sv,const GLshort*,v);
OPENGL_FORWARD(void,glRasterPos4dv,const GLdouble*,v);
OPENGL_FORWARD(void,glRasterPos4fv,const GLfloat*,v);
OPENGL_FORWARD(void,glRasterPos4iv,const GLint*,v);
OPENGL_FORWARD(void,glRasterPos4sv,const GLshort*,v);
OPENGL_FORWARD(void,glRectd,GLdouble,x1,GLdouble,y1,GLdouble,x2,GLdouble,y2);
OPENGL_FORWARD(void,glRectf,GLfloat,x1,GLfloat,y1,GLfloat,x2,GLfloat,y2);
OPENGL_FORWARD(void,glRecti,GLint,x1,GLint,y1,GLint,x2,GLint,y2);
OPENGL_FORWARD(void,glRects,GLshort,x1,GLshort,y1,GLshort,x2,GLshort,y2);
OPENGL_FORWARD(void,glRectdv,const GLdouble*,v1,const GLdouble*,v2);
OPENGL_FORWARD(void,glRectfv,const GLfloat*,v1,const GLfloat*,v2);
OPENGL_FORWARD(void,glRectiv,const GLint*,v1,const GLint*,v2);
OPENGL_FORWARD(void,glRectsv,const GLshort*,v1,const GLshort*,v2);
OPENGL_FORWARD(void,glVertexPointer,GLint,size,GLenum,type,GLsizei,stride,const GLvoid*,ptr);
OPENGL_FORWARD(void,glNormalPointer,GLenum,type,GLsizei,stride,const GLvoid*,ptr);
OPENGL_FORWARD(void,glColorPointer,GLint,size,GLenum,type,GLsizei,stride,const GLvoid*,ptr);
OPENGL_FORWARD(void,glIndexPointer,GLenum,type,GLsizei,stride,const GLvoid*,ptr);
OPENGL_FORWARD(void,glTexCoordPointer,GLint,size,GLenum,type,GLsizei,stride,const GLvoid*,ptr);
OPENGL_FORWARD(void,glEdgeFlagPointer,GLsizei,stride,const GLvoid*,ptr);
OPENGL_FORWARD(void,glGetPointerv,GLenum,pname,GLvoid**,params);
OPENGL_FORWARD(void,glArrayElement,GLint,i);
OPENGL_FORWARD(void,glDrawArrays,GLenum,mode,GLint,first,GLsizei,count);
OPENGL_FORWARD(void,glDrawElements,GLenum,mode,GLsizei,count,GLenum,type,const GLvoid*,indices);
OPENGL_FORWARD(void,glInterleavedArrays,GLenum,format,GLsizei,stride,const GLvoid*,pointer);
OPENGL_FORWARD(void,glShadeModel,GLenum,mode);
OPENGL_FORWARD(void,glLightf,GLenum,light,GLenum,pname,GLfloat,param);
OPENGL_FORWARD(void,glLighti,GLenum,light,GLenum,pname,GLint,param);
OPENGL_FORWARD(void,glLightfv,GLenum,light,GLenum,pname,const GLfloat*,params);
OPENGL_FORWARD(void,glLightiv,GLenum,light,GLenum,pname,const GLint*,params);
OPENGL_FORWARD(void,glGetLightfv,GLenum,light,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetLightiv,GLenum,light,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glLightModelf,GLenum,pname,GLfloat,param);
OPENGL_FORWARD(void,glLightModeli,GLenum,pname,GLint,param);
OPENGL_FORWARD(void,glLightModelfv,GLenum,pname,const GLfloat*,params);
OPENGL_FORWARD(void,glLightModeliv,GLenum,pname,const GLint*,params);
OPENGL_FORWARD(void,glMaterialf,GLenum,face,GLenum,pname,GLfloat,param);
OPENGL_FORWARD(void,glMateriali,GLenum,face,GLenum,pname,GLint,param);
OPENGL_FORWARD(void,glMaterialfv,GLenum,face,GLenum,pname,const GLfloat*,params);
OPENGL_FORWARD(void,glMaterialiv,GLenum,face,GLenum,pname,const GLint*,params);
OPENGL_FORWARD(void,glGetMaterialfv,GLenum,face,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetMaterialiv,GLenum,face,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glColorMaterial,GLenum,face,GLenum,mode);
OPENGL_FORWARD(void,glPixelZoom,GLfloat,xfactor,GLfloat,yfactor);
OPENGL_FORWARD(void,glPixelStoref,GLenum,pname,GLfloat,param);
OPENGL_FORWARD(void,glPixelStorei,GLenum,pname,GLint,param);
OPENGL_FORWARD(void,glPixelTransferf,GLenum,pname,GLfloat,param);
OPENGL_FORWARD(void,glPixelTransferi,GLenum,pname,GLint,param);
OPENGL_FORWARD(void,glPixelMapfv,GLenum,map,GLsizei,mapsize,const GLfloat*,values);
OPENGL_FORWARD(void,glPixelMapuiv,GLenum,map,GLsizei,mapsize,const GLuint*,values);
OPENGL_FORWARD(void,glPixelMapusv,GLenum,map,GLsizei,mapsize,const GLushort*,values);
OPENGL_FORWARD(void,glGetPixelMapfv,GLenum,map,GLfloat*,values);
OPENGL_FORWARD(void,glGetPixelMapuiv,GLenum,map,GLuint*,values);
OPENGL_FORWARD(void,glGetPixelMapusv,GLenum,map,GLushort*,values);
OPENGL_FORWARD(void,glBitmap,GLsizei,width,GLsizei,height,GLfloat,xorig,GLfloat,yorig,GLfloat,xmove,GLfloat,ymove,const GLubyte*,bitmap);
OPENGL_FORWARD(void,glReadPixels,GLint,x,GLint,y,GLsizei,width,GLsizei,height,GLenum,format,GLenum,type,GLvoid*,pixels);
OPENGL_FORWARD(void,glDrawPixels,GLsizei,width,GLsizei,height,GLenum,format,GLenum,type,const GLvoid*,pixels);
OPENGL_FORWARD(void,glCopyPixels,GLint,x,GLint,y,GLsizei,width,GLsizei,height,GLenum,type);
OPENGL_FORWARD(void,glStencilFunc,GLenum,func,GLint,ref,GLuint,mask);
OPENGL_FORWARD(void,glStencilMask,GLuint,mask);
OPENGL_FORWARD(void,glStencilOp,GLenum,fail,GLenum,zfail,GLenum,zpass);
OPENGL_FORWARD(void,glClearStencil,GLint,s);
OPENGL_FORWARD(void,glTexGend,GLenum,coord,GLenum,pname,GLdouble,param);
OPENGL_FORWARD(void,glTexGenf,GLenum,coord,GLenum,pname,GLfloat,param);
OPENGL_FORWARD(void,glTexGeni,GLenum,coord,GLenum,pname,GLint,param);
OPENGL_FORWARD(void,glTexGendv,GLenum,coord,GLenum,pname,const GLdouble*,params);
OPENGL_FORWARD(void,glTexGenfv,GLenum,coord,GLenum,pname,const GLfloat*,params);
OPENGL_FORWARD(void,glTexGeniv,GLenum,coord,GLenum,pname,const GLint*,params);
OPENGL_FORWARD(void,glGetTexGendv,GLenum,coord,GLenum,pname,GLdouble*,params);
OPENGL_FORWARD(void,glGetTexGenfv,GLenum,coord,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetTexGeniv,GLenum,coord,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glTexEnvf,GLenum,target,GLenum,pname,GLfloat,param);
OPENGL_FORWARD(void,glTexEnvi,GLenum,target,GLenum,pname,GLint,param);
OPENGL_FORWARD(void,glTexEnvfv,GLenum,target,GLenum,pname,const GLfloat*,params);
OPENGL_FORWARD(void,glTexEnviv,GLenum,target,GLenum,pname,const GLint*,params);
OPENGL_FORWARD(void,glGetTexEnvfv,GLenum,target,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetTexEnviv,GLenum,target,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glTexParameterf,GLenum,target,GLenum,pname,GLfloat,param);
OPENGL_FORWARD(void,glTexParameteri,GLenum,target,GLenum,pname,GLint,param);
OPENGL_FORWARD(void,glTexParameterfv,GLenum,target,GLenum,pname,const GLfloat*,params);
OPENGL_FORWARD(void,glTexParameteriv,GLenum,target,GLenum,pname,const GLint*,params);
OPENGL_FORWARD(void,glGetTexParameterfv,GLenum,target,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetTexParameteriv,GLenum,target,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glGetTexLevelParameterfv,GLenum,target,GLint,level,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetTexLevelParameteriv,GLenum,target,GLint,level,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glTexImage1D,GLenum,target,GLint,level,GLint,internalFormat,GLsizei,width,GLint,border,GLenum,format,GLenum,type,const GLvoid*,pixels);
OPENGL_FORWARD(void,glTexImage2D,GLenum,target,GLint,level,GLint,internalFormat,GLsizei,width,GLsizei,height,GLint,border,GLenum,format,GLenum,type,const GLvoid*,pixels);
OPENGL_FORWARD(void,glGetTexImage,GLenum,target,GLint,level,GLenum,format,GLenum,type,GLvoid*,pixels);
OPENGL_FORWARD(void,glGenTextures,GLsizei,n,GLuint*,textures);
OPENGL_FORWARD(void,glDeleteTextures,GLsizei,n,const GLuint*,textures);
OPENGL_FORWARD(void,glBindTexture,GLenum,target,GLuint,texture);
OPENGL_FORWARD(void,glPrioritizeTextures,GLsizei,n,const GLuint*,textures,const GLclampf*,priorities);
OPENGL_FORWARD(GLboolean,glAreTexturesResident,GLsizei,n,const GLuint*,textures,GLboolean*,residences);
OPENGL_FORWARD(GLboolean,glIsTexture,GLuint,texture);
OPENGL_FORWARD(void,glTexSubImage1D,GLenum,target,GLint,level,GLint,xoffset,GLsizei,width,GLenum,format,GLenum,type,const GLvoid*,pixels);
OPENGL_FORWARD(void,glTexSubImage2D,GLenum,target,GLint,level,GLint,xoffset,GLint,yoffset,GLsizei,width,GLsizei,height,GLenum,format,GLenum,type,const GLvoid*,pixels);
OPENGL_FORWARD(void,glCopyTexImage1D,GLenum,target,GLint,level,GLenum,internalformat,GLint,x,GLint,y,GLsizei,width,GLint,border);
OPENGL_FORWARD(void,glCopyTexImage2D,GLenum,target,GLint,level,GLenum,internalformat,GLint,x,GLint,y,GLsizei,width,GLsizei,height,GLint,border);
OPENGL_FORWARD(void,glCopyTexSubImage1D,GLenum,target,GLint,level,GLint,xoffset,GLint,x,GLint,y,GLsizei,width);
OPENGL_FORWARD(void,glCopyTexSubImage2D,GLenum,target,GLint,level,GLint,xoffset,GLint,yoffset,GLint,x,GLint,y,GLsizei,width,GLsizei,height);
OPENGL_FORWARD(void,glMap1d,GLenum,target,GLdouble,u1,GLdouble,u2,GLint,stride,GLint,order,const GLdouble*,points);
OPENGL_FORWARD(void,glMap1f,GLenum,target,GLfloat,u1,GLfloat,u2,GLint,stride,GLint,order,const GLfloat*,points);
OPENGL_FORWARD(void,glMap2d,GLenum,target,GLdouble,u1,GLdouble,u2,GLint,ustride,GLint,uorder,GLdouble,v1,GLdouble,v2,GLint,vstride,GLint,vorder,const GLdouble*,points);
OPENGL_FORWARD(void,glMap2f,GLenum,target,GLfloat,u1,GLfloat,u2,GLint,ustride,GLint,uorder,GLfloat,v1,GLfloat,v2,GLint,vstride,GLint,vorder,const GLfloat*,points);
OPENGL_FORWARD(void,glGetMapdv,GLenum,target,GLenum,query,GLdouble*,v);
OPENGL_FORWARD(void,glGetMapfv,GLenum,target,GLenum,query,GLfloat*,v);
OPENGL_FORWARD(void,glGetMapiv,GLenum,target,GLenum,query,GLint*,v);
OPENGL_FORWARD(void,glEvalCoord1d,GLdouble,u);
OPENGL_FORWARD(void,glEvalCoord1f,GLfloat,u);
OPENGL_FORWARD(void,glEvalCoord1dv,const GLdouble*,u);
OPENGL_FORWARD(void,glEvalCoord1fv,const GLfloat*,u);
OPENGL_FORWARD(void,glEvalCoord2d,GLdouble,u,GLdouble,v);
OPENGL_FORWARD(void,glEvalCoord2f,GLfloat,u,GLfloat,v);
OPENGL_FORWARD(void,glEvalCoord2dv,const GLdouble*,u);
OPENGL_FORWARD(void,glEvalCoord2fv,const GLfloat*,u);
OPENGL_FORWARD(void,glMapGrid1d,GLint,un,GLdouble,u1,GLdouble,u2);
OPENGL_FORWARD(void,glMapGrid1f,GLint,un,GLfloat,u1,GLfloat,u2);
OPENGL_FORWARD(void,glMapGrid2d,GLint,un,GLdouble,u1,GLdouble,u2,GLint,vn,GLdouble,v1,GLdouble,v2);
OPENGL_FORWARD(void,glMapGrid2f,GLint,un,GLfloat,u1,GLfloat,u2,GLint,vn,GLfloat,v1,GLfloat,v2);
OPENGL_FORWARD(void,glEvalPoint1,GLint,i);
OPENGL_FORWARD(void,glEvalPoint2,GLint,i,GLint,j);
OPENGL_FORWARD(void,glEvalMesh1,GLenum,mode,GLint,i1,GLint,i2);
OPENGL_FORWARD(void,glEvalMesh2,GLenum,mode,GLint,i1,GLint,i2,GLint,j1,GLint,j2);
OPENGL_FORWARD(void,glFogf,GLenum,pname,GLfloat,param);
OPENGL_FORWARD(void,glFogi,GLenum,pname,GLint,param);
OPENGL_FORWARD(void,glFogfv,GLenum,pname,const GLfloat*,params);
OPENGL_FORWARD(void,glFogiv,GLenum,pname,const GLint*,params);
OPENGL_FORWARD(void,glFeedbackBuffer,GLsizei,size,GLenum,type,GLfloat*,buffer);
OPENGL_FORWARD(void,glPassThrough,GLfloat,token);
OPENGL_FORWARD(void,glSelectBuffer,GLsizei,size,GLuint*,buffer);
OPENGL_FORWARD(void,glInitNames,void,);
OPENGL_FORWARD(void,glLoadName,GLuint,name);
OPENGL_FORWARD(void,glPushName,GLuint,name);
OPENGL_FORWARD(void,glPopName,void,);
OPENGL_FORWARD(void,glDrawRangeElements,GLenum,mode,GLuint,start,GLuint,end,GLsizei,count,GLenum,type,const GLvoid*,indices);
OPENGL_FORWARD(void,glTexImage3D,GLenum,target,GLint,level,GLint,internalFormat,GLsizei,width,GLsizei,height,GLsizei,depth,GLint,border,GLenum,format,GLenum,type,const GLvoid*,pixels);
OPENGL_FORWARD(void,glTexSubImage3D,GLenum,target,GLint,level,GLint,xoffset,GLint,yoffset,GLint,zoffset,GLsizei,width,GLsizei,height,GLsizei,depth,GLenum,format,GLenum,type,const GLvoid*,pixels);
OPENGL_FORWARD(void,glCopyTexSubImage3D,GLenum,target,GLint,level,GLint,xoffset,GLint,yoffset,GLint,zoffset,GLint,x,GLint,y,GLsizei,width,GLsizei,height);
OPENGL_FORWARD(void,glColorTable,GLenum,target,GLenum,internalformat,GLsizei,width,GLenum,format,GLenum,type,const GLvoid*,table);
OPENGL_FORWARD(void,glColorSubTable,GLenum,target,GLsizei,start,GLsizei,count,GLenum,format,GLenum,type,const GLvoid*,data);
OPENGL_FORWARD(void,glColorTableParameteriv,GLenum,target,GLenum,pname,const GLint*,params);
OPENGL_FORWARD(void,glColorTableParameterfv,GLenum,target,GLenum,pname,const GLfloat*,params);
OPENGL_FORWARD(void,glCopyColorSubTable,GLenum,target,GLsizei,start,GLint,x,GLint,y,GLsizei,width);
OPENGL_FORWARD(void,glCopyColorTable,GLenum,target,GLenum,internalformat,GLint,x,GLint,y,GLsizei,width);
OPENGL_FORWARD(void,glGetColorTable,GLenum,target,GLenum,format,GLenum,type,GLvoid*,table);
OPENGL_FORWARD(void,glGetColorTableParameterfv,GLenum,target,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetColorTableParameteriv,GLenum,target,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glBlendEquation,GLenum,mode);
OPENGL_FORWARD(void,glBlendColor,GLclampf,red,GLclampf,green,GLclampf,blue,GLclampf,alpha);
OPENGL_FORWARD(void,glHistogram,GLenum,target,GLsizei,width,GLenum,internalformat,GLboolean,sink);
OPENGL_FORWARD(void,glResetHistogram,GLenum,target);
OPENGL_FORWARD(void,glGetHistogram,GLenum,target,GLboolean,reset,GLenum,format,GLenum,type,GLvoid*,values);
OPENGL_FORWARD(void,glGetHistogramParameterfv,GLenum,target,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetHistogramParameteriv,GLenum,target,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glMinmax,GLenum,target,GLenum,internalformat,GLboolean,sink);
OPENGL_FORWARD(void,glResetMinmax,GLenum,target);
OPENGL_FORWARD(void,glGetMinmax,GLenum,target,GLboolean,reset,GLenum,format,GLenum,types,GLvoid*,values);
OPENGL_FORWARD(void,glGetMinmaxParameterfv,GLenum,target,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetMinmaxParameteriv,GLenum,target,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glConvolutionFilter1D,GLenum,target,GLenum,internalformat,GLsizei,width,GLenum,format,GLenum,type,const GLvoid*,image);
OPENGL_FORWARD(void,glConvolutionFilter2D,GLenum,target,GLenum,internalformat,GLsizei,width,GLsizei,height,GLenum,format,GLenum,type,const GLvoid*,image);
OPENGL_FORWARD(void,glConvolutionParameterf,GLenum,target,GLenum,pname,GLfloat,params);
OPENGL_FORWARD(void,glConvolutionParameterfv,GLenum,target,GLenum,pname,const GLfloat*,params);
OPENGL_FORWARD(void,glConvolutionParameteri,GLenum,target,GLenum,pname,GLint,params);
OPENGL_FORWARD(void,glConvolutionParameteriv,GLenum,target,GLenum,pname,const GLint*,params);
OPENGL_FORWARD(void,glCopyConvolutionFilter1D,GLenum,target,GLenum,internalformat,GLint,x,GLint,y,GLsizei,width);
OPENGL_FORWARD(void,glCopyConvolutionFilter2D,GLenum,target,GLenum,internalformat,GLint,x,GLint,y,GLsizei,width,GLsizei,height);
OPENGL_FORWARD(void,glGetConvolutionFilter,GLenum,target,GLenum,format,GLenum,type,GLvoid*,image);
OPENGL_FORWARD(void,glGetConvolutionParameterfv,GLenum,target,GLenum,pname,GLfloat*,params);
OPENGL_FORWARD(void,glGetConvolutionParameteriv,GLenum,target,GLenum,pname,GLint*,params);
OPENGL_FORWARD(void,glSeparableFilter2D,GLenum,target,GLenum,internalformat,GLsizei,width,GLsizei,height,GLenum,format,GLenum,type,const GLvoid*,row,const GLvoid*,column);
OPENGL_FORWARD(void,glGetSeparableFilter,GLenum,target,GLenum,format,GLenum,type,GLvoid*,row,GLvoid*,column,GLvoid*,span);
OPENGL_FORWARD(void,glActiveTexture,GLenum,texture);
OPENGL_FORWARD(void,glClientActiveTexture,GLenum,texture);
OPENGL_FORWARD(void,glCompressedTexImage1D,GLenum,target,GLint,level,GLenum,internalformat,GLsizei,width,GLint,border,GLsizei,imageSize,const GLvoid*,data);
OPENGL_FORWARD(void,glCompressedTexImage2D,GLenum,target,GLint,level,GLenum,internalformat,GLsizei,width,GLsizei,height,GLint,border,GLsizei,imageSize,const GLvoid*,data);
OPENGL_FORWARD(void,glCompressedTexImage3D,GLenum,target,GLint,level,GLenum,internalformat,GLsizei,width,GLsizei,height,GLsizei,depth,GLint,border,GLsizei,imageSize,const GLvoid*,data);
OPENGL_FORWARD(void,glCompressedTexSubImage1D,GLenum,target,GLint,level,GLint,xoffset,GLsizei,width,GLenum,format,GLsizei,imageSize,const GLvoid*,data);
OPENGL_FORWARD(void,glCompressedTexSubImage2D,GLenum,target,GLint,level,GLint,xoffset,GLint,yoffset,GLsizei,width,GLsizei,height,GLenum,format,GLsizei,imageSize,const GLvoid*,data);
OPENGL_FORWARD(void,glCompressedTexSubImage3D,GLenum,target,GLint,level,GLint,xoffset,GLint,yoffset,GLint,zoffset,GLsizei,width,GLsizei,height,GLsizei,depth,GLenum,format,GLsizei,imageSize,const GLvoid*,data);
OPENGL_FORWARD(void,glGetCompressedTexImage,GLenum,target,GLint,lod,GLvoid*,img);
OPENGL_FORWARD(void,glMultiTexCoord1d,GLenum,target,GLdouble,s);
OPENGL_FORWARD(void,glMultiTexCoord1dv,GLenum,target,const GLdouble*,v);
OPENGL_FORWARD(void,glMultiTexCoord1f,GLenum,target,GLfloat,s);
OPENGL_FORWARD(void,glMultiTexCoord1fv,GLenum,target,const GLfloat*,v);
OPENGL_FORWARD(void,glMultiTexCoord1i,GLenum,target,GLint,s);
OPENGL_FORWARD(void,glMultiTexCoord1iv,GLenum,target,const GLint*,v);
OPENGL_FORWARD(void,glMultiTexCoord1s,GLenum,target,GLshort,s);
OPENGL_FORWARD(void,glMultiTexCoord1sv,GLenum,target,const GLshort*,v);
OPENGL_FORWARD(void,glMultiTexCoord2d,GLenum,target,GLdouble,s,GLdouble,t);
OPENGL_FORWARD(void,glMultiTexCoord2dv,GLenum,target,const GLdouble*,v);
OPENGL_FORWARD(void,glMultiTexCoord2f,GLenum,target,GLfloat,s,GLfloat,t);
OPENGL_FORWARD(void,glMultiTexCoord2fv,GLenum,target,const GLfloat*,v);
OPENGL_FORWARD(void,glMultiTexCoord2i,GLenum,target,GLint,s,GLint,t);
OPENGL_FORWARD(void,glMultiTexCoord2iv,GLenum,target,const GLint*,v);
OPENGL_FORWARD(void,glMultiTexCoord2s,GLenum,target,GLshort,s,GLshort,t);
OPENGL_FORWARD(void,glMultiTexCoord2sv,GLenum,target,const GLshort*,v);
OPENGL_FORWARD(void,glMultiTexCoord3d,GLenum,target,GLdouble,s,GLdouble,t,GLdouble,r);
OPENGL_FORWARD(void,glMultiTexCoord3dv,GLenum,target,const GLdouble*,v);
OPENGL_FORWARD(void,glMultiTexCoord3f,GLenum,target,GLfloat,s,GLfloat,t,GLfloat,r);
OPENGL_FORWARD(void,glMultiTexCoord3fv,GLenum,target,const GLfloat*,v);
OPENGL_FORWARD(void,glMultiTexCoord3i,GLenum,target,GLint,s,GLint,t,GLint,r);
OPENGL_FORWARD(void,glMultiTexCoord3iv,GLenum,target,const GLint*,v);
OPENGL_FORWARD(void,glMultiTexCoord3s,GLenum,target,GLshort,s,GLshort,t,GLshort,r);
OPENGL_FORWARD(void,glMultiTexCoord3sv,GLenum,target,const GLshort*,v);
OPENGL_FORWARD(void,glMultiTexCoord4d,GLenum,target,GLdouble,s,GLdouble,t,GLdouble,r,GLdouble,q);
OPENGL_FORWARD(void,glMultiTexCoord4dv,GLenum,target,const GLdouble*,v);
OPENGL_FORWARD(void,glMultiTexCoord4f,GLenum,target,GLfloat,s,GLfloat,t,GLfloat,r,GLfloat,q);
OPENGL_FORWARD(void,glMultiTexCoord4fv,GLenum,target,const GLfloat*,v);
OPENGL_FORWARD(void,glMultiTexCoord4i,GLenum,target,GLint,s,GLint,t,GLint,r,GLint,q);
OPENGL_FORWARD(void,glMultiTexCoord4iv,GLenum,target,const GLint*,v);
OPENGL_FORWARD(void,glMultiTexCoord4s,GLenum,target,GLshort,s,GLshort,t,GLshort,r,GLshort,q);
OPENGL_FORWARD(void,glMultiTexCoord4sv,GLenum,target,const GLshort*,v);
OPENGL_FORWARD(void,glLoadTransposeMatrixd,const GLdouble*,m);
OPENGL_FORWARD(void,glLoadTransposeMatrixf,const GLfloat*,m);
OPENGL_FORWARD(void,glMultTransposeMatrixd,const GLdouble*,m);
OPENGL_FORWARD(void,glMultTransposeMatrixf,const GLfloat*,m);
OPENGL_FORWARD(void,glSampleCoverage,GLclampf,value,GLboolean,invert);
OPENGL_FORWARD(void,glActiveTextureARB,GLenum,texture);
OPENGL_FORWARD(void,glClientActiveTextureARB,GLenum,texture);
OPENGL_FORWARD(void,glMultiTexCoord1dARB,GLenum,target,GLdouble,s);
OPENGL_FORWARD(void,glMultiTexCoord1dvARB,GLenum,target,const GLdouble*,v);
OPENGL_FORWARD(void,glMultiTexCoord1fARB,GLenum,target,GLfloat,s);
OPENGL_FORWARD(void,glMultiTexCoord1fvARB,GLenum,target,const GLfloat*,v);
OPENGL_FORWARD(void,glMultiTexCoord1iARB,GLenum,target,GLint,s);
OPENGL_FORWARD(void,glMultiTexCoord1ivARB,GLenum,target,const GLint*,v);
OPENGL_FORWARD(void,glMultiTexCoord1sARB,GLenum,target,GLshort,s);
OPENGL_FORWARD(void,glMultiTexCoord1svARB,GLenum,target,const GLshort*,v);
OPENGL_FORWARD(void,glMultiTexCoord2dARB,GLenum,target,GLdouble,s,GLdouble,t);
OPENGL_FORWARD(void,glMultiTexCoord2dvARB,GLenum,target,const GLdouble*,v);
OPENGL_FORWARD(void,glMultiTexCoord2fARB,GLenum,target,GLfloat,s,GLfloat,t);
OPENGL_FORWARD(void,glMultiTexCoord2fvARB,GLenum,target,const GLfloat*,v);
OPENGL_FORWARD(void,glMultiTexCoord2iARB,GLenum,target,GLint,s,GLint,t);
OPENGL_FORWARD(void,glMultiTexCoord2ivARB,GLenum,target,const GLint*,v);
OPENGL_FORWARD(void,glMultiTexCoord2sARB,GLenum,target,GLshort,s,GLshort,t);
OPENGL_FORWARD(void,glMultiTexCoord2svARB,GLenum,target,const GLshort*,v);
OPENGL_FORWARD(void,glMultiTexCoord3dARB,GLenum,target,GLdouble,s,GLdouble,t,GLdouble,r);
OPENGL_FORWARD(void,glMultiTexCoord3dvARB,GLenum,target,const GLdouble*,v);
OPENGL_FORWARD(void,glMultiTexCoord3fARB,GLenum,target,GLfloat,s,GLfloat,t,GLfloat,r);
OPENGL_FORWARD(void,glMultiTexCoord3fvARB,GLenum,target,const GLfloat*,v);
OPENGL_FORWARD(void,glMultiTexCoord3iARB,GLenum,target,GLint,s,GLint,t,GLint,r);
OPENGL_FORWARD(void,glMultiTexCoord3ivARB,GLenum,target,const GLint*,v);
OPENGL_FORWARD(void,glMultiTexCoord3sARB,GLenum,target,GLshort,s,GLshort,t,GLshort,r);
OPENGL_FORWARD(void,glMultiTexCoord3svARB,GLenum,target,const GLshort*,v);
OPENGL_FORWARD(void,glMultiTexCoord4dARB,GLenum,target,GLdouble,s,GLdouble,t,GLdouble,r,GLdouble,q);
OPENGL_FORWARD(void,glMultiTexCoord4dvARB,GLenum,target,const GLdouble*,v);
OPENGL_FORWARD(void,glMultiTexCoord4fARB,GLenum,target,GLfloat,s,GLfloat,t,GLfloat,r,GLfloat,q);
OPENGL_FORWARD(void,glMultiTexCoord4fvARB,GLenum,target,const GLfloat*,v);
OPENGL_FORWARD(void,glMultiTexCoord4iARB,GLenum,target,GLint,s,GLint,t,GLint,r,GLint,q);
OPENGL_FORWARD(void,glMultiTexCoord4ivARB,GLenum,target,const GLint*,v);
OPENGL_FORWARD(void,glMultiTexCoord4sARB,GLenum,target,GLshort,s,GLshort,t,GLshort,r,GLshort,q);
OPENGL_FORWARD(void,glMultiTexCoord4svARB,GLenum,target,const GLshort*,v);
OPENGL_FORWARD(void,glBlendEquationSeparateATI,GLenum,modeRGB,GLenum,modeA);
OPENGL_FORWARD(void,glEGLImageTargetTexture2DOES,GLenum,target,GLeglImageOES,image);
OPENGL_FORWARD(void,glEGLImageTargetRenderbufferStorageOES,GLenum,target,GLeglImageOES,image);


/// Register all OpenGLAPI calls, defined above
void OpenglRedirectorBase::registerOpenGLSymbols(const std::vector<std::string>& symbols, SymbolRedirection& redirector)
{
    /*
    for(const auto& symbol: symbols)
    {
        if(helper::definedAPIFunctions.count(symbol) > 0)
        {
            redirector.addRedirection(symbol, helper::definedAPIFunctions[symbol]);
        }
    }*/
    
    for(auto& pair: helper::definedAPIFunctions)
    {
        redirector.addRedirection(pair.first, pair.second);
    }
}


