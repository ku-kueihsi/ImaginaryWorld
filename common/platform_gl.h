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
#include <GLES2/gl2ext.h>
#include <GLES2/gl2platform.h>
#include <android/log.h>

#ifndef NDEBUG
#define LOG_PRINT(title, fmt, args...) __android_log_print(ANDROID_LOG_DEBUG, title, fmt, ##args)
#else
#define LOG_PRINT(title, fmt, args...)
#endif //NDEBUG

#define ASSERT(cond) ((cond)?(void)0:__android_log_assert("##cond", "assert", ""))
#define ASSERTMSG(cond, fmt, args...) ((cond)?(void)0:__android_log_assert("##cond", "assert", fmt, ##args))
//#define ASSERT(e...) __android_log_assert(e, "TAG", #e)

#endif //__ANDROID__

//#include <GL/gl.h>
//#include <GL/glext.h>

#endif	/* __PLATFORM_GL_H__ */

