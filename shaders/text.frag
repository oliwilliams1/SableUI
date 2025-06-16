#version 330 core

out vec4 FragColor;

in vec2 UV;

uniform sampler2D uAtlas;

void main()
{
	FragColor = vec4(texture(uAtlas, UV).r);
}