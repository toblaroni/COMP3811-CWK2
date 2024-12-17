// Particle fragment shader
// https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles

#version 410

in vec2 v2fTexCoord;
in vec4 v2fParticleColor;

uniform sampler2D uSprite;

out vec4 oColor;

void main() {
    oColor = (texture( uSprite, v2fTexCoord ) * v2fParticleColor);
}