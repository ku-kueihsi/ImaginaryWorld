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
#include "platform_gl.h" //OpenGL

#include <assimp/Importer.hpp>      // C++ importer interface
#include <assimp/scene.h>       // Output data structure
#include <assimp/postprocess.h> // Post processing flags
#include <eigen3/Eigen/Dense>
#include "SDL2/SDL.h"
//#include "SDL2/SDL_opengles2.h"
#include "gl_tools.h"
#include "data_utils.h"

using namespace std;
using namespace Eigen;
using namespace glTools;

//#define WINWIDTH 1280
//#define WINHEIGHT 800

int winWidth = 1280;
int winHeight = 800;

static void sdldie(const char *msg)
{
	LOG_PRINT("SDL_LOG", "%s: %s\n", msg, SDL_GetError());
	SDL_Quit();
	exit(1);
}

static void checkSDLError(int line = -1)
{
#ifndef NDEBUG
	const char *error = SDL_GetError();
	if (*error != '\0') {
		LOG_PRINT("SDL_LOG", "SDL Error: %s\n", error);
		if (line != -1)
			LOG_PRINT("SDL_LOG", " + line: %i\n", line);
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
//static FILE * vertshader_file = NULL, *fragshader_file = NULL;

static GLint u_matrix = -1;
static GLint attr_pos = 0, attr_color = 1;

static GLint screen_width = 0;
static GLint screen_height = 0;

static GLuint VBOs[4];
//end of GL global


static void draw(void) {
//	GLfloat mat[16], rot[16], scale[16];
//
//	/* Set modelview/projection matrix */
//	make_z_rot_matrix(view_rotx, rot);
//	make_scale_matrix(0.5, 0.5, 0.5, scale);
//	mul_matrix(mat, rot, scale);
//	glUniformMatrix4fv(u_matrix, 1, GL_FALSE, mat);

	Matrix<GLfloat, 3, 1, ColMajor > direction(0.0f, 0.0f, -1.0f), camera_pos(0.0f, 0.0f, 5.0f), up(0.0f, 1.0f, 0.0f);

	//Matrix<GLfloat, 4, 4, ColMajor> pers_view_mat, rmat, tmat, perspective_mat, camera_mat;

//	scale_matrix(0.5f, smat.data());
//	rotation_matrix(0.0f, 0.0f, 1.0f, view_rotx, rmat.data());
	Matrix4fc smat = scale_matrix(0.5f);
//	rotation_matrix(0.0f, 0.0f, 1.0f, view_rotx, rmat);
	Matrix4fc rmat = rotation_matrix(1.0f, 1.0f, 1.0f, view_rotx);
	Matrix4fc tmat = translation_matrix(0.0f, 0.0f, 1.0f);
	Matrix4fc perspective_mat = perspective_matrix((GLfloat)3.14f * (GLfloat)0.25f, (GLfloat)screen_width / (GLfloat)screen_height, (GLfloat)0.01f, (GLfloat)1000.0f);
	Matrix4fc camera_mat = camera_matrix(direction, camera_pos, up);
	Matrix4fc pers_view_mat = perspective_mat * camera_mat * tmat * smat * rmat;
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
	GLfloat orthoScale = 0.9f;
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

static void create_shaders(void) {
	GLint stat;

//	SDL_RWops *vfile = NULL;
//	SDL_RWops *ffile = NULL;
//	vfile = SDL_RWFromFile("simplevert.glsl", "r");
//	ffile = SDL_RWFromFile("simplefrag.glsl", "r");
//	if (!vfile || !ffile){
//		exit(1);
//	}

	GLuint program = LoadShader("shader/simplevert.glsl", "shader/simplefrag.glsl");

//	SDL_RWclose(ffile);
//	SDL_RWclose(vfile);

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
	UNUSED(argc);
	UNUSED(argv);
	// init sdl
	SDL_Init(SDL_INIT_VIDEO);

	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
//	auto window = SDL_CreateWindow("SDL 2.0 Android Tutorial", 0, 0, WINWIDTH, WINHEIGHT, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	auto window = SDL_CreateWindow("SDL 2.0 Android Tutorial", 0, 0, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN_DESKTOP);

	//auto window = SDL_CreateWindow("SDL 2.0 Android Tutorial", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 512, 512, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
	if (window == 0)
	{
		SDL_Quit();
		return false;
	}
	SDL_GetWindowSize(window, &winWidth, &winHeight);
	SDL_SetHint(SDL_HINT_RENDER_DRIVER, "opengles2");
	auto glcontext = SDL_GL_CreateContext(window);

	//    if (!glcontext){
	//        printf("%s\n", glGetString(GL_VERSION));
	//        printf("OpenGL version too low.\n");
	//    }
//	checkSDLError(__LINE__);

	// raw gl code
	glGetString(GL_VERSION);

	LOG_PRINT("SDL_LOG", "%s\n", glGetString(GL_VERSION));

	on_surface_created();
	on_surface_changed(winWidth, winHeight);
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
			case SDL_FINGERDOWN:
			{
				on_touch();
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
