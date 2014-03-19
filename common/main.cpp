#include <time.h>
#include <iostream>
#include <string>
#include <algorithm>
#include <fstream>
#include <vector>
#include <cmath>
#include <string.h>
#include <cstdlib>
#include <cstdio>
#include <assert.h>
#include <chrono>
#include <thread>

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <eigen3/Eigen/Dense>
#include "SDL2/SDL.h"
#include "SDL2/SDL_opengles2.h"
#include <android/log.h>

using namespace std;
using namespace Eigen;

#define WINWIDTH 1280
#define WINHEIGHT 800

static void sdldie(const char *msg)
{
	printf("%s: %s\n", msg, SDL_GetError());
	SDL_Quit();
	exit(1);
}

static void checkSDLError(int line = -1)
{
#ifndef NDEBUG
	const char *error = SDL_GetError();
	if (*error != '\0') {
		printf("SDL Error: %s\n", error);
		if (line != -1)
			printf(" + line: %i\n", line);
		SDL_ClearError();
	}
#endif
}

#define EPS 0.00001
//int testf() {
//	Matrix<int, 2, 2, ColMajor> m;
//	m(0, 0) = 3;
//	m(1, 0) = 2.5;
//	m(0, 1) = -1;
//	m(1, 1) = m(1, 0) + m(0, 1);
////	std::cout << m << std::endl;
//}

typedef Matrix<GLfloat, 4, 4, ColMajor> GLmatrix4f;
typedef Matrix<GLfloat, 4, 1, ColMajor> GLvector4f;

size_t filelen(FILE * pfile) {
	if (!pfile) {
		return 0;
	}
	fseek(pfile, NULL, SEEK_END);
	size_t filesize = ftell(pfile);
	rewind(pfile);
	return filesize;
}

//GL global
static GLfloat view_rotx = 0.0; // view_roty = 0.0;
static FILE * vertshader_file = NULL, *fragshader_file = NULL;

static GLint u_matrix = -1;
static GLint attr_pos = 0, attr_color = 1;

static GLint screen_width = 0;
static GLint screen_height = 0;

static GLuint VBOs[4];
//end of GL global

void set_vertshader_file(FILE * pfile) {
	vertshader_file = pfile;
//	__android_log_print(ANDROID_LOG_VERBOSE, "test", "%d", filelen(vertshader_file));
//	__android_log_print(ANDROID_LOG_VERBOSE, "test", "%d\n", vertshader_file);
}

void set_fragshader_file(FILE * pfile) {
	fragshader_file = pfile;
//	__android_log_print(ANDROID_LOG_VERBOSE, "test", "%d", filelen(fragshader_file));
//	__android_log_print(ANDROID_LOG_VERBOSE, "test", "%d\n", fragshader_file);
}



template<class Tfloat>
void rotation_matrix(Tfloat u, Tfloat v, Tfloat w, Tfloat theta, Matrix<Tfloat, 4, 4, ColMajor > &mat) {
	//generate the rotation matrix, rotate theta around vector (u, v, w) at point (a, b, c)

	//create template matrix and direction vector
	mat = Matrix<Tfloat, 4, 4, ColMajor>::Identity();
	Matrix < Tfloat, 3, 1, ColMajor > vec(u, v, w);

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
}

template<class Tfloat>
void scale_matrix(Tfloat xs, Tfloat ys, Tfloat zs, Matrix<Tfloat, 4, 4, ColMajor > &mat) {
	//generate the scale matrix

	//create template matrix
	mat = Matrix<Tfloat, 4, 4, ColMajor>::Identity();
	mat(0, 0) = xs;
	mat(1, 1) = ys;
	mat(2, 2) = zs;
}

template<class Tfloat>
void scale_matrix(Tfloat s, Matrix<Tfloat, 4, 4, ColMajor > &mat) {
	//generate the scale matrix
	scale_matrix(s, s, s, mat);
}

template<class Tfloat>
void translation_matrix(Tfloat x, Tfloat y, Tfloat z, Matrix<Tfloat, 4, 4, ColMajor > &mat) {
	//generate the translation matrix
	//create template matrix
	mat = Matrix<Tfloat, 4, 4, ColMajor>::Identity();
	mat(0, 3) = x;
	mat(1, 3) = y;
	mat(2, 3) = z;
}

