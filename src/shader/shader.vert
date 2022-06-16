#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;


uniform mat4 model;
uniform mat4 projection;
uniform mat4 camera;

out vec2 TexCoords;

void main()
{
	gl_Position =  projection * camera * model * vec4(vertex,  1.0);
	TexCoords = uv;
};