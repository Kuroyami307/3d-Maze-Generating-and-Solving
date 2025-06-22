#version 300 es
precision mediump float;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform vec3 highlightColor; // base object color (RGB)
uniform vec3 baseColor;
uniform float startTime;
uniform float time;

out vec4 FragColor;

void main()
{
    // === Lighting setup ===
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragPos);
    vec3 viewDir = normalize(cameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

    float progress = 0.0;
    vec3 ourColor = baseColor;

    if(startTime > 0.0 && highlightColor != baseColor)
    {
        float localTime = time - startTime;
        progress = clamp(localTime / 5.0, 0.0, 1.0);
        ourColor = mix(highlightColor, baseColor, progress);
    }

    // Ambient
    float ambientStrength = 0.2;
    vec3 ambient = ambientStrength * vec3(ourColor);

    // Diffuse
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * vec3(ourColor);

    // Specular
    float specularStrength = 0.4;
    float shininess = 32.0;
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
    vec3 specular = specularStrength * spec * vec3(1.0); // white specular

    // Final color
    vec3 result = ambient + diffuse;
    FragColor = vec4(result, 1.0);
}
