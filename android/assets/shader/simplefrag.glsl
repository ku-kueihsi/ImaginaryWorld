precision lowp float;
varying vec2 v_TextureCoordinates;
varying vec3 v_Normal;
varying vec4 debugColor;
void main() {
	//u_TextureUnit = 0;
    //float DiffuseFactor = dot(normalize(v_Normal), normalize(vec3(1.0f, -1.0f, 1.0f)));
    //vec4 DiffuseColor = debugColor * 3.0f; 
    //DiffuseColor = vec4(1.0f, 1.0f, 1.0f, 1.0f) * DiffuseFactor;
    
	gl_FragColor = debugColor;
	//gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);  
}