template<class Tfloat>
//void perspective_matrix(Tfloat angleOfView = 45.0, Tfloat aspectRatio = 0.75, Tfloat near = 0.001, Tfloat far = 1000.0, Matrix<Tfloat, 4, 4, ColMajor > &mat) {
void perspective_matrix(Tfloat angleOfView, Tfloat aspectRatio, Tfloat near, Tfloat far, Matrix<Tfloat, 4, 4, ColMajor > &mat) {
	//generate the perspective matrix
	//radians angleOfView
	//aspectRatio = width / height

    // Some calculus before the formula.
    Tfloat size = near * tan(angleOfView / 2.0);
    Tfloat left = -size, right = size, bottom = -size / aspectRatio, top = size / aspectRatio;

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
}

template<class Tfloat>
void camera_matrix(Matrix<Tfloat, 4, 4, ColMajor > &mat,
		const Matrix<Tfloat, 3, 1, ColMajor > &direction,
		const Matrix<Tfloat, 3, 1, ColMajor > &camera,
		const Matrix<Tfloat, 3, 1, ColMajor > &camera_up
		) {
	//generate the camera matrix
	//create template vector
	Matrix<Tfloat, 3, 1, ColMajor > right, up, back;
	Matrix<Tfloat, 4, 4, ColMajor > tmat, rmat;
	back = -direction;
	back.normalize();
	right = camera_up.cross(back);
	right.normalize();
	up = back.cross(right);

	if (EPS > back.norm() || EPS > up.norm() || EPS > right.norm()){
		mat = Matrix<Tfloat, 4, 4, ColMajor >::Identity();
		return;
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

	translation_matrix(-camera(0), -camera(1), -camera(2), tmat);

	mat = rmat * tmat;
}

static void draw(void) {
//	GLfloat mat[16], rot[16], scale[16];
//
//	/* Set modelview/projection matrix */
//	make_z_rot_matrix(view_rotx, rot);
//	make_scale_matrix(0.5, 0.5, 0.5, scale);
//	mul_matrix(mat, rot, scale);
//	glUniformMatrix4fv(u_matrix, 1, GL_FALSE, mat);

	Matrix<GLfloat, 3, 1, ColMajor > direction(0.0f, 0.0f, -1.0f), camera_pos(0.0f, 0.0f, 5.0f), up(0.0f, 1.0f, 0.0f);

	Matrix<GLfloat, 4, 4, ColMajor> pers_view_mat, rmat, smat, tmat, perspective_mat, camera_mat;

//	scale_matrix(0.5f, smat.data());
//	rotation_matrix(0.0f, 0.0f, 1.0f, view_rotx, rmat.data());
	scale_matrix(0.5f, smat);
//	rotation_matrix(0.0f, 0.0f, 1.0f, view_rotx, rmat);
	rotation_matrix(1.0f, 1.0f, 1.0f, view_rotx, rmat);
	translation_matrix(0.0f, 0.0f, 1.0f, tmat);
	perspective_matrix((GLfloat)3.14f * (GLfloat)0.25f, (GLfloat)screen_width / (GLfloat)screen_height, (GLfloat)0.01f, (GLfloat)1000.0f, perspective_mat);
	camera_matrix(camera_mat, direction, camera_pos, up);
	pers_view_mat = perspective_mat * camera_mat * tmat * smat * rmat;
	glUniformMatrix4fv(u_matrix, 1, GL_FALSE, pers_view_mat.data());

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	{
		glEnableVertexAttribArray(attr_pos);
		glEnableVertexAttribArray(attr_color);

		glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
		glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
		glVertexAttribPointer(attr_color, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, 3);

		glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(attr_pos);
		glDisableVertexAttribArray(attr_color);
	}

	Matrix<GLfloat, 4, 4, ColMajor > orthoMat = Matrix<GLfloat, 4, 4, ColMajor >::Identity();
	GLfloat orthoScale = 0.2;
	orthoMat(0, 0) = orthoScale;
	orthoMat(1, 1) = orthoScale;
	orthoMat(2, 2) = orthoScale;
	glUniformMatrix4fv(u_matrix, 1, GL_FALSE, orthoMat.data());
	{
	    glDisable(GL_DEPTH_TEST);
	    glDepthMask(GL_FALSE);

		glEnableVertexAttribArray(attr_pos);
		glEnableVertexAttribArray(attr_color);

		glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
		glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, 0);
		glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
		glVertexAttribPointer(attr_color, 3, GL_FLOAT, GL_FALSE, 0, 0);

		glDrawArrays(GL_TRIANGLES, 0, 3);

	    glBindBuffer(GL_ARRAY_BUFFER, 0);

		glDisableVertexAttribArray(attr_pos);
		glDisableVertexAttribArray(attr_color);

	    glEnable(GL_DEPTH_TEST);
	    glDepthMask(GL_TRUE);
	}

}

