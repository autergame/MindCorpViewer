#version 330

in vec2 UV;

out vec4 FragColor;

uniform sampler2D Diffuse;

void main()
{
	FragColor = texture(Diffuse, UV);
}