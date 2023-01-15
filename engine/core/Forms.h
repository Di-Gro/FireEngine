#pragma once

#include <vector>

#include <d3d11.h>
#include <SimpleMath.h>

#include "Mesh.h"
#include "CSBridge.h"

using namespace DirectX::SimpleMath;

class MyColors {
public:
	static const Vector4 LightPink;
	static const Vector4 SkyBlue;
	static const Vector4 SkyWhite;
};

namespace Forms {

	extern Vector4 MainColor;

	class Form {
	public:
		std::vector<Vector4> points;
		std::vector<int> indexes;
		D3D_PRIMITIVE_TOPOLOGY topology;

		Form() {}

		Form(const Form&) = delete;

		Form(Form&& other) noexcept {
			(*this) = std::move(other);
		}

		Form& operator=(Form&& other) noexcept {
			if (this != &other) {
				points = std::move(other.points);
				indexes = std::move(other.indexes);
				topology = other.topology;
			}
			return *this;
		}
	};

	Form Box(Vector3 size, Vector4 color, const Vector4* mainColor = nullptr);
	Form Sphere(float radius, int latitudes, int longitudes, Vector4 color, const Vector4* mainColor = nullptr);
	Form HalfSphere(float radius, int latitudes, int longitudes, Vector4 color, int normals, const Vector4* mainColor = nullptr);

};

namespace Forms4 {

	class Form {
	public:
		std::vector<Mesh4::Vertex> verteces;
		std::vector<int> indexes;
		D3D_PRIMITIVE_TOPOLOGY topology;

		Form() {}

		Form(const Form&) = delete;

		Form(Form&& other) noexcept {
			(*this) = std::move(other);
		}

		Form& operator=(Form&& other) noexcept {
			if (this != &other) {
				verteces = std::move(other.verteces);
				indexes = std::move(other.indexes);
				topology = other.topology;
			}
			return *this;
		}
	};

	Form SphereLined(float radius, int latitudes, int longitudes, Vector4 color = { 1, 1, 1, 1 });
	Form Sphere(float radius, int latitudes, int longitudes, Vector4 color = { 1, 1, 1, 1 });
	
	Form HalfSphereLined(float radius, int latitudes, int longitudes, Vector4 color = { 1, 1, 1, 1 });
	Form HalfSphere(float radius, int latitudes, int longitudes, Vector4 color = { 1, 1, 1, 1 });

	Form Box(Vector3 size, Vector4 color = { 1, 1, 1, 1 });
	Form BoxLined(Vector3 size, Vector4 color = { 1, 1, 1, 1 });
}
