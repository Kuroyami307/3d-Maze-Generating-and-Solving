#version 300 es
precision mediump float;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;
uniform vec3 ourColor; // base object color (RGB)

out vec4 FragColor;

void main()
{
    // === Lighting setup ===
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPosition - FragPos);
    vec3 viewDir = normalize(cameraPosition - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);

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
