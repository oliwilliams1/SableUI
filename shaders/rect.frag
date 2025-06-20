#version 330 core

out vec4 FragColor;

uniform vec3 uColour = vec3(0.0);
uniform bool uUseTexture = false;
uniform sampler2D uTexture;

in vec2 uv;

void main()
{
    if (uUseTexture)
    {
		FragColor = texture(uTexture, uv);
		return;
	}
	else
	{
		FragColor = vec4(uColour, 1.0);
		return;
	}
}