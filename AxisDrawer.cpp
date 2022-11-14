//#include "AxisDrawer.h"
//
//void AxisDrawer::OnStart() {
//	std::vector<Mesh4::Vertex> verteces;
//	std::vector<int> indeces = { 0,1, 2,3, 4,5 };
//
//	Vector4 xColor = { 1, 0, 0, 1 };
//	Vector4 yColor = { 0, 1, 0, 1 };
//	Vector4 zColor = { 1, 1, 1, 1 };
//
//	auto& vx0 = verteces.emplace_back();
//	vx0.position = Vector4::Zero;
//	vx0.color = xColor;
//
//	auto& vx1 = verteces.emplace_back();
//	vx1.position = Vector4(Vector3::Right * length);
//	vx1.color = xColor;
//
//	auto& vy0 = verteces.emplace_back();
//	vy0.position = Vector4::Zero;
//	vy0.color = yColor;
//
//	auto& vy1 = verteces.emplace_back();
//	vy1.position = Vector4(Vector3::Up * length);
//	vy1.color = yColor;
//
//	auto& vz0 = verteces.emplace_back();
//	vz0.position = Vector4::Zero;
//	vz0.color = zColor;
//
//	auto& vz1 = verteces.emplace_back();
//	vz1.position = Vector4(Vector3::Forward * length);
//	vz1.color = zColor;
//
//	CreateMesh(&verteces, &indeces);
//	mesh()->topology = D3D11_PRIMITIVE_TOPOLOGY_LINELIST;
//}