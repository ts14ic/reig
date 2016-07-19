#version 330 core

layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec2 vertTexCoord;

uniform vec3 color;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec4 fragColor;
out vec2 fragTexCoord;

void main() {
    fragColor = vec4(color / 255.0, 1.0);
    fragTexCoord = vertTexCoord;
    gl_Position = projection * view * model * vec4(vertPos, 1.0f);
}
