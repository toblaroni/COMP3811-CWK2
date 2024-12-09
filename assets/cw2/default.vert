#version 430

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iColor;
layout( location = 2 ) in vec2 iTexCoord;
layout( location = 3 ) in vec3 iNormal;

// layout( location = 0 ) uniform mat4 uProjCameraWorld;
uniform mat4 uProjCameraWorld;
uniform mat3 uNormalMatrix;

out vec3 v2fColor;
out vec2 v2fTexCoord;
out vec3 v2fNormal;

void main()
{
    v2fColor = iColor;
    v2fTexCoord = iTexCoord;

    v2fNormal = normalize(uNormalMatrix * iNormal);

    gl_Position = uProjCameraWorld * vec4( iPosition, 1.0 );
}