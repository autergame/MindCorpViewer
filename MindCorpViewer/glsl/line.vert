#version 330

layout (location = 0) in vec4 Positions;

uniform mat4 MVP;

void main()
{
	gl_Position = MVP * Positions;
}