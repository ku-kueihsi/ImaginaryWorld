#define IW_GL_ES 1

#if IW_GL_ES 
//OpenGL ES 2.0
//#extension GL_EXT_separate_shader_objects : enable
//#extension GL_ARB_separate_shader_objects : enable
//precision mediump float;
//precision lowp float;
#ifdef GL_EXT_separate_shader_objects
#define UseLayout(x) layout(location = x)
#else
#define UseLayout(x)
#endif

#define IN attribute
#define OUT varying

#else //PC GL
#version 330
#define IN in
#define OUT out
#define UseLayout(x) layout(location = x)
#define lowp
#define mediump
#define highp
#endif

attribute vec3 a_Position;
attribute vec3 a_Normal;
attribute lowp vec2 a_TextureCoordinates;
attribute vec3 a_boneIds;
attribute vec3 a_boneWeights;

const int kMaxBoneWeights = 3;
const int kMaxBones = 50;

uniform mat4 modelviewProjection;
uniform mat4 world_mat;
uniform mat4 gBones[kMaxBones];

OUT vec2 v_TextureCoordinates;
OUT vec3 v_Normal;
//OUT vec4 debugColor;

void main()
{
//    mat4 BoneTransform = gBones[a_boneIds[0]] * a_boneWeights[0];
//    BoneTransform     += gBones[a_boneIds[1]] * a_boneWeights[1];
//    BoneTransform     += gBones[a_boneIds[2]] * a_boneWeights[2];

    mat4 BoneTransform = gBones[int(a_boneIds[0])] * a_boneWeights[0];
    BoneTransform     += gBones[int(a_boneIds[1])] * a_boneWeights[1];
    BoneTransform     += gBones[int(a_boneIds[2])] * a_boneWeights[2];

    v_TextureCoordinates = a_TextureCoordinates;

    v_Normal = (world_mat * BoneTransform * vec4(a_Normal, 0.0)).xyz;
    gl_Position = modelviewProjection * BoneTransform * vec4(a_Position, 1.0);
//    gl_Position = modelviewProjection * vec4(a_Position, 1.0);

//    debugColor[a_boneWeights[a_boneIds[0]], a_boneWeights[a_boneIds[1]], 1, 1];
//    debugColor = vec4(a_boneIds[0], 0, 0, 1);
//    v_Normal = (world_mat * gBones[a_boneIds[0]] * vec4(a_Normal, 0.0)).xyz;
//    gl_Position = modelviewProjection * gBones[a_boneIds[0]] * vec4(a_Position, 1.0);

//    v_Normal = (world_mat * vec4(a_Normal, 0.0)).xyz;
//    gl_Position = modelviewProjection * vec4(a_Position, 1.0);
    //gl_Position = a_Position;
}


