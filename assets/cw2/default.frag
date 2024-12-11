#version 430

in vec2 v2fTexCoord;
in vec3 v2fNormal;

in vec3 v2fAmbient;
in vec3 v2fDiffuse;
in vec3 v2fSpecular;
in float v2fShininess;
in vec3 v2fEmissive;
in float v2fIllum;

in vec3 v2fViewPos;

uniform vec3 uLightDir;

uniform vec3 uLightDiffuse;
uniform vec3 uLightSpecular;
uniform vec3 uSceneAmbient;
uniform bool uUseTexture;
uniform vec3 uLightPosViewSpace;

layout( location = 0 ) out vec3 oColor;

layout( binding = 0 ) uniform sampler2D uTexture;


vec3 calcBlinnPhongLighting( vec3 normal, vec3 lightDir, vec3 viewDir ) {
    // Calculate Blinn-Phong lighting
    float lightDist = length(uLightPosViewSpace - v2fViewPos);
    float falloff = 1.0 / (lightDist * lightDist);

    // Blinn-Phong Lighting 
    // K_a * I_a
    vec3 ambience = v2fAmbient * uSceneAmbient;

    // Diffuse contribution
    float nDotL = max( 0.0, dot( normal, lightDir ) );
    vec3 diffuse = (nDotL * uLightDiffuse * v2fDiffuse) * falloff;   // Apply falloff

    // Intensify specular contribution
    float spec_modifier = 7.0;

    vec3 H = normalize(lightDir + viewDir);    // Half vector
    float hDotN = max(0.0, dot(H, normal));
    vec3 specular = (pow(hDotN, v2fShininess) * uLightSpecular * v2fSpecular) * spec_modifier * falloff;    // Apply falloff

    // Combine the lighting
    vec3 lighting = ambience + diffuse + specular*spec_modifier + v2fEmissive;
    lighting *= v2fIllum;
    return lighting;

}


void main()
{
    vec3 normal = normalize(v2fNormal);

    // Calculate view direction
    // This is direction from fragment to camera
    vec3 viewDir = normalize(-v2fViewPos);
    vec3 lightDir = normalize(uLightPosViewSpace - v2fViewPos);

    // Original directional lighting
    float nDotL = max( 0.0, dot( normal, uLightDir ) );
    // Just use the diffuse component of the material since this is what v2fcolor was originally
    vec3 lighting = (uSceneAmbient + nDotL * uLightDiffuse) * v2fDiffuse;

    lighting += calcBlinnPhongLighting( normal, lightDir, viewDir );

    // Add the texture stuff
    oColor = uUseTexture ? lighting * texture( uTexture, v2fTexCoord ).rgb : lighting;
    oColor = clamp( oColor, 0.0, 1.0 );
}