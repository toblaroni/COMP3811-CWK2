#version 410

in vec3 v2fNormal;

in vec3 v2fAmbient;
in vec3 v2fDiffuse;
in vec3 v2fSpecular;
in float v2fShininess;
in vec3 v2fEmissive;
in float v2fIllum;
in vec3 v2fLightPosViewSpace;

in vec3 v2fViewPos;

uniform vec3 uLightDiffuse;
uniform vec3 uLightSpecular;
uniform vec3 uSceneAmbient;

layout( location = 0 ) out vec3 oColor;

void main()
{
    vec3 normal = normalize(v2fNormal);

    // Calculate view direction
    // This is direction from fragment to camera
    vec3 viewDir = normalize(-v2fViewPos);

    vec3 lightDir = normalize(v2fLightPosViewSpace - v2fViewPos);
    float lightDist = length(v2fLightPosViewSpace - v2fViewPos);
    float falloff = 1.0 / (lightDist * lightDist);

    // Blinn-Phong Lighting 
    // K_a * I_a
    vec3 ambience = v2fAmbient * uSceneAmbient;

    // Diffuse contribution
    float nDotL = max( 0.0, dot( normal, lightDir ) );
    vec3 diffuse = (nDotL * uLightDiffuse * v2fDiffuse) * falloff;   // Apply falloff

    // Specular contribution
    // Specular is not that much
    float spec_modifier = 100.0;

    vec3 H = normalize(lightDir + viewDir);    // Half vector
    float hDotN = max(0.0, dot(H, normal));
    vec3 specular = (pow(hDotN, v2fShininess) * uLightSpecular * v2fSpecular) * spec_modifier * falloff;    // Apply falloff

    oColor = ambience + diffuse + specular*spec_modifier + v2fEmissive;
    oColor *= v2fIllum;
    oColor = clamp(oColor, 0.0, 1.0);

}