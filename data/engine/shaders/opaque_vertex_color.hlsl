struct VS_IN {
    float4 pos : POSITION;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 uv : TEXCOORD;
};

struct PS_IN {
    float4 pos : SV_POSITION;
    float4 worldPos : TEXCOORD0;
    float4 color : COLOR;
    float4 normal : NORMAL;
    float4 uv : TEXCOORD1;
};

struct MeshData {
    float4x4 wvp;
    float4x4 world;
    float3 cameraPosition;
};

struct MaterialData {
    float3 diffuseColor;
    float diffuse;	// Kd
	float ambient;	// Ka
	float specular;	// Ks
	float shininess;// spec pow (Ns)
};

struct DirectionLightData {
    float3 direction;
    float intensity;
	float3 color;
};

struct PSOpaque {
    float4 diffuseRGB : SV_Target0;
    float4 normal : SV_Target1;
    float4 vertexColor : SV_Target2;
    float4 worldPos : SV_Target3;
    float4 matParams : SV_Target4;
};


// Buf_OpaquePass_Light_PS
cbuffer PS_DirectionLightData : register(b1) { DirectionLightData dirLight; }

// Buf_Material_PS
cbuffer PS_MaterialData : register(b2) { MaterialData material; }

// Buf_Mesh_VS + Buf_Mesh_PS
cbuffer VS_PS_MeshData : register(b3) { MeshData meshData; }

// Res_RenderPass_PS
Texture2D ShadowMap : register(t0);
SamplerComparisonState CompSampler : register(s0);

// Res_Material_PS
Texture2D DiffuseMap : register(t8);
SamplerState Sampler : register(s8);


PS_IN VSMain(VS_IN input) {
    PS_IN output = (PS_IN)0;

    input.pos.w = 1.0f;
    input.normal.w = 0.0f;

    // output.normal = normalize(mul(float4(input.normal.xyz, 0.0f), meshData.world));

    output.worldPos = mul(input.pos, meshData.world);
    output.pos = mul(input.pos, meshData.wvp);
    output.color = input.color;
    output.normal = normalize(mul(input.normal, meshData.world));
    output.normal = float4(output.normal.xyz, 1);
    output.uv = float4(input.uv.xy, 1, 1);

    return output;
}

// float4 PSMain(PS_IN input) : SV_Target0{
//     float4 color = input.color;
//     return color;
// }

PSOpaque PSMain(PS_IN input) {
    PSOpaque output = (PSOpaque)0;

    output.diffuseRGB = input.color;

    output.matParams.r = material.diffuse;
    output.matParams.g = material.ambient;
    output.matParams.b = material.specular;
    output.matParams.a = material.shininess;

    output.normal = normalize(input.normal);
    output.vertexColor = input.color;
    output.worldPos = input.worldPos;

    return output;
}
