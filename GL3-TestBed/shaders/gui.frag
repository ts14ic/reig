#version 330 core

in vec2 fragTexPos;
in vec4 fragColor;

uniform sampler2D fragTexture;
uniform uint fragTexId;

out vec4 color;

void main() {
    if(fragTexId != 0u) {
        color = texture(fragTexture, vec2(fragTexPos.x, fragTexPos.y));
    }
    else {
        color = fragColor / 255.0;
    }
}
