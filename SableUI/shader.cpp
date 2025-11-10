#include <fstream>
#include <cstring>

#include "SableUI/shader.h"
#include "SableUI/console.h"

static std::vector<GLuint> shaderPrograms;

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

static GLuint AddShader(GLuint shaderProgram, const char* pShaderText, GLenum ShaderType)
{
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0)
	{
		SableUI_Runtime_Error("Error creating shader type %d", ShaderType);
	}

	const GLchar* p[1];
	p[0] = pShaderText;

	GLint Lengths[1]{};
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

	glAttachShader(shaderProgram, ShaderObj);

	return ShaderObj;
}

void SableUI::Shader::LoadBasicShaders(const char* vs, const char* fs)
{
    if (!vs || !fs)
    {
        SableUI_Runtime_Error("Both vertex and fragment shaders must be provided!");
        return;
    }

    m_shaderProgram = glCreateProgram();
    if (m_shaderProgram == 0)
    {
        SableUI_Runtime_Error("Error creating shader program!");
        return;
    }

    GLuint vertShader = AddShader(m_shaderProgram, vs, GL_VERTEX_SHADER);
    GLuint fragShader = AddShader(m_shaderProgram, fs, GL_FRAGMENT_SHADER);

    glLinkProgram(m_shaderProgram);

    GLint success = 0;
    GLchar errorLog[1024] = { 0 };
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(m_shaderProgram, sizeof(errorLog), nullptr, errorLog);
        SableUI_Runtime_Error("Error linking shader program: %s", errorLog);
    }

    glValidateProgram(m_shaderProgram);

    glGetProgramiv(m_shaderProgram, GL_VALIDATE_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(m_shaderProgram, sizeof(errorLog), nullptr, errorLog);
        SableUI_Runtime_Error("Error validating shader program: %s", errorLog);
    }

    glDetachShader(m_shaderProgram, vertShader);
    glDetachShader(m_shaderProgram, fragShader);
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);

    glUseProgram(m_shaderProgram);
    shaderPrograms.push_back(m_shaderProgram);
}

void SableUI::Shader::Use() const
{
	glUseProgram(m_shaderProgram);
}

void SableUI::DestroyShaders()
{
	for (GLuint shaderProgram : shaderPrograms)
	{
		glDeleteProgram(shaderProgram);
	}
}