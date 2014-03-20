#include "gl_tools.h"
#include "data_utils.h"
#include "platform_gl.h"
namespace glTools{
using namespace std;
using namespace Eigen;
#define EPS 0.00001

GLuint LoadShaderMemory(string vertexString, string fragmentString)
{
    //
    //Embedded shader for test
    ////========================
    //  static const char *fragShaderText = "varying vec4 v_color;\n"
    //      "void main() {\n"
    //      "   gl_FragColor = v_color;\n"
    //      "}\n";
    //  static const char *vertShaderText = "uniform mat4 modelviewProjection;\n"
    //      "attribute vec4 pos;\n"
    //      "attribute vec4 color;\n"
    //      "varying vec4 v_color;\n"
    //      "void main() {\n"
    //      "   gl_Position = modelviewProjection * pos;\n"
    //      "   v_color = color;\n"
    //      "}\n";

    GLuint vertexShader, fragmentShader;
    GLint stat;

    LOG_PRINT("SDL_LOG", "%d,%d\n", vertexString.size(), fragmentString.size());

    const GLchar *kVertexShader = vertexString.c_str();
    const GLchar *kFragmentShader = fragmentString.c_str();
    //  cout << VertexShader << endl << FragmentShader << endl;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const GLchar **) &kVertexShader, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &stat);
    if (!stat) {
        LOG_PRINT("SDL_LOG", "Error: vertex shader did not compile!\n");
        LOG_PRINT("SDL_LOG", "%s\n", vertexString.c_str());
        exit(1);
    }

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const GLchar **) &kFragmentShader, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &stat);
    if (!stat) {
    	LOG_PRINT("SDL_LOG", "Error: fragment shader did not compile!\n");
    	LOG_PRINT("SDL_LOG", "%s\n", kFragmentShader);
        exit(1);
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);

    return program;
}

GLuint LoadShader(string vertexFileName, string fragmentFileName) {
	string vertexString = fileToString(vertexFileName);
	string fragmentString = fileToString(fragmentFileName);

	//LOG_PRINT("SDL_LOG", "%s\n%s\n", vertexFileName.c_str(), vertexString.c_str());
	LOG_PRINT("SDL_LOG", "%d,%d\n", vertexString.size(), fragmentString.size());
    return LoadShaderMemory(vertexString, fragmentString);
}

#define GLMAT44 Eigen::Matrix<Tfloat, 4, 4, Eigen::ColMajor >
#define GLVEC3 Eigen::Matrix<Tfloat, 3, 1, Eigen::ColMajor >

template<class Tfloat>
GLMAT44 rotation_matrix(Tfloat u, Tfloat v, Tfloat w, Tfloat theta)
{
    //generate the rotation matrix, rotate theta around vector (u, v, w) at point (a, b, c)

    //create template matrix and direction vector
	GLMAT44 mat = GLMAT44::Identity();
	GLVEC3 vec(u, v, w);

    //normalize the direction vector
    vec.normalize();
    u = vec(0);
    v = vec(1);
    w = vec(2);

    //temp variables
    Tfloat costheta = cos(theta), sintheta = sin(theta);
    Tfloat complement_costheta = 1 - costheta;
    Tfloat u2 = u * u;
    Tfloat v2 = v * v;
    Tfloat w2 = w * w;
    Tfloat cos_part, sin_part;

    //3 x 3 rotation
    mat(0, 0) = u2 + (v2 + w2) * costheta;
    mat(1, 1) = v2 + (u2 + w2) * costheta;
    mat(2, 2) = w2 + (u2 + v2) * costheta;
    cos_part = u * v * (complement_costheta);
    sin_part = w * sintheta;
    mat(0, 1) = cos_part - sin_part;
    mat(1, 0) = cos_part + sin_part;
    cos_part = u * w * (complement_costheta);
    sin_part = v * sintheta;
    mat(0, 2) = cos_part + sin_part;
    mat(2, 0) = cos_part - sin_part;
    cos_part = v * w * (complement_costheta);
    sin_part = u * sintheta;
    mat(1, 2) = cos_part - sin_part;
    mat(1, 2) = cos_part + sin_part;
    return mat;
}

