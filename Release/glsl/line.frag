#version 330

uniform int Color;

void main()
{       
    if (Color == 0)
        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);
    else if (Color == 1)
        gl_FragColor = vec4(0.0, 0.0, 1.0, 1.0);
}