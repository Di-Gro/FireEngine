#pragma once
#include <map>

#include "Shader.h"

class Render;

class ShaderAsset {
private:
	Render* m_render;

	std::map<size_t, Shader> m_shaders;

public:
	void Init(Render* render);

	void CompileShader(const fs::path& path);
	void RecompileShaders();

	bool HasShader(size_t hashCode) { return m_shaders.count(hashCode) > 0; }
	Shader* GetShader(size_t hashCode) { return &m_shaders.at(hashCode); }
	Shader* GetShader(const fs::path& path) { return GetShader(GetShaderHash(path)); }

	size_t GetShaderHash(const fs::path& path) { return std::hash<std::string>()(path.string()); }

private:
	bool m_CompileShader(const fs::path& path, Shader* shader);

	bool m_BuildShaders();

};

