#version 430

// For now just Implement the passthrough shaders from exg2

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec3 iColor;
layout( location = 2 ) in vec3 iNormal;

layout( location = 0 ) uniform mat3 uNormalMatrix;

out vec3 v2fColor;
out vec3 v2fNormal;

void main()
{
    v2fColor = iColor;

    v2fNormal = normalize(uNormalMatrix * iNormal);

    gl_Position = vec4( iPosition, 1.0 );
}
