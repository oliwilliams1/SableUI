#version 330 core

out vec4 FragColor;

in vec3 UV;

uniform sampler2DArray uAtlas;

void main()
{
	FragColor = vec4(texture(uAtlas, UV).r);
}