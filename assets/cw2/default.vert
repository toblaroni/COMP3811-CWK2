#version 410

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iNormal;

layout( location = 2 ) in vec3 iAmbient;
layout( location = 3 ) in vec3 iDiffuse;
layout( location = 4 ) in vec3 iSpecular;
layout( location = 5 ) in vec3 iShininess;
layout( location = 6 ) in float iEmissive;
layout( location = 7 ) in float iIllum;

out vec3 v2fAmbient;
out vec3 v2fDiffuse;
out vec3 v2fSpecular;
out vec3 v2fShininess;
out float v2fEmissive;
out float v2fIllum;

// layout( location = 0 ) uniform mat4 uProjCameraWorld;
uniform mat4 uProjCameraWorld;
uniform mat3 uNormalMatrix;

out vec3 v2fNormal;

void main()
{

    // Pass material attributes to the fragment shader
    v2fAmbient = iAmbient;
    v2fDiffuse = iDiffuse;
    v2fSpecular = iSpecular;
    v2fShininess = iShininess;
    v2fEmissive = iEmissive;
    v2fIllum = iIllum;

    v2fNormal = normalize(uNormalMatrix * iNormal);

    gl_Position = uProjCameraWorld * vec4( iPosition, 1.0 );
}