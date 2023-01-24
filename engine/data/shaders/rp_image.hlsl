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
    float4x4 uvMatrix;
    float3 direction;
    float intensity;
	float3 color;
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

    output.worldPos = mul(input.pos, meshData.world);
    output.pos = mul(input.pos, meshData.wvp);
    output.color = input.color;
    output.normal = normalize(mul(input.normal, meshData.world));
    output.uv = input.uv;

    return output;
}

float4 PSMain(PS_IN input) : SV_Target {
    float4 color = DiffuseMap.Sample(Sampler, float2(input.uv.x, 1-input.uv.y));
    color = color.xxxx;
    return saturate(color * 1.0f);
}
