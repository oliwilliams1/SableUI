#version 330 core

out vec4 FragColour;

in vec3 UV;
in vec4 colour;

uniform sampler2DArray uAtlas;

void main()
{
    vec3 coverage = texture(uAtlas, UV).rgb;

    float alpha = max(max(coverage.r, coverage.g), coverage.b);

    vec3 ink = colour.rgb * coverage;
    ink = pow(ink, vec3(1.0/2.2));

    FragColour = vec4(ink, alpha * colour.a);
}