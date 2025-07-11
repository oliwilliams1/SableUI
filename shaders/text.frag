#version 330 core

out vec4 FragColour;

in vec3 UV;

uniform sampler2DArray uAtlas;

void main()
{
	vec3 colour = texture(uAtlas, UV).rgb;

	/*  Font wieght approx
		1 = Condensed
		1.15 = Regular
		1.43 = Semibold
		2.2  = Bold          */
	colour = pow(colour, vec3(1.0 / 1.15));
	FragColour = vec4(colour, 1.0);
}