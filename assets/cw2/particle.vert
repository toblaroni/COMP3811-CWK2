// Particle vertex shader
// https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles

#version 410

layout( location = 0 ) in vec3 iPosition;
layout( location = 1 ) in vec2 iTexCoord;

out vec2 v2fTexCoord;
out vec4 v2fParticleColor;

uniform mat4 uProjCameraWorld;
uniform vec3 uOffset;
uniform vec4 uColor;

void main() {
    float scale = 10.0;     // Make this a uniform and change per pixel

    v2fTexCoord = iTexCoord;
    v2fParticleColor = uColor;
    gl_Position = uProjCameraWorld * vec4(iPosition*scale + uOffset, 1.0);
}