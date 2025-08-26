#version 450 core

uniform sampler2D uFontAtlas;

in vec2 vTexCoords;
in vec4 vColor;

out vec4 color;

void main() {
    float alpha = texture(uFontAtlas, vTexCoords).r;

    color = vec4(vColor.rgb, vColor.a * alpha);
}