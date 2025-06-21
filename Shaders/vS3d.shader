#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 inpNorm;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 FragPos;
out vec3 Normal;

void main()
{
    // Output position in clip space
    gl_Position = projection * view * model * vec4(aPos, 1.0);

    // Position in world space for lighting
    FragPos = vec3(model * vec4(aPos, 1.0));

    // Transform normal to world space using normal matrix
    Normal = mat3(transpose(inverse(model))) * inpNorm;
}
