#include "Forms.h"

#include <vector>
#include <iostream>

#include <d3d11.h>
#include <SimpleMath.h>

#include "Math.h"

using namespace DirectX::SimpleMath;

const Vector4 MyColors::LightPink = { 231 / 255.0f, 159 / 255.0f, 196 / 255.0f, 1.0f };
const Vector4 MyColors::SkyBlue = { 0, 119 / 255.0f, 253 / 255.0f, 1.0f };
const Vector4 MyColors::SkyWhite = { 166 / 255.0f, 236 / 255.0f, 250 / 255.0f, 1.0f };

namespace Forms {

	Vector4 MainColor = MyColors::LightPink;

	Form Box(Vector3 size, Vector4 color, const Vector4* mainColor) {
		auto half = size / 2;
		auto a = -half;
		auto b = half;

		Form form;
		form.topology = D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST; 

		Vector4 firstColor = mainColor != nullptr ? *mainColor : Forms::MainColor;

		form.points = {
			{a.x, a.y, a.z, 1.0f}, color,
			{a.x, a.y, b.z, 1.0f}, color,
			{b.x, a.y, b.z, 1.0f}, color,
			{b.x, a.y, a.z, 1.0f}, color,

			{a.x, b.y, a.z, 1.0f}, firstColor,
			{a.x, b.y, b.z, 1.0f}, firstColor,
			{b.x, b.y, b.z, 1.0f}, firstColor,
			{b.x, b.y, a.z, 1.0f}, firstColor,
		};

		form.indexes = {
			3,2,1, 1,0,3, // низ
			4,5,6, 6,7,4, // верх
			0,1,5, 4,0,5, // лево
			2,3,7, 7,6,2, // право
			0,4,7, 7,3,0, // перед
			1,2,6, 6,5,1, // зад
		};

		return form;
	}

