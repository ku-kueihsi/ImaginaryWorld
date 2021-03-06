#define IW_GL_ES 1

#if IW_GL_ES
 
//OpenGL ES 2.0
//#version 100
//#extension GL_EXT_separate_shader_objects : enable
//#extension GL_ARB_separate_shader_objects : enable
//precision mediump float;
precision lowp float;
#define IN varying

#else //PC GL 330+

#version 330
#define IN in
#define lowp
#define mediump
#define highp

#endif

uniform lowp sampler2D u_TextureUnit;
IN lowp vec2 v_TextureCoordinates;
IN vec3 v_Normal;
//IN lowp vec4 debugColor;

void main()
{
    //u_TextureUnit = 0;
    float DiffuseFactor = dot(normalize(v_Normal), normalize(vec3(1.0, -1.0, 1.0)));
    vec4 DiffuseColor;                                                              
       
    DiffuseFactor = clamp(DiffuseFactor, 0.1, 1.0); 
    DiffuseColor = vec4(1.0, 1.0, 1.0, 1.0) * DiffuseFactor;                                                                              
 //   if (DiffuseFactor > 0) {                                                        
 //       DiffuseColor = vec4(1.0, 1.0, 1.0, 1.0) * DiffuseFactor;                                               
 //   }                                                                               
 //   else {                                                                          
 //      DiffuseColor = vec4(0.0, 0.0, 0.0, 0.0);                                            
 //   } 
    //gl_FragColor = texture2D(u_TextureUnit, v_TextureCoordinates) * (DiffuseColor + vec4(0.1, 0.1, 0.1, 0.0));
    gl_FragColor = texture2D(u_TextureUnit, v_TextureCoordinates) * DiffuseColor;
//    gl_FragColor = debugColor;
//    gl_FragColor = vec4(1,1,1,1);

    //gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
}

//varying vec4 v_color;
//void main() {
//	gl_FragColor = v_color;
//	//gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);  
//}
