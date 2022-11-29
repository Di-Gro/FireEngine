//#pragma once
//
//#include <vector>
//#include <string>
//
//#include <d3d11.h>
//#include <SimpleMath.h>
//
//#include "Forms.h"
//#include "Mesh.h"
//#include "Game.h"
//
//using namespace DirectX::SimpleMath;
//
//class DynamicMeshComponent : public Component {
//
//public:
//	bool isDebug = false;
//	bool visible = true;
//
//private:
//	Mesh4* m_mesh = nullptr;
//	std::vector<const Material*> m_materials;
//	std::vector<Material*> m_dynamicMaterials;
//
//public:
//	const Mesh4* mesh() { return m_mesh; }
//
//	void AddShape( std::vector<Mesh4::Vertex>* verteces, std::vector<int>* indeces, size_t materialIndex = 0);
//	void SetMaterial(size_t index, const fs::path& shaderPath);
//	void RemoveMaterial(int index);
//	void ClearMesh();
//
//	void OnInit() override;
//	void OnDraw() override;
//	void OnDrawDebug() override;
//	void OnDestroy() override;
//
//
//private:
//	void m_Draw();
//	void m_DeleteResources();
//
//	void m_FillByDefaultMaterial(int targetSize);
//
//};
//
