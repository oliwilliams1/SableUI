#version 330 core

out vec4 FragColour;

in vec3 UV;
in vec4 colour;

uniform sampler2DArray uAtlas;

void main()
{
    float alpha = texture(uAtlas, UV).r;
    vec3 ink = colour.rgb * alpha;
    
    ink = pow(ink, vec3(1.0/1.8));

    FragColour = vec4(ink, alpha * colour.a);
}