#version 330
//author https://github.com/autergame

in vec2 UV;

uniform sampler2D Diffuse;

void main()
{
    gl_FragColor = vec4(texture(Diffuse, UV).rgb, 1);
}