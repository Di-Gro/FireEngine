#pragma once

#include <string>

#include "SimpleMath.h"
#include "wrl.h_d3d11_alias.h"
#include "FileSystem.h"

class Shader;

class Material {
	#pragma pack(push, 4)
	public: struct Data {
		Vector3 diffuseColor = Vector3::One;	// 12
		float diffuse = 1.0f;		// Kd			// 4
		float ambient = 0.8f;		// Ka			// 4
		float specular = 0.5f;		// Ks			// 4
		float shininess = 250.0f;	// spec pow Ns	// 4
		float _1[1];
	};
	#pragma pack(pop)

	public: class Texture {
	public:
		fs::path path;
		comptr<ID3D11Texture2D> texture2D;
		comptr<ID3D11ShaderResourceView> srv;

		bool empty() { return path.empty(); }
	};

public:
	std::string name;
	const Shader* shader;
	Texture diffuse;
	Data data;
};
