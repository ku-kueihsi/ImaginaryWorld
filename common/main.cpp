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
#include "SDL2/SDL_opengles2.h"
#include "gl_tools.h"
#include "data_utils.h"
#include "image.h"
#include "mesh.h"

using namespace std;
using namespace Eigen;
using namespace glTools;


int winWidth = 1280;
int winHeight = 800;


#define TREEDEBUG 0
#if TREEDEBUG

//struct TranslationSample
//{
//public:
//    GLfloat time;
//    GLVector3f translation;
////    TranslationSample &operator= (const TranslationSample &translationSample){
////        this->time = translationSample.time;
////        this->translation = translationSample.translation;
////        return *this;
////    }
//};
//
//void NodeRecursive(const aiNode *pNode, int depth){
//    string nodeName(pNode->mName.data);

//    cout << string(depth, '-') << nodeName << endl;
//    cout << string(depth, '-') << pNode->mNumMeshes << endl;
////    cout << string(depth, '-') << "(" << endl;
//    for (int i = 0; i < pNode->mNumChildren; ++i){
//        const aiNode *child = pNode->mChildren[i];
//        NodeRecursive(child, depth + 1);
//    }
////    cout << string(depth, '-') << ")" << endl;
//}


//#define MAX_BONE_WEIGHTS 3
int main()
{
    //bone test main

//    const char pFileName[] = "../assets/model/dummy_rig.dae";
//    const char pFileName[] = "/tmp/test/scripts/Kid_DNF/Kid.dae";
    const char pFileName[] = "../assets/model/simple_debug.dae";
    Assimp::Importer importer;
    importer.SetPropertyInteger(AI_CONFIG_PP_LBW_MAX_WEIGHTS, 3);
    const aiScene* pScene = importer.ReadFile(pFileName, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    UnitTree utree;
    utree.Init(pScene);
    utree.UpdateAnimation(1.0f);
    utree.Print();

    return 0;
}

#else

//GL global
static GLfloat view_rotx = 0.0; // view_roty = 0.0;
//static FILE * vertshader_file = NULL, *fragshader_file = NULL;

static GLint u_matrix = -1;
static GLint u_world_mat = -1;
//static GLint attr_pos = 0, attr_color = 1;
static GLint screen_width = winWidth;
static GLint screen_height = winHeight;

//static GLuint texture;
//static GLuint buffer;
//static GLuint program;

string vstr, fstr;

//File2Str(vstr, "simplevert.glsl");
//File2Str(fstr, "simplefrag.glsl");

//static Texture texobj;
static ShaderProgram programobj;
//static Mesh meshobj;
static RenderUnit objobj;

//static GLint a_position_location;
//static GLint a_texture_coordinates_location;
//static GLint u_texture_unit_location;

/* A simple function that prints a message, the error code returned by SDL,
 * and quits the application */

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

static void create_shaders(void)
{
//    GLint stat;

    string vstr = fileToString("shader/daeVertex.glsl");
    string fstr = fileToString("shader/daeFragment.glsl");
//    string vstr = fileToString("/sdcard/Download/shader/simplevert2.glsl");
//    string fstr = fileToString("/sdcard/Download/shader/simplefrag2.glsl");

    programobj.Load(vstr, fstr);
}

void Init_GL()
{
    GLint h = winHeight;
    GLint w = winWidth;
    if (h == 0)
        h = 1;

    glViewport(0, 0, w, h);

    create_shaders();

    u_matrix = glGetUniformLocation(programobj.GetId(), "modelviewProjection");
    u_world_mat = glGetUniformLocation(programobj.GetId(), "world_mat");
//    texobj.Load("texture/cow-tex-fin.png");
//    texobj.Load("texture/dummy_white.png");

//    const char pFileName[] = "./model/cow.dae";
    const char pFileName[] = "model/dummy_rig.dae";
//    const char pFileName[] = "model/simple_debug.dae";

    string fileData = fileToString(pFileName);
//    LOG_PRINT("SDL_LOG", "%d\n", fileData.size());

    objobj.setResourcePath("texture/");
    objobj.InitFromMemory(fileData);
    objobj.SetShaderIndex(programobj.GetId());
    objobj.SetUpShader();

    glEnable(GL_DEPTH_TEST);
//    glFrontFace(GL_CW);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    //glEnable(GL_LIGHTING);
}

void Render_GL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
    programobj.Use();

    Matrix<GLfloat, 3, 1, ColMajor > direction(0.0f, 5.0f - (-5.0f), 0.0f - 5.0f), camera_pos(0.0f, -5.0f, 5.0f), up(0.0f, 0.0f, 1.0f);


    //  scale_matrix(0.5f, smat.data());
    //  rotation_matrix(0.0f, 0.0f, 1.0f, view_rotx, rmat.data());
    Matrix4fc smat = scale_matrix(0.3f);
//    scale_matrix(smat, 0.3f, 0.3f, 0.3f);
    Matrix4fc rmat = rotation_matrix(0.0f, 0.0f, 1.0f, view_rotx);
//    rotation_matrix(rmat, 1.0f, 1.0f, 1.0f, view_rotx);
    Matrix4fc tmat = translation_matrix(0.0f, 5.0f, 0.0f);
    Matrix4fc perspective_mat = perspective_matrix((GLfloat) 3.14f * (GLfloat) 0.25f, (GLfloat) screen_width / (GLfloat) screen_height, (GLfloat) 0.01f, (GLfloat) 1000.0f);
    Matrix4fc camera_mat = camera_matrix(direction, camera_pos, up);
    Matrix4fc world_mat = tmat * smat * rmat;
    Matrix4fc pers_view_mat = perspective_mat * camera_mat * world_mat;
    glUniformMatrix4fv(u_matrix, 1, GL_FALSE, pers_view_mat.data());
    glUniformMatrix4fv(u_world_mat, 1, GL_FALSE, world_mat.data());


//    texobj.Bind();
//    meshobj.Render();
    objobj.Render();
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

    if (!glcontext){
    	LOG_PRINT("SDL_LOG", "%s\n", glGetString(GL_VERSION));
    	LOG_PRINT("SDL_LOG", "OpenGL version too low.\n");
    	die("can not get glcontext");
    }
//    checkSDLError(__LINE__);

	// raw gl code
	glGetString(GL_VERSION);
	LOG_PRINT("SDL_LOG", "%s\n", glGetString(GL_VERSION));


    Init_GL();

    std::chrono::time_point<std::chrono::high_resolution_clock> lastTime = std::chrono::high_resolution_clock::now();
    const GLfloat kTimePerTick = 1 / 60.0f;
    bool exit = false;
    while (!exit) {
        SDL_PumpEvents();

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
            case SDL_KEYDOWN:
            {
                //                    {
                //                        //checks if a key is being remapped and prints what the remapping is
                //                        if (event.key.keysym.scancode != SDL_GetScancodeFromKey(event.key.keysym.sym)) {
                //                            printf("Physical %s key acting as %s key",
                //                                    SDL_GetScancodeName(event.key.keysym.scancode),
                //                                    SDL_GetKeyName(event.key.keysym.sym));
                //                            exit = true;
                //                        }
                //                    }
                //                    printf("Physical %s key acting as %s key",
                //                            SDL_GetScancodeName(event.key.keysym.scancode),
                //                            SDL_GetKeyName(event.key.keysym.sym));
                //                    //                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                if (event.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
                    exit = true;
                }else if (event.key.keysym.scancode == SDL_SCANCODE_D) {
                    view_rotx += 3.14 / 10;
                }else if (event.key.keysym.scancode == SDL_SCANCODE_A) {
                    view_rotx -= 3.14 / 10;
                }
                break;
            }
            case SDL_FINGERDOWN:
            {
            	view_rotx += 3.14 / 10;
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
        std::chrono::time_point<std::chrono::high_resolution_clock> currentTime = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> deltaTime = lastTime - currentTime;
        std::chrono::duration<double> tmp(kTimePerTick);
        deltaTime += tmp;
        if (deltaTime.count() > 0){
            std::this_thread::sleep_for (deltaTime);
        }
        Render_GL();
        //        glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
        //        glClear(GL_COLOR_BUFFER_BIT);
        SDL_GL_SwapWindow(window);
        lastTime = currentTime;
    }
//    glClearColor(0.0f, 0.0f, 0.8f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT);
//    SDL_GL_SwapWindow(window);
//
//    // done
//        SDL_Delay(2000);
    SDL_GL_DeleteContext(glcontext);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
#ifdef __cplusplus
}
#endif
#endif
