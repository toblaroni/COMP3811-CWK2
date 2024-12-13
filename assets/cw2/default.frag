#version 410

#define NUM_LIGHTS 3

in vec2 v2fTexCoord;
in vec3 v2fNormal;

in vec3 v2fAmbient;
in vec3 v2fDiffuse;
in vec3 v2fSpecular;
in float v2fShininess;
in vec3 v2fEmissive;
in float v2fIllum;

in vec3 v2fViewPos;

uniform mat4 uViewMatrix;
uniform bool uUseTexture;

// Directional light
uniform vec3 uDirectLightDir;
uniform vec3 uDirectLightAmbient;
uniform vec3 uDirectLightDiffuse;

// Stuff point lights
// Multiple lights - https://opentk.net/learn/chapter2/6-multiple-lights.html
uniform vec3 uLightPos[NUM_LIGHTS];
uniform vec3 uLightDiffuse[NUM_LIGHTS];
uniform vec3 uLightSpecular[NUM_LIGHTS];
uniform vec3 uSceneAmbient[NUM_LIGHTS];

layout( location = 0 ) out vec3 oColor;

// This doesn't work on Mac
// layout( binding = 0 ) uniform sampler2D uTexture;
uniform sampler2D uTexture; // No layout(binding) qualifier

vec3 calcBlinnPhongLighting( vec3 normal, vec3 lightDir, vec3 viewDir, 
                             vec3 aLightPos, vec3 aSceneAmbient, 
                             vec3 aLightDiffuse, vec3 aLightSpecular ) {
    
    
    // Calculate Blinn-Phong lighting
    float lightDist = length(aLightPos - v2fViewPos);
    float falloff = 1.0 / (lightDist * lightDist);

    // return vec3(falloff);

    // Blinn-Phong Lighting 
    // K_a * I_a
    vec3 ambience = v2fAmbient * aSceneAmbient;

    // Diffuse contribution
    float nDotL = max( 0.0, dot( normal, lightDir ) );
    vec3 diffuse = (nDotL * aLightDiffuse * v2fDiffuse) * falloff;   // Apply falloff

    // Intensify specular contribution
    // Make highlights pop and shiny things shine more
    float spec_modifier = 5.0;

    vec3 H = normalize(lightDir + viewDir);    // Half vector
    float hDotN = max(0.0, dot(H, normal));
    vec3 specular = (pow(hDotN, v2fShininess) * aLightSpecular * v2fSpecular) * spec_modifier * falloff;    // Apply falloff

    // Combine the lighting
    vec3 lighting = ambience + diffuse + specular + v2fEmissive;

    // return specular;     // Debugging

    lighting *= v2fIllum;
    return lighting;
}


void main()
{
    vec3 normal = normalize(v2fNormal);

    // Original directional lighting
    float nDotL = max( 0.0, dot( normal, uDirectLightDir ) );
    // Just use the diffuse component of the material since this is what v2fcolor was originally
    vec3 lighting = (uDirectLightAmbient + nDotL * uDirectLightDiffuse) * v2fDiffuse;

    // === Point lights ===
    // Calculate view direction
    // This is direction from fragment to camera
    vec3 viewDir = normalize(-v2fViewPos);

    for (int i = 0; i < NUM_LIGHTS; ++i) {
        vec3 lightPosViewSpace = (uViewMatrix * vec4(uLightPos[i], 1.0)).xyz;   // Translate to view pos

        vec3 lightDir = normalize(lightPosViewSpace - v2fViewPos);
        lighting += calcBlinnPhongLighting(
            normal, lightDir, viewDir,
            lightPosViewSpace, uSceneAmbient[i],
            uLightDiffuse[i], uLightSpecular[i]
        );
    }

    // Add the texture stuff
    oColor = uUseTexture ? lighting * texture( uTexture, v2fTexCoord ).rgb : lighting;
    oColor = clamp( oColor, 0.0, 1.0 );

}