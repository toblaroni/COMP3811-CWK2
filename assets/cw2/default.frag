#version 430

in vec3 v2fColor;
in vec2 v2fTexCoord;
in vec3 v2fNormal;

uniform vec3 uLightDir;
uniform vec3 uLightDiffuse;
uniform vec3 uSceneAmbient;

layout( location = 0 ) out vec3 oColor;

layout( binding = 0 ) uniform sampler2D uTexture;

void main()
{
    vec3 normal = normalize(v2fNormal);

    float nDotL = max( 0.0, dot( normal, uLightDir ) );

    oColor = texture( uTexture, v2fTexCoord ).rgb;
    // oColor = (uSceneAmbient + nDotL * uLightDiffuse) * v2fColor;
    // oColor = v2fColor
}

