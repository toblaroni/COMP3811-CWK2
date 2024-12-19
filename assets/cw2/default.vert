#version 410

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iNormal;
layout( location = 2 ) in vec2 iTexCoord;

layout( location = 3 ) in vec3 iAmbient;
layout( location = 4 ) in vec3 iDiffuse;
layout( location = 5 ) in vec3 iSpecular;
layout( location = 6 ) in float iShininess;
layout( location = 7 ) in vec3 iEmissive;
layout( location = 8 ) in float iIllum;

uniform mat4 uProjCameraWorld;
uniform mat3 uNormalMatrix;
uniform mat4 uModel2World;

out vec2 v2fTexCoord;

out vec3 v2fNormal;

out vec3 v2fAmbient;
out vec3 v2fDiffuse;
out vec3 v2fSpecular;
out float v2fShininess;
out vec3 v2fEmissive;
out float v2fIllum;

out vec3 v2fWorldPos;    // Pass position in 'view' space

void main()
{
    v2fTexCoord = iTexCoord;

    // Pass material attributes to the fragment shader
    v2fAmbient = iAmbient;
    v2fDiffuse = iDiffuse;
    v2fSpecular = iSpecular;
    v2fShininess = iShininess;
    v2fEmissive = iEmissive;
    v2fIllum = iIllum;

    v2fNormal = normalize(uNormalMatrix * iNormal);

    // Vertex position in world space
    vec4 worldPosition = uModel2World * vec4(iPosition, 1.0);
    v2fWorldPos = worldPosition.xyz;

    gl_Position = uProjCameraWorld * vec4( iPosition, 1.0 );
}
