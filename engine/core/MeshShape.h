#pragma once

#include <d3d11.h>
#include "wrl.h_d3d11_alias.h"

#include "Vertex.h"

struct MeshShape {
	Vertex* verteces = nullptr;
	int* indeces = nullptr;
	int materialIndex = -1;

	int vertecesSize = 0;
	int indecesSize = 0;

	comptr<ID3D11Buffer> vertexBuffer;
	comptr<ID3D11Buffer> indexBuffer;
	comptr<ID3D11Buffer> meshCBuffer;
};