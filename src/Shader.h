#pragma once

#include "GLOBALS.h"
#if (ENABLE_GUI)
#include <unordered_map>
#include <string>

#include "glm/glm.hpp"

struct ShaderProgramSource
{
	string VertexSource;
	string FragmentSource;
};

class Shader
{
private:
	std::filesystem::path m_FilePath;
	unsigned int m_RendererID;
	//caching for uniform
	std::unordered_map<string, int> m_UniformLocationCache;
public:
	Shader(const std::filesystem::path& filepath);
	~Shader();

	void Bind() const;
	void Unbind() const;

	//set uniforms
	void SetUniform1i(const string& name, int val);
	void SetUniform1f(const string& name, float val);
	void SetUniform2f(const string& name, const glm::vec2& val);
	void SetUniform3f(const string& name, const glm::vec3& val);
	void SetUniform4f(const string& name, const glm::vec4& val);
	void SetUniformMat3f(const string& name, const glm::mat3& matrix);
	void SetUniformMat4f(const string& name, const glm::mat4& matrix);
private:
	ShaderProgramSource ParseShader(const std::filesystem::path& filepath);
	unsigned int CompileShader(unsigned int type, const string& source);
	unsigned int CreateShader(const string& vertexShader, const string& fragmentShader);
	int GetUniformLocation(const string& name);
};

#endif //ENABLE_GUI
