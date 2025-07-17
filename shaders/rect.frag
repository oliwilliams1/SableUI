#version 330 core

out vec4 FragColor;

uniform vec4 uColour = vec4(vec3(0.0), 1.0);
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
		FragColor = vec4(uColour);
		return;
	}
}