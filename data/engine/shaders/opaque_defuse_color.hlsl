//////ShaderPass///////ShaderPass///////ShaderPass/////ShaderPass///////ShaderPass
struct ActorData {
    uint actorId;
};

struct CameraData {
    float3 position;
};

struct ShadowData {
    float4x4 uvMatrix;
    float mapScale;
};
struct LightData {
    float3 position;   float param1;
    float3 direction;  float param2;
    float3 color;      float param3;
    float param4;
    float param5;
    float param6;
    float param7;
};

struct MaterialData {
    float3 diffuseColor;
    float diffuse;	// Kd
	float ambient;	// Ka
	float specular;	// Ks
	float shininess;// spec pow (Ns)
};
struct MeshData {
    float4x4 wvp;
    float4x4 world;
    float3 cameraPosition;
};
// PASS_CB_CAMERA_PS 0
// cbuffer PS_CameraData : register(b0) { CameraData cb_camera; }

// PASS_CB_SHADOW_PS 1
// cbuffer PS_ShadowData : register(b1) { ShadowData cb_shadow; }

// PASS_CB_LIGHT_PS 2
// cbuffer PS_LightData : register(b2) { LightData cb_light; }

// PASS_CB_MATERIAL_PS 3
cbuffer PS_MaterialData : register(b3) { MaterialData cb_material; }

// PASS_CB_MESH_PS 4
cbuffer VS_PS_MeshData : register(b4) { MeshData cb_mesh; }

// PASS_CB_ACTOR_PS 5
cbuffer PS_ActorData : register(b5) { ActorData cb_actor; }

// PASS_R_PASS_PS 0
// Texture2D ShadowMap : register(t0);
// SamplerComparisonState CompSampler : register(s0);

// GBuffer
// Texture2D gbuf_diffuseRGB : register(t1);
// Texture2D gbuf_normal : register(t2);
// Texture2D gbuf_vertexColor : register(t3);
// Texture2D gbuf_worldPos : register(t4);
// Texture2D gbuf_matParams : register(t5);
//
// float3 GetDiffuse(float2 screenPos){ return gbuf_diffuseRGB.Load(float3(screenPos.xy, 0)).rgb; }
// float3 GetNormal(float2 screenPos){ return gbuf_normal.Load(float3(screenPos.xy, 0)).rgb; }
// float3 GetVertexColor(float2 screenPos){ return gbuf_vertexColor.Load(float3(screenPos.xy, 0)).rgb; }
// float3 GetWorldPos(float2 screenPos){ return gbuf_worldPos.Load(float3(screenPos.xy, 0)).rgb; }
// float GetMatDiffuse(float2 screenPos){ return gbuf_matParams.Load(float3(screenPos.xy, 0)).r; }
// float GetMatAmbient(float2 screenPos){ return gbuf_matParams.Load(float3(screenPos.xy, 0)).g; }
// float GetMatSpecular(float2 screenPos){ return gbuf_matParams.Load(float3(screenPos.xy, 0)).b;}
// float GetMatShininess(float2 screenPos){ return gbuf_matParams.Load(float3(screenPos.xy, 0)).a; }

//////ShaderPass///////ShaderPass///////ShaderPass/////ShaderPass///////ShaderPass

// Res_Material_PS
Texture2D DiffuseMap : register(t8);
SamplerState Sampler : register(s8);

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

struct PSOpaque {
    float4 diffuseRGB : SV_Target0;
    float4 normal : SV_Target1;
    float4 vertexColor : SV_Target2;
    float4 worldPos : SV_Target3;
    float4 matParams : SV_Target4;
    uint actorId : SV_Target5;
};

PS_IN VSMain(VS_IN input) {
    PS_IN output = (PS_IN)0;

    input.pos.w = 1.0f;
    input.normal.w = 0.0f;

    output.worldPos = mul(input.pos, cb_mesh.world);
    output.pos = mul(input.pos, cb_mesh.wvp);
    output.color = input.color;
    output.normal = normalize(mul(input.normal, cb_mesh.world));
    output.normal = float4(output.normal.xyz, 1);
    output.uv = float4(input.uv.xy, 1, 1);

    return output;
}

PSOpaque PSMain(PS_IN input) {
    PSOpaque output = (PSOpaque)0;

    output.diffuseRGB = float4(cb_material.diffuseColor.rgb, 1);

    output.matParams.r = cb_material.diffuse;
    output.matParams.g = cb_material.ambient;
    output.matParams.b = cb_material.specular;
    output.matParams.a = cb_material.shininess;

    output.normal = input.normal;
    output.vertexColor = input.color;
    output.worldPos = input.worldPos;

    output.actorId = cb_actor.actorId;

    return output;
}
