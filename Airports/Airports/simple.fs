#version 330 compatibility


/*
uniform samplerCube Texture0;
varying vec3 TexCoord0;
void main()
{
    vec4 texel = textureCube(Texture0, TexCoord0);
    gl_FragColor = texel;
}
*/


varying float v_Timer;
flat in vec3 v_Normal;
in vec3 v_Vertex;

const vec3 lightPos = vec3(0.0,0.5,0.0);
vec3 red = vec3(1.0,0.0,0.0);

void main()
{
    vec3 N = normalize(v_Normal);
    vec3 E = normalize(-v_Vertex);
    vec3 L = normalize(lightPos - v_Vertex);
    vec3 R = normalize(-reflect(L,N));

    float diffuse = max(dot(N,L),0.0);
    float specular = pow(max(dot(R,E),0.0),64.0);
    if(gl_FragCoord.x < 0.0)
        gl_FragColor = gl_Color * v_Timer * diffuse + specular*v_Timer;
    else
        //gl_FragColor = vec4(red * diffuse + specular,1.0);
        gl_FragColor = gl_Color*diffuse+specular;
    //gl_FragColor = vec4(1.0,0.0,0.0,1.0);
}