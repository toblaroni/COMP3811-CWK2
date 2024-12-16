#version 410


in vec4 v2fColor;


uniform vec4 uButtonActiveColor;
uniform bool uButtonOutline;


layout( location = 0 ) out vec4 oColor;


void main()
{

    oColor = v2fColor * uButtonActiveColor;
    oColor = clamp(oColor, 0.0, 1.0);

}