// Particle vertex shader
// https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles
//
#version 410

layout( location = 0 ) in vec3 iPosition;  // Input particle position (e.g., from squareVertices)
layout( location = 1 ) in vec2 iTexCoord;  // Texture coordinates

out vec2 v2fTexCoord;
out vec4 v2fParticleColor;

uniform mat4 uProjCameraWorld;  // Projection and camera matrix
uniform vec4 uColor;            // Particle color

uniform vec3 uParticleCenterWorldSpace;  // The center of the particle (world-space position)
uniform vec3 uCameraRightWorldSpace;    // Right vector (from camera view matrix)
uniform vec3 uCameraUpWorldSpace;       // Up vector (from camera view matrix)
uniform vec2 uBillboardSize;            // Size of the billboard (scale factor)

void main() {
    // Calculate the world-space position for each vertex of the particle
    // https://www.opengl-tutorial.org/intermediate-tutorials/billboards-particles/billboards/#solution-2--the-3d-way
    vec3 vertexPositionWorldSpace = 
        uParticleCenterWorldSpace
        + uCameraRightWorldSpace * iPosition.x * uBillboardSize.x
        + uCameraUpWorldSpace * iPosition.y * uBillboardSize.y;

    // Pass the texture coordinates and color to the fragment shader
    v2fTexCoord = iTexCoord;
    v2fParticleColor = uColor;

    // Compute the final clip-space position
    gl_Position = uProjCameraWorld * vec4(vertexPositionWorldSpace, 1.0);
}
