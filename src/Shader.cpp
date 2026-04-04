
#include "Shader.h"
#if (ENABLE_GUI)

#include "Renderer.h"

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#include <GL/glew.h>

Shader::Shader(const string& filepath)
	: m_FilePath(filepath), m_RendererID(0)
{
	//Creates shader from file
	auto source = ParseShader(filepath);
	m_RendererID = CreateShader(source.VertexSource, source.FragmentSource);
}

Shader::~Shader()
{  
	GLCall(glDeleteProgram(m_RendererID));
}

void Shader::Bind() const
{ 
	//binds prg to shader
	GLCall(glUseProgram(m_RendererID));
}

void Shader::Unbind() const
{     
	//binds prg to shader
	GLCall(glUseProgram(0));
}

void Shader::SetUniform1i(const string& name, int val)
{  
	GLCall(glUniform1i(GetUniformLocation(name), val));
}

void Shader::SetUniform1f(const string& name, float val)
{        
	GLCall(glUniform1f(GetUniformLocation(name), val));
}

void Shader::SetUniform2f(const string& name, const glm::vec2& val)
{      
	GLCall(glUniform2f(GetUniformLocation(name), val[0], val[1]));
}

void Shader::SetUniform3f(const string& name, const glm::vec3& val)
{        
	GLCall(glUniform3f(GetUniformLocation(name), val[0], val[1], val[2]));
}

void Shader::SetUniform4f(const string& name, const glm::vec4& val)
{
	GLCall(glUniform4f(GetUniformLocation(name), val[0], val[0], val[0], val[3]));
}

void Shader::SetUniformMat3f(const string& name, const glm::mat3& matrix)
{      
	GLCall(glUniformMatrix3fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

void Shader::SetUniformMat4f(const string& name, const glm::mat4& matrix)
{  
	GLCall(glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]));
}

ShaderProgramSource Shader::ParseShader(const string& filepath)
{
	std::ifstream ifstream(filepath);

	enum class ShaderType { NONE = -1, VETEX = 0, FRAGMENT = 1 };

	string lineContainer;
	std::stringstream ss[2];
	ShaderType currtype = ShaderType::NONE;
	while (std::getline(ifstream, lineContainer))
	{
		if (lineContainer.find("#shader") != string::npos)
		{
			if (lineContainer.find("vertex") != string::npos)
				currtype = ShaderType::VETEX;
			if (lineContainer.find("fragment") != string::npos)
				currtype = ShaderType::FRAGMENT;
		}
		else
		{
			ss[(int)currtype] << lineContainer << '\n';
		}
	}
	return { ss[0].str(), ss[1].str() };
}

unsigned int Shader::CompileShader(unsigned int type, const string& source)
{
	unsigned int id = 0;
	GLCall(id = glCreateShader(type));
	auto src = source.c_str();
	GLCall(glShaderSource(id, 1, &src, nullptr));
	GLCall(glCompileShader(id));

	int result;
	GLCall(glGetShaderiv(id, GL_COMPILE_STATUS, &result));
	if (result == GL_FALSE)
	{
		int length;
		GLCall(glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length));
		char* message = (char*)alloca(length * sizeof(char));

		GLCall(glGetShaderInfoLog(id, length, &length, message));
		GP::out << "Failed to compile shader" << std::endl;
		GP::out << message << std::endl;

		GLCall(glDeleteShader(id));
		return 0;
	}

	return id;
}

unsigned int Shader::CreateShader(const string& vertexShader, const string& fragmentShader)
{
	unsigned int programID = 0;
	GLCall(programID = glCreateProgram());
	auto vs = CompileShader(GL_VERTEX_SHADER, vertexShader);
	auto fs = CompileShader(GL_FRAGMENT_SHADER, fragmentShader);

	GLCall(glAttachShader(programID, vs));
	GLCall(glAttachShader(programID, fs));
	GLCall(glLinkProgram(programID));
	GLCall(glValidateProgram(programID));

	GLCall(glDeleteShader(vs));
	GLCall(glDeleteShader(fs));

#ifdef NDEBUG
	GLCall(glDetachShader(vs));
	GLCall(glDetachShader(fs));
#endif

	return programID;
}

int Shader::GetUniformLocation(const string& name)
{
	if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
		return m_UniformLocationCache[name];
	int location = 0;  

	GLCall(location = glGetUniformLocation(m_RendererID, name.c_str()));
	if (location == -1)
		GP::out << "Warning: uniform '" << name << "' doesn't exist" << std::endl;

	m_UniformLocationCache[name] = location;
	return location;
}

#endif //ENABLE_GUI