template<class Tfloat>
GLMAT44 scale_matrix(Tfloat xs, Tfloat ys, Tfloat zs)
{
    //generate the scale matrix

    //create template matrix
	GLMAT44 mat = GLMAT44::Identity();
    mat(0, 0) = xs;
    mat(1, 1) = ys;
    mat(2, 2) = zs;
    return mat;
}

template<class Tfloat>
GLMAT44 scale_matrix(Tfloat s)
{
    //generate the scale matrix
    return scale_matrix(s, s, s);
}

template<class Tfloat>
GLMAT44 translation_matrix(Tfloat x, Tfloat y, Tfloat z)
{
    //generate the translation matrix
    //create template matrix
	GLMAT44 mat = GLMAT44::Identity();
    mat(0, 3) = x;
    mat(1, 3) = y;
    mat(2, 3) = z;
    return mat;
}

template<class Tfloat>
//void perspective_matrix(Tfloat angleOfView = 45.0, Tfloat aspectRatio = 0.75, Tfloat near = 0.001, Tfloat far = 1000.0, GLMAT44 &mat) {
GLMAT44 perspective_matrix(Tfloat angleOfView, Tfloat aspectRatio, Tfloat near, Tfloat far)
{
    //generate the perspective matrix
    //radians angleOfView
    //aspectRatio = width / height

    // Some calculus before the formula.
    Tfloat size = near * tan(angleOfView / 2.0);
    Tfloat left = -size, right = size, bottom = -size / aspectRatio, top = size / aspectRatio;

    GLMAT44 mat;
    // First Column
    mat(0, 0) = 2 * near / (right - left);
    mat(1, 0) = 0.0;
    mat(2, 0) = 0.0;
    mat(3, 0) = 0.0;

    // Second Column
    mat(0, 1) = 0.0;
    mat(1, 1) = 2 * near / (top - bottom);
    mat(2, 1) = 0.0;
    mat(3, 1) = 0.0;

    // Third Column
    mat(0, 2) = (right + left) / (right - left);
    mat(1, 2) = (top + bottom) / (top - bottom);
    mat(2, 2) = -(far + near) / (far - near);
    mat(3, 2) = -1;

    // Fourth Column
    mat(0, 3) = 0.0;
    mat(1, 3) = 0.0;
    mat(2, 3) = -(2 * far * near) / (far - near);
    mat(3, 3) = 0.0;

    return mat;
}

template<class Tfloat>
GLMAT44 camera_matrix(const GLVEC3 direction,
        const GLVEC3 camera,
        const GLVEC3 camera_up
        )
{
    //generate the camera matrix
    //create template vector
    GLVEC3 right, up, back;
    GLMAT44 rmat;
    back = -direction;
    back.normalize();
    right = camera_up.cross(back);
    right.normalize();
    up = back.cross(right);

    if (EPS > back.norm() || EPS > up.norm() || EPS > right.norm()) {
        return GLMAT44::Identity();;
    }

    //rmat.block(0, 0, 1, 3) = right;
    //rmat.block(1, 0, 1, 3) = up;
    //rmat.block(2, 0, 1, 3) = back;
    for (int i = 0; i < 3; ++i) {
        rmat(0, i) = right(i);
        rmat(1, i) = up(i);
        rmat(2, i) = back(i);
        rmat(3, i) = 0.0f;
        rmat(i, 3) = 0.0f;
    }
    rmat(3, 3) = 1;

    GLMAT44 tmat = translation_matrix(-camera(0), -camera(1), -camera(2));

    return rmat * tmat;
}

template Matrix4fc rotation_matrix(GLfloat u, GLfloat v, GLfloat w, GLfloat theta);

template Matrix4fc scale_matrix(GLfloat s);

template Matrix4fc scale_matrix(GLfloat xs, GLfloat ys, GLfloat zs);

template Matrix4fc translation_matrix(GLfloat x, GLfloat y, GLfloat z);

template Matrix4fc perspective_matrix(GLfloat angleOfView, GLfloat aspectRatio, GLfloat near, GLfloat far);

template Matrix4fc camera_matrix(const Vector3f direction,
        const Vector3f camera,
        const Vector3f camera_up);
}
