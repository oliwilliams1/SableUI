#version 330 core

out vec4 FragColour;

in vec3 UV;
in vec4 colour;

uniform sampler2DArray uAtlas;

void main()
{
	vec3 fac = texture(uAtlas, UV).rgb;
	FragColour = colour * vec4(fac, 1.0);
}