	Form Sphere(float radius, int latitudes, int longitudes, Vector4 color, const Vector4* mainColor) {

		auto to1d = [&](int x, int y) { return x * (longitudes + 1) + y; };

		float dw = 360.0f / latitudes;
		float dh = 180.0f / longitudes;

		Form form;
		//form.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		form.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		struct P { int w; int h; };
		auto delta = {
			P{0,0}, P{1,0},
			P{1,0}, P{1,1},
			P{1,1}, P{0,0},

			P{0,0}, P{1,1},
			P{1,1}, P{0,1},
			P{0,1}, P{0,0},
		};
		if (form.topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
			delta = {
				P{0,0}, P{1,0}, P{1,1},
				P{1,1}, P{0,1}, P{0,0},
			};
		}

		Vector4 firstColor = mainColor != nullptr ? *mainColor : Forms::MainColor;

		for (int w = 0; w <= latitudes; ++w) {
			float u = w * dw;

			for (int h = 0; h <= longitudes; ++h) {
				float t = h * dh;

				float x, y, z;
				x = (float)(-radius * sin(rad(t)) * sin(rad(u)));
				y = (float)(-radius * cos(rad(t)));
				z = (float)(-radius * sin(rad(t)) * cos(rad(u)));

				float time = (y + radius) / (radius * 2);
				auto c = Color::Lerp(color, firstColor, time-0.2f);

				form.points.push_back({ x, y, z, 1.0f });
				//form.points.push_back({ u, t, 0, 1.0f });
				form.points.push_back(c);

				if (w < latitudes && h < longitudes) {
					int di = -1;
					for (P d : delta) {
						di++;
						if (form.topology == D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {
							if (h == 0 && di < 6)
								continue;
							if (h == longitudes - 1 && di >= 6)
								continue;
						}
						if (form.topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
							if (h == 0 && di < 3)
								continue;
							if (h == longitudes - 1 && di >= 3)
								continue;
						}
						form.indexes.push_back(to1d(w + d.w, h + d.h));
					}
				}
			}
		}

		std::cout << form.points.size() << " " << form.indexes.size() << std::endl;

		return std::move(form);
	}

	Form HalfSphere(float radius, int latitudes, int longitudes, Vector4 color, int normals, const Vector4* mainColor) {

		normals = normals >= 0 ? 1 : -1;
		auto to1d = [&](int x, int y) { return x * (longitudes + 1) + y; };

		float dw = 360.0f / latitudes;
		float dh = 180.0f / longitudes;

		longitudes /= 2;

		Form form;
		//form.topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
		form.topology = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

		struct P { int w; int h; };
		auto delta = {
			P{0,0}, P{1,0},
			P{1,0}, P{1,1},
			P{1,1}, P{0,0},

			P{0,0}, P{1,1},
			P{1,1}, P{0,1},
			P{0,1}, P{0,0},
		};
		if (form.topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
			delta = {
				P{0,0}, P{1,0}, P{1,1},
				P{1,1}, P{0,1}, P{0,0},
			};
		}

		Vector4 firstColor = mainColor != nullptr ? *mainColor : Forms::MainColor;

		for (int w = 0; w <= latitudes; ++w) {
			float u = w * dw;

			for (int h = 0; h <= longitudes; ++h) {
				float t = h * dh;

				float x, y, z;
				x = (float)(-radius * normals * sin(rad(t)) * sin(rad(u)));
				y = (float)(-radius * normals * cos(rad(t)));
				z = (float)(-radius * normals * sin(rad(t)) * cos(rad(u)));

				float time = h  / (float) longitudes;
				auto c = Color::Lerp(color, firstColor, time);

				form.points.push_back({ x, y, z, 1.0f });
				//form.points.push_back({ u, t, 0, 1.0f });
				form.points.push_back(c);

				if (w < latitudes && h < longitudes) {
					int di = -1;
					for (P d : delta) {
						di++;
						if (form.topology == D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {
							if (h == 0 && di < 6)
								continue;
							//if (h == longitudes - 1 && di >= 6)
							//	continue;
						}
						if (form.topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
							if (h == 0 && di < 3)
								continue;
							//if (h == longitudes - 1 && di >= 3)
							//	continue;
						}
						form.indexes.push_back(to1d(w + d.w, h + d.h));
					}
				}
			}
		}

		std::cout << form.points.size() << " " << form.indexes.size() << std::endl;

		return std::move(form);
	}


};


namespace Forms4 {

	Form Sphere(float radius, int latitudes, int longitudes, Vector4 color, D3D_PRIMITIVE_TOPOLOGY topology) {
		auto to1d = [&](int x, int y) { return x * (longitudes + 1) + y; };

		float dw = 360.0f / latitudes;
		float dh = 180.0f / longitudes;

		Form form;
		form.topology = topology;

		struct P { int w; int h; };
		auto delta = {
			P{0,0}, P{1,0},
			P{1,0}, P{1,1},
			P{1,1}, P{0,0},

			P{0,0}, P{1,1},
			P{1,1}, P{0,1},
			P{0,1}, P{0,0},
		};
		if (form.topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
			delta = {
				P{0,0}, P{1,0}, P{1,1},
				P{1,1}, P{0,1}, P{0,0},
			};
		}

		for (int w = 0; w <= latitudes; ++w) {
			float u = w * dw;

			for (int h = 0; h <= longitudes; ++h) {
				float t = h * dh;

				float x, y, z;
				x = (float)(-radius * sin(rad(t)) * sin(rad(u)));
				y = (float)(-radius * cos(rad(t)));
				z = (float)(-radius * sin(rad(t)) * cos(rad(u)));


				auto& vertex = form.verteces.emplace_back();
				vertex.position = { x, y, z, 0 };
				vertex.normal = vertex.position;
				vertex.normal.Normalize();
				vertex.color = color;

				if (w < latitudes && h < longitudes) {
					int di = -1;
					for (P d : delta) {
						di++;
						if (form.topology == D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {
							if (h == 0 && di < 6)
								continue;
							if (h == longitudes - 1 && di >= 6)
								continue;
						}
						if (form.topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
							if (h == 0 && di < 3)
								continue;
							if (h == longitudes - 1 && di >= 3)
								continue;
						}
						form.indexes.push_back(to1d(w + d.w, h + d.h));
					}
				}
			}
		}

		return std::move(form);
	}

	Form SphereLined(float radius, int latitudes, int longitudes, Vector4 color) {
		return Sphere(radius, latitudes, longitudes, color, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}

	Form Sphere(float radius, int latitudes, int longitudes, Vector4 color) {
		return Sphere(radius, latitudes, longitudes, color, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}

	static Form Box(Vector3 size, Vector4 color, D3D_PRIMITIVE_TOPOLOGY topology) {
		auto half = size / 2;
		auto a = -half;
		auto b = half;

		Form form;
		form.topology = topology;

		std::vector<Vector3> normals;
		std::vector<int> indexes;

		Vector4 points[] = {
			{a.x, a.y, a.z, 1.0f},
			{a.x, a.y, b.z, 1.0f},
			{b.x, a.y, b.z, 1.0f}, 
			{b.x, a.y, a.z, 1.0f}, 

			{a.x, b.y, a.z, 1.0f}, 
			{a.x, b.y, b.z, 1.0f}, 
			{b.x, b.y, b.z, 1.0f}, 
			{b.x, b.y, a.z, 1.0f}, 
		};

		if (topology == D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST) {
			indexes = {
				3,2,1, 1,0,3, // низ
				4,5,6, 6,7,4, // верх
				0,1,5, 4,0,5, // лево
				2,3,7, 7,6,2, // право
				0,4,7, 7,3,0, // перед
				1,2,6, 6,5,1, // зад
			};

			normals.insert(normals.end(), 6, Vector3::Down);
			normals.insert(normals.end(), 6, Vector3::Up);
			normals.insert(normals.end(), 6, Vector3::Left);
			normals.insert(normals.end(), 6, Vector3::Right);
			normals.insert(normals.end(), 6, Vector3::Forward);
			normals.insert(normals.end(), 6, Vector3::Backward);
		}
		else if (topology == D3D11_PRIMITIVE_TOPOLOGY_LINELIST) {
			indexes = {
				3,2, 2,1, 1,0, 0,3, 3,1, // низ
				4,5, 5,6, 6,7, 7,4, 4,6, // верх
				0,1, 1,5, 5,4, 4,0, 4,1, // лево
				2,3, 3,7, 7,6, 6,2, 3,6, // право
				0,4, 4,7, 7,3, 3,0, 0,7, // перед
				1,2, 2,6, 6,5, 5,1, 1,6, // зад
			};

			normals.insert(normals.end(), 10, Vector3::Down);
			normals.insert(normals.end(), 10, Vector3::Up);
			normals.insert(normals.end(), 10, Vector3::Left);
			normals.insert(normals.end(), 10, Vector3::Right);
			normals.insert(normals.end(), 10, Vector3::Forward);
			normals.insert(normals.end(), 10, Vector3::Backward);
		}
		for (int i = 0; i < indexes.size(); ++i) {
			int index = indexes[i];

			auto& vertex = form.verteces.emplace_back();
			vertex.position = points[index];
			vertex.normal = Vector4(normals[i]);
			vertex.color = color;

			form.indexes.push_back(i);
		}

		//for (auto& point : points) {
		//	auto& vertex = form.verteces.emplace_back();
		//	vertex.position = point;
		//	vertex.color = color;
		//}

		return form;
	}

	Form BoxLined(Vector3 size, Vector4 color) {
		return Box(size, color, D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	}
	
	Form Box(Vector3 size, Vector4 color) {
		return Box(size, color, D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	}


}