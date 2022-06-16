#version 330 core
layout (location = 0) in vec3 vertex;
layout (location = 1) in vec2 uv;
layout (location = 2) in vec4 col;


uniform mat4 model;
uniform mat4 projection;
uniform mat4 camera;

out vec2 TexCoords;
out vec4 color;

void main()
{
	gl_Position =  projection * camera * model * vec4(vertex,  1.0);
	TexCoords = uv;
	color = col;
};