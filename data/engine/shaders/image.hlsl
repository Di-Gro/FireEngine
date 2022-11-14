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

cbuffer VS_MeshData : register(b0) {
    MeshData meshData;
}

cbuffer PS_MaterialData : register(b1) {
    MaterialData material;
}

cbuffer PS_DirectionLightData : register(b2) {
    DirectionLightData dirLight;
}

Texture2D DiffuseMap : register(t0);
Texture2D ShadowMap : register(t1);
SamplerState Sampler : register(s0);

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

    //float3 shadowMapUV = mul(input.uv, dirLight.uvMatrix);
    // float4 shadowDepth = ShadowMap.Sample(Sampler, shadowMapUV.xy);
    // float2 shadowMapUV = float2(input.uv.x, 1-input.uv.y)
    // float4 shadowDepth = ShadowMap.Sample(Sampler, float2(input.uv.x, 1-input.uv.y));
    // shadowDepth = float4(1,1,1,1) - shadowDepth.xxxx;
    //
    // return saturate(shadowDepth * 1.0f);

    float4 color = DiffuseMap.Sample(Sampler, float2(input.uv.x, 1-input.uv.y));
    // color = float4(1,1,1,1) - color.xxxx;
    color = color.xxxx;
    return saturate(color * 1.0f);
}
