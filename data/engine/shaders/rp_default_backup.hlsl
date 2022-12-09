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

float invLerp(float from, float to, float value){
  return value - from;
}

float remap(float origFrom, float origTo, float targetFrom, float targetTo, float value){
  float rel = invLerp(origFrom, origTo, value);
  return lerp(targetFrom, targetTo, rel);
}

float4 PSMain(PS_IN input) : SV_Target {

    // wpos пикселя в texcoord shadow map
    float3 smuv = mul(input.worldPos, dirLight.uvMatrix);
    smuv.y = 1 - smuv.y;
    smuv.xy = smuv.xy / 1;

    float bias = 0.0008;
    float x = smuv.z - bias;
    float shadow = ShadowMap.SampleCmp(CompSampler, smuv.xy, x);

    float4 diffuseTexColor = DiffuseMap.Sample(Sampler, float2(input.uv.x, 1-input.uv.y));
    clip(diffuseTexColor.a - 0.01f);

    float4 diffuseColor = float4(material.diffuseColor.xyz, 1.0);

    float3 kd = diffuseTexColor.xyz * input.color.xyz;
    float3 normal = normalize(input.normal.xyz);

    float3 viewDir = normalize(meshData.cameraPosition.xyz - input.worldPos.xyz);
    float3 lightDir = -dirLight.direction.xyz;
    float3 refVec = normalize(reflect(lightDir, normal));

    float3 diffuse = max(0, dot(lightDir, normal)) * kd;
    float3 ambient = kd * material.ambient;
    float3 spec = pow(max(0, dot(-viewDir, refVec)), material.shininess) * material.specular;

    float3 das = ambient + (diffuse + spec) * shadow;
    float3 color = dirLight.color * dirLight.intensity.xxx * das;

    return float4(color, 1.0f);
}


