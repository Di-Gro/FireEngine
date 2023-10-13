#pragma once

#include <SimpleMath.h>

using namespace DirectX::SimpleMath;

/// ¬ HLSL пол€ структур выравниваютс€ по 16 байт 
/// и упаковываютс€ в прошлые 16 байт, если вмещаютс€.

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
