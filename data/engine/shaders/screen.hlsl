
struct PS_IN {
    float4 pos : SV_POSITION;
    float4 uv : TEXCOORD;
};

struct DirectionLightData {
    float4x4 uvMatrix;
    float3 direction;
    float intensity;
	float3 color;
};

struct CameraData {
    float3 position;
};

// Buf_RenderPass_Camera_PS
cbuffer PS_CameraData : register(b0) { CameraData camera; }
// Buf_LightingPass_Light_PS
cbuffer PS_DirectionLightData : register(b1) { DirectionLightData dirLight; }

// Res_RenderPass_PS
Texture2D ShadowMap : register(t0);
SamplerComparisonState CompSampler : register(s0);

Texture2D gbuf_diffuseRGB : register(t1);
Texture2D gbuf_normal : register(t2);
Texture2D gbuf_emissive : register(t3);
Texture2D gbuf_worldPos : register(t4);
Texture2D gbuf_matParams : register(t5);

// Res_Material_PS
Texture2D DiffuseMap : register(t8);
SamplerState Sampler : register(s8);


float3 GetDiffuse(float2 screenPos){
    return gbuf_diffuseRGB.Load(float3(screenPos.xy, 0)).rgb;
}

float3 GetNormal(float2 screenPos){
    return gbuf_normal.Load(float3(screenPos.xy, 0)).rgb;
}

float3 GetVertexColor(float2 screenPos){
    return gbuf_emissive.Load(float3(screenPos.xy, 0)).rgb;
}

float3 GetWorldPos(float2 screenPos){
    return gbuf_worldPos.Load(float3(screenPos.xy, 0)).rgb;
}
float GetMatDiffuse(float2 screenPos){
    return gbuf_matParams.Load(float3(screenPos.xy, 0)).r;
}
float GetMatAmbient(float2 screenPos){
    return gbuf_matParams.Load(float3(screenPos.xy, 0)).g;
}
float GetMatSpecular(float2 screenPos){
    return gbuf_matParams.Load(float3(screenPos.xy, 0)).b;
}
float GetMatShininess(float2 screenPos){
    return gbuf_matParams.Load(float3(screenPos.xy, 0)).a;
}

PS_IN VSMain(uint id: SV_VertexID) {
    PS_IN output = (PS_IN)0;

    float2 uv = float2(id % 2, (id % 4) >> 1);
    output.uv = float4(uv.x, uv.y, 0, 0);
    output.pos = float4((output.uv.x -0.5f) * 2, -(output.uv.y - 0.5f) * 2, 0, 1);

    return output;
}

float4 PSMain(PS_IN input): SV_Target {

    // GBuffer
    float2 screenPos = input.pos.xy;

    float3 gbuf_diffuse = GetDiffuse(screenPos);
    float3 gbuf_vertexColor = GetVertexColor(screenPos);
    float3 gbuf_normal = GetNormal(screenPos);
    float3 gbuf_worldPos = GetWorldPos(screenPos);

    float gbuf_matDiffuse = GetMatDiffuse(screenPos);
    float gbuf_matSpecular = GetMatSpecular(screenPos);
    float gbuf_matAmbient = GetMatAmbient(screenPos);
    float gbuf_matShininess = GetMatShininess(screenPos);

    // Shadow
    float3 smuv = mul(gbuf_worldPos, dirLight.uvMatrix);
    smuv.y = 1 - smuv.y;

    float bias = 0.0008;
    float x = smuv.z - bias;
    float shadow = ShadowMap.SampleCmp(CompSampler, smuv.xy, x);

    // Directional Light
    float3 kd = gbuf_matDiffuse * gbuf_diffuse * gbuf_vertexColor;
    float3 normal = gbuf_normal;

    float3 viewDir = normalize(camera.position.xyz - gbuf_worldPos.xyz);
    float3 lightDir = dirLight.direction.xyz;
    float3 refVec = normalize(reflect(-lightDir, normal));

    float3 diffuse = max(0, dot(-lightDir, normal)) * kd;
    float3 ambient = kd * gbuf_matAmbient;
    float3 spec = pow(max(0, dot(-viewDir, refVec)), gbuf_matShininess) * gbuf_matSpecular;

    float3 das = ambient + (diffuse + spec) * shadow;
    float3 color = dirLight.color * dirLight.intensity.xxx * das;

	float3 cameraPos = camera.position.xyz;
	float3 vec = cameraPos - gbuf_worldPos;
    return float4(gbuf_diffuse.xyz, 1);
}

// float v = shadow;
// return float4(v,v,v, 1);
// // Directional Light
// float3 kd = gbuf_diffuse; //* gbuf_vertexColor;
/// ->
// float3 normal = normalize(gbuf_normal);fffsdf
//
// float3 viewDir = normalize(camera.position - gbuf_worldPos);
// float3 lightDir = normalize(dirLight.direction.xyz);
// float3 refVec = normalize(reflect(-lightDir, normal));
//
// float ndotl = saturate(dot(normal, -lightDir));
// // if(ndotl <= 0)
// //     discard;
//
// float3 diffuse = ndotl * gbuf_diffuse;
// float3 ambient = gbuf_diffuse * gbuf_matAmbient;
// float3 spec = pow(max(0, dot(-viewDir, refVec)), 10) * gbuf_matSpecular;
// //
// float3 das = ambient + (diffuse + spec);// * shadow;
// float3 color = dirLight.color * dirLight.intensity.xxx * das;
// // float3 color = dirLight.color * (diffuse + spec) * dirLight.intensity.xxx;
//
// float v = ndotl;
// // return float4(v,v,v, 1);
// return float4(color.rgb, 1);
// <-



