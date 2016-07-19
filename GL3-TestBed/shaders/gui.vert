#version 330 core

layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec2 vertTexPos;
layout (location = 2) in vec4 vertColor;

uniform mat4 vertexProjection;

out vec2 fragTexPos;
out vec4 fragColor;

void main() {
    gl_Position = vertexProjection * vec4(vertPos, 0.0, 1.0);
    fragColor = vertColor;
    fragTexPos = vertTexPos;
}
