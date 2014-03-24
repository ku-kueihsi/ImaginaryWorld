/* 
 * File:   gl_tools.h
 * Author: jesse
 *
 * Created on November 9, 2013, 11:10 AM
 */

#ifndef __GL_TOOLS_H__
#define	__GL_TOOLS_H__

#include "platform_gl.h"
#include <eigen3/Eigen/Dense>
#include <string>

//#ifdef __GNUC__
//#  define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
//#else
//#  define UNUSED(x) UNUSED_ ## x
//#endif
//
//#ifdef __GNUC__
//#  define UNUSED_FUNCTION(x) __attribute__((__unused__)) UNUSED_ ## x
//#else
//#  define UNUSED_FUNCTION(x) UNUSED_ ## x
//#endif

#define UNUSED(x) (void)(x)

namespace glTools{

void die(const char *msg);

std::string correctPath(std::string);

GLuint LoadShaderMemory(std::string vertexString, std::string fragmentString);
GLuint LoadShader(std::string vertexFileName, std::string fragmentFileName);

#define GLMAT44 Eigen::Matrix<Tfloat, 4, 4, Eigen::ColMajor >
#define GLVEC3 Eigen::Matrix<Tfloat, 3, 1, Eigen::ColMajor >
typedef Eigen::Matrix<GLfloat, 4, 4, Eigen::ColMajor > Matrix4fc;
//typedef Eigen::Matrix<GLdouble, 4, 4, Eigen::ColMajor > Matrix4dc;


template<class Tfloat>
GLMAT44 rotation_matrix(Tfloat u, Tfloat v, Tfloat w, Tfloat theta);

template<class Tfloat>
GLMAT44 scale_matrix(Tfloat s);

template<class Tfloat>
GLMAT44 scale_matrix(Tfloat xs, Tfloat ys, Tfloat zs);

template<class Tfloat>
GLMAT44 translation_matrix(Tfloat x, Tfloat y, Tfloat z);

template<class Tfloat>
GLMAT44 perspective_matrix(Tfloat angleOfView, Tfloat aspectRatio, Tfloat near, Tfloat far);

template<class Tfloat>
GLMAT44 camera_matrix(const GLVEC3 direction, const GLVEC3 camera, const GLVEC3 camera_up);

//extern template <> void scale_matrix<GLfloat>(Eigen::Matrix<GLfloat, 4, 4, Eigen::ColMajor > &mat, GLfloat xs, GLfloat ys, GLfloat zs);
//extern template <> void scale_matrix<GLfloat>(Eigen::Matrix<GLfloat, 4, 4, Eigen::ColMajor > &mat, GLfloat s);

#undef GLVEC3
#undef GLMAT44
}

#endif	/* GL_TOOLS_H */