GLuint LoadShader(FILE * vfile, FILE * ffile) {
//	__android_log_print(ANDROID_LOG_VERBOSE, "test", "loading shaders");
//	if (!vfile || !ffile) {
//		exit(1);
//	}
//	size_t len;
//	len = filelen(vfile);
//	char * vertShaderText = new char[len + 1];
//	fread(vertShaderText, len, sizeof(char), vfile);
//	vertShaderText[len] = '\0';
//
//	len = filelen(ffile);
//	char * fragShaderText = new char[len + 1];
//	fread(fragShaderText, len, sizeof(char), ffile);
//	fragShaderText[len] = '\0';

//	__android_log_print(ANDROID_LOG_VERBOSE, "test", "%s\n", vertShaderText);
//	__android_log_print(ANDROID_LOG_VERBOSE, "test", "%s\n", fragShaderText);


	static const char *fragShaderText = "precision mediump float;\n"
			"varying vec4 v_color;\n"
			"void main() {\n"
			"   gl_FragColor = v_color;\n"
			"}\n";
	static const char *vertShaderText = "precision mediump float;\n"
			"uniform mat4 modelviewProjection;\n"
			"attribute vec4 pos;\n"
			"attribute vec4 color;\n"
			"varying vec4 v_color;\n"
			"void main() {\n"
			"   gl_Position = modelviewProjection * pos;\n"
			"   v_color = color;\n"
			"}\n";

	GLuint fragShader, vertShader, program;
	GLint stat;

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, (const char **) &fragShaderText, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		//      printf("Error: fragment shader did not compile!\n");
		exit(1);
	}

	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, (const char **) &vertShaderText, NULL);
	glCompileShader(vertShader);
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		//      printf("Error: vertex shader did not compile!\n");
		exit(1);
	}

	program = glCreateProgram();
	glAttachShader(program, fragShader);
	glAttachShader(program, vertShader);

//	delete vertShaderText;
//	delete fragShaderText;
	return program;
}

GLuint LoadShader(SDL_RWops * vfile, SDL_RWops * ffile) {
//	__android_log_print(ANDROID_LOG_VERBOSE, "test", "loading shaders");
	if (!vfile || !ffile) {
		exit(1);
	}
	size_t len;
	len = SDL_RWseek(vfile, 0, SEEK_END);
	SDL_RWseek(vfile, 0, SEEK_SET);
	char * vertShaderText = new char[len + 1];
	SDL_RWread(vfile, vertShaderText, sizeof(char), len);
	vertShaderText[len] = '\0';

	len = SDL_RWseek(ffile, 0, SEEK_END);
	SDL_RWseek(ffile, 0, SEEK_SET);
	char * fragShaderText = new char[len + 1];
	SDL_RWread(ffile, fragShaderText, sizeof(char), len);
	fragShaderText[len] = '\0';

	__android_log_print(ANDROID_LOG_VERBOSE, "test", "%s\n", vertShaderText);
	__android_log_print(ANDROID_LOG_VERBOSE, "test", "%s\n", fragShaderText);


//	static const char *fragShaderText = "precision mediump float;\n"
//			"varying vec4 v_color;\n"
//			"void main() {\n"
//			"   gl_FragColor = v_color;\n"
//			"}\n";
//	static const char *vertShaderText = "precision mediump float;\n"
//			"uniform mat4 modelviewProjection;\n"
//			"attribute vec4 pos;\n"
//			"attribute vec4 color;\n"
//			"varying vec4 v_color;\n"
//			"void main() {\n"
//			"   gl_Position = modelviewProjection * pos;\n"
//			"   v_color = color;\n"
//			"}\n";

	GLuint fragShader, vertShader, program;
	GLint stat;

	fragShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragShader, 1, (const char **) &fragShaderText, NULL);
	glCompileShader(fragShader);
	glGetShaderiv(fragShader, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		//      printf("Error: fragment shader did not compile!\n");
		exit(1);
	}

	vertShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertShader, 1, (const char **) &vertShaderText, NULL);
	glCompileShader(vertShader);
	glGetShaderiv(vertShader, GL_COMPILE_STATUS, &stat);
	if (!stat) {
		//      printf("Error: vertex shader did not compile!\n");
		exit(1);
	}

	program = glCreateProgram();
	glAttachShader(program, fragShader);
	glAttachShader(program, vertShader);

	delete vertShaderText;
	delete fragShaderText;
	return program;
}

