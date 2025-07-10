#version 330 core

out vec4 FragColour;

in vec3 UV;

uniform sampler2DArray uAtlas;

void main()
{
	vec3 colour = texture(uAtlas, UV).rgb;
	
	FragColour = vec4(colour, 1.0);
}