#pragma once

#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

/// � HLSL ���� �������� ������������� �� 16 ���� 
/// � ������������� � ������� 16 ����, ���� ���������.

namespace ShaderStructs {

#pragma pack(push, 4)
	struct CameraCBuffer {
		Vector3 position;
		float _1[1];
	};
#pragma pack(pop)

#pragma pack(push, 4)
	struct ShadowCBuffer {
		Matrix uvMatrix;
		float mapScale;
		float _f[3];
	};
#pragma pack(pop)

#pragma pack(push, 4)
	struct MeshCBuffer {
		Matrix wvpMatrix;
		Matrix worldMatrix;
		Vector3 cameraPosition;
		float _1[1];
	};
#pragma pack(pop)

}