static void create_shaders(void) {
	GLint stat;

	SDL_RWops *vfile = NULL;
	SDL_RWops *ffile = NULL;
	vfile = SDL_RWFromFile("simplevert.glsl", "r");
	ffile = SDL_RWFromFile("simplefrag.glsl", "r");
	if (!vfile || !ffile){
		exit(1);
	}

	GLuint program = LoadShader(vfile, ffile);

	SDL_RWclose(ffile);
	SDL_RWclose(vfile);

	glLinkProgram(program);

	glGetProgramiv(program, GL_LINK_STATUS, &stat);
	if (!stat) {
		char log[1000];
		GLsizei len;
		glGetProgramInfoLog(program, 1000, &len, log);
		printf("Error: linking:\n%s\n", log);
		exit(1);
	}

	glUseProgram(program);

	if (1) {
		/* test setting attrib locations */
		glBindAttribLocation(program, attr_pos, "pos");
		glBindAttribLocation(program, attr_color, "color");
		glLinkProgram(program); /* needed to put attribs into effect */
	} else {
		/* test automatic attrib locations */
		attr_pos = glGetAttribLocation(program, "pos");
		attr_color = glGetAttribLocation(program, "color");
	}

	glGenBuffers(4, VBOs);
//    glBindVertexArray(VBOs[0]);

	static const GLfloat verts[3][2] = { { -1, -1 }, { 1, -1 }, { 0, 1 } };
	static const GLfloat colors[3][3] =
			{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
	static const GLfloat hudVertices[3][2] = {{0.0f, 0.0f}, {1.0f, 0.0f}, {1.0f, 1.0f}};
	static const GLfloat vertexColors[3][3] =
				{ { 0.5f, 0, 0 }, { 0, 0.5f, 0 }, { 0, 0, 0.5f } };

    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * 6, verts, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * 9, colors, GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * 6, hudVertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, VBOs[3]);
	glBufferData(GL_ARRAY_BUFFER, sizeof (GLfloat) * 9, vertexColors, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

//	glVertexAttribPointer(attr_pos, 2, GL_FLOAT, GL_FALSE, 0, verts);
//	glVertexAttribPointer(attr_color, 3, GL_FLOAT, GL_FALSE, 0, colors);

	u_matrix = glGetUniformLocation(program, "modelviewProjection");
//	printf("Uniform modelviewProjection at %d\n", u_matrix);
//	printf("Attrib pos at %d\n", attr_pos);
//	printf("Attrib color at %d\n", attr_color);
}

void on_surface_created() {
	glClearColor(0.4, 0.4, 0.4, 0.0);
	// OpenGL init
	glEnable (GL_DEPTH_TEST);
	glEnable (GL_CULL_FACE);
	//glEnable(GL_LIGHTING);
	create_shaders();
}

void on_surface_changed(int width, int height) {
	// No-op
	screen_width = width;
	screen_height = height;
	if (screen_height == 0)
		screen_height = 1;
	// Set the viewport to be the entire window
	glViewport(0, 0, screen_width, screen_height);

}

void on_touch() {
	//view_rotx += 5.0;
	view_rotx += 3.14 * 5 / 180;
	printf("touched\n");
	//draw();
}

void on_draw_frame() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	draw();
//    view_rotx += 5.0;
	// Reset transformations

}

#ifdef __cplusplus
extern "C" {
#endif
int main(int argc, char * argv[])
{
	// init sdl
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
	auto window = SDL_CreateWindow("SDL 2.0 Android Tutorial", 0, 0, WINWIDTH, WINHEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	//auto window = SDL_CreateWindow("SDL 2.0 Android Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

	if (window == 0)
	{
		SDL_Quit();
		return false;
	}
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
	auto glcontext = SDL_GL_CreateContext(window);

	//    if (!glcontext){
	//        printf("%s\n", glGetString(GL_VERSION));
	//        printf("OpenGL version too low.\n");
	//    }
	checkSDLError(__LINE__);

	// raw gl code
	glGetString(GL_VERSION);
	printf("%s\n", glGetString(GL_VERSION));

	on_surface_created();
	on_surface_changed(WINWIDTH, WINHEIGHT);
//

//	std::chrono::time_point<std::chrono::high_resolution_clock> lastTime = std::chrono::high_resolution_clock::now();
//	const GLfloat kTimePerTick = 1 / 60.0f;

	bool exit = false;
	while (!exit) {
		SDL_PumpEvents();

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
			{
				if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
					exit = true;
				}else if (event.key.keysym.scancode == SDL_SCANCODE_D) {
					on_touch();
				}else if (event.key.keysym.scancode == SDL_SCANCODE_A) {

				}
				break;
			}
			case SDL_QUIT:
			{
				exit = true;
				break;
			}
			default:
			{
				break;
			}
			}
		}
		on_draw_frame();
		SDL_GL_SwapWindow(window);

	}

	SDL_GL_DeleteContext(glcontext);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
#ifdef __cplusplus
}
#endif
