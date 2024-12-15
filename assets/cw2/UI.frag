#version 410


in vec4 v2fColor;


uniform vec4 uButtonActiveColor;

layout( location = 0 ) out vec4 oColor;


void main()
{

    oColor = v2fColor * uButtonActiveColor;

}