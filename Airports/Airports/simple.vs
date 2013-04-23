#version 330 compatibility

/*
uniform mat4 ProjectionModelviewMatrix;
varying vec3 TexCoord0;
void main()
{
   gl_Position = ProjectionModelviewMatrix * gl_Vertex;
   TexCoord0 = gl_Normal;
}
*/


uniform float u_Timer;
uniform mat4 u_ModelViewMatrix;
uniform mat4 u_ProjectionMatrix;
varying float v_Timer;
flat out vec3 v_Normal;				// jako varying, novejsi verze
out vec3 v_Vertex;

const vec3 E = vec3(0.0,0.0,0.0);

vec4 g_UserColor = vec4(0.0,1.0,0.0,1.0); // uzivatelem zvolena barva

vec4 getColor(in float position, out float side_effect) // vstupni a vystupni promenna
{
	//return vec4(0.0,0.0, (position < 0.0) ? 1.0 : 0.0, 1.0);
	return vec4(0.0,0.0,0.0,1.0);
}

void getColor1(inout vec4 color) // vstupne-vystupni promenna
{
	//color.yxzw = (color.x < 1.0) ? g_UserColor : vec4(1.0);
	color.yxzw = g_UserColor;
}

void main()
{

#if 0
	
	gl_FrontColor = g_UserColor; // pouzije uzivatelovu barvu

	if(gl_Vertex.x < 0.0) // lze i ternarnim vyrazem gl_FrontColor = (gl_Vertex.x < 0.0) ? g_UserColor : vec4(...);
		gl_FrontColor = g_UserColor;
	else
		gl_FrontColor = vec4(1.0,0.0,0.0,1.0);

	float a = 0;
	gl_FrontColor = getColor(gl_Vertex.x,a); // vypocet barvy pomoci funkce

#else
	vec4 color = gl_Vertex;
	getColor1(color);

	gl_FrontColor = color;// * diffuse;
	
	v_Timer = u_Timer;
	v_Normal = normalize(mat3(gl_ModelViewMatrix) * gl_Normal);
	vec4 viewPos = gl_ModelViewMatrix * gl_Vertex;
	v_Vertex = viewPos.xyz;

#endif
	gl_Position = gl_ProjectionMatrix * gl_ModelViewMatrix * gl_Vertex; // pozice vertexu do modelovaci, pohledove, projekcni matice
}