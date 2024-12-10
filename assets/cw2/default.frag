#version 410

in vec3 v2fNormal;

in vec3 v2fAmbient;
in vec3 v2fDiffuse;
in vec3 v2fSpecular;
in float v2fShininess;
in vec3 v2fEmissive;
in float v2fIllum;

in vec3 v2fViewPos;

uniform vec3 uLightDiffuse;
uniform vec3 uLightSpecular;
uniform vec3 uSceneAmbient;
uniform vec3 uLightPos;

layout( location = 0 ) out vec3 oColor;

void main()
{
    vec3 normal = normalize(v2fNormal);

    // Calculate view direction
    vec3 viewDir = normalize(-v2fViewPos);

    vec3 lightDir = normalize(uLightPos - v2fViewPos);
    float lightDist = length(lightDir);
    float falloff = 1.0 / (lightDist * lightDist);

    // Blinn-Phong Lighting 
    // K_a * I_a
    vec3 ambience = v2fAmbient * uSceneAmbient;

    // Diffuse contribution
    float nDotL = max( 0.0, dot( normal, lightDir ) );
    vec3 diffuse = (nDotL * uLightDiffuse * v2fDiffuse) * falloff;   // Apply falloff

    // Specular contribution
    vec3 H = normalize(lightDir + viewDir);    // Half vector
    float hDotN = max(0.0, dot(H, normal));
    vec3 specular = (pow(hDotN, v2fShininess*100) * uLightSpecular * v2fSpecular) * falloff;    // Apply falloff


    oColor = ambience + diffuse + specular + v2fEmissive;

    oColor = clamp(oColor, 0.0, 1.0);
}

