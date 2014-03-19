/* 
 * File:   platform_gl.h
 * Author: jesse
 *
 * Created on November 7, 2013, 2:08 PM
 */

#ifndef __PLATFORM_GL_H__
#define	__PLATFORM_GL_H__

#ifdef __ANDROID__

#ifndef GL_GLEXT_PROTOTYPES
#define GL_GLEXT_PROTOTYPES
#endif
#include <GLES2/gl2.h>
#include <android/log.h>

//#ifndef NDEBUG
#define LOG_PRINT(title, format, x) __android_log_print(ANDROID_LOG_VERBOSE, title, format, x)
//#else
//#define LOG_PRINT(title, format, x)
//#endif //NDEBUG

#endif //__ANDROID__

//#include <GL/gl.h>
//#include <GL/glext.h>

#endif	/* __PLATFORM_GL_H__ */

