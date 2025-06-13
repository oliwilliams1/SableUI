#include <fstream>

#include "SableUI/shader.h"
#include "SableUI/console.h"

static bool ReadFile(const char* pFileName, std::string& outFile)
{
	std::ifstream f(pFileName);

	bool ret = false;

	if (f.is_open())
	{
		std::string line;
		while (std::getline(f, line))
		{
			outFile.append(line);
			outFile.append("\n");
		}

		f.close();
		ret = true;
	}
	else
	{
		SableUI_Warn("Unable to open file: %s", pFileName);
	}

	return ret;
}

void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0)
	{
		SableUI_Runtime_Error("Error creating shader type %d", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;

	GLint Lengths[1];
	Lengths[0] = (GLint)strlen(pShaderText);

	glShaderSource(ShaderObj, 1, p, Lengths);
	glCompileShader(ShaderObj);

	GLint success;
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);

	if (!success)
	{
		GLchar InfoLog[4096];
		glGetShaderInfoLog(ShaderObj, 4096, NULL, InfoLog);
		SableUI_Runtime_Error("Error compiling shader type %d: '%s'", ShaderType, InfoLog);
	}

	glAttachShader(ShaderProgram, ShaderObj);
}

void Shader::LoadBasicShaders(const char* p_vsSource, const char* p_fsSource)
{
	std::string vsSource = p_vsSource;
	std::string fsSource = p_fsSource;

    shaderProgram = glCreateProgram();

	std::string vs, fs;

    if (shaderProgram == 0)
    {
        SableUI_Runtime_Error("Error creating shader program!");
    }

    if (vsSource.size() > 0)
    {
        if (!ReadFile(vsSource.c_str(), vs)) SableUI_Runtime_Error("Failed to read vertex shader file: %s", vsSource.c_str());
        AddShader(shaderProgram, vs.c_str(), GL_VERTEX_SHADER);
    }

    if (fsSource.size() > 0)
    {
        if (!ReadFile(fsSource.c_str(), fs)) SableUI_Runtime_Error("Failed to read fragment shader file: %s", fsSource.c_str());
        AddShader(shaderProgram, fs.c_str(), GL_FRAGMENT_SHADER);
    }

    if (vs.size() == 0 && fs.size() == 0) return;

    GLint success = 0;
    GLchar errorLog[1024] = { 0 };

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (success == 0)
    {
        glGetProgramInfoLog(shaderProgram, sizeof(errorLog), nullptr, errorLog);
        SableUI_Runtime_Error("Error linking shader program: %s shader name: %s", errorLog);
    }

    glUseProgram(shaderProgram);
}

Shader::~Shader()
{
    glDeleteProgram(shaderProgram);
}

void Shader::Use() const
{
	glUseProgram(shaderProgram);
}