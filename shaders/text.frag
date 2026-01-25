#version 330 core

out vec4 FragColour;

in vec3 UV;
in vec4 colour;

uniform sampler2DArray uAtlas;

void main()
{
	float a = texture(uAtlas, UV).r;

	a = a * a * (3.0 - 2.0 * a);

	FragColour = vec4(colour.rgb, a * colour.a);
}