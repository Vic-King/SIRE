
#ifndef _PGHP_OPENGL_H_
#define _PGHP_OPENGL_H_

#ifdef __APPLE__

#define GL_GLEXT_PROTOTYPES
#include <OpenGL/gl3.h>
#include <OpenGL/glu.h>
#else

#define GL_GLEXT_PROTOTYPES
#include <GL/gl.h>
#include <GL/glu.h>
#endif

/** A macro to track OpenGL errors
  */
#ifndef PGHP_NO_GLDEBUG
#define GL_TEST_ERR\
    do {\
    GLenum eCode;\
    if((eCode=glGetError())!=GL_NO_ERROR)\
    std::cerr << "OpenGL error : " <<  gluErrorString(eCode) << " in " <<  __FILE__ << " : " << __LINE__ << std::endl;\
    } while(0)
#else
#define GL_TEST_ERR
#endif


#endif // _PGHP_OPENGL_H_
