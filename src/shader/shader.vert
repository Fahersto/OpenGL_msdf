#version 330 core
layout (location = 0) in vec3 vertex;
uniform mat4 model;
uniform mat4 projection;
uniform mat4 camera;
uniform vec2 renderingOffset;

out vec2 TexCoords;

void main()
{
	gl_Position =  projection * camera * model * (vec4(vertex,  1.0) + vec4(renderingOffset, 0, 0));
	TexCoords = vertex.xy;
};