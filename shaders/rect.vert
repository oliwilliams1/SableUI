#version 330 core

layout(location = 0) in vec2 aUV;

uniform vec4 uRect; // x, y, w, h

out vec2 uv;

void main()
{
    gl_Position = vec4(uRect.xy + aUV * uRect.zw, 0.0, 1.0);
    uv = aUV;
}
