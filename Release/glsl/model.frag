#version 330
//author https://github.com/autergame

in vec2 UV;

uniform sampler2D Diffuse;

void main()
{
	vec4 basecolor = texture(Diffuse, UV);
	if (basecolor.a < 0.9)
		discard;
	gl_FragColor = basecolor;	
}