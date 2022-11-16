#pragma once
#include <map>

#include "Shader.h"

class Render;

class ShaderAsset {

private:
	Render* m_render;

	std::map<unsigned int, Shader> m_shaders;

public:
	void Init(Render* render);

	void CompileShader(const fs::path& path);
	void RecompileShaders();

	bool HasShader(unsigned int hashCode) { return m_shaders.count(hashCode) > 0; }
	Shader* GetShader(unsigned int hashCode) { return &m_shaders.at(hashCode); }

	unsigned int GetShaderHash(const fs::path& path) { return std::hash<std::string>()(path.string()); }

private:
	bool m_CompileShader(const fs::path& path, Shader* shader);

};

