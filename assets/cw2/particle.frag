// Particle fragment shader
// https://learnopengl.com/index.php?p=In-Practice/2D-Game/Particles

#version 410

in vec2 v2fTexCoord;
in vec4 v2fParticleColor;

uniform sampler2D uSprite;

out vec4 oColor;

void main() {
    vec4 texColor = texture(uSprite, v2fTexCoord);

    // This feels hacky, but looks infinitely better
    // This is inefficient
    // if (texColor.a < 0.65) discard;

    oColor = (texColor * v2fParticleColor);
    oColor = clamp(oColor, 0.0, 1.0);
}
