//
// include
//
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
    float _f1;
};

// PASS_CB_CAMERA_PS 0
cbuffer PS_CameraData : register(b0) { CameraData cb_camera; }
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
//
// include
//

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

PS_IN VSMain(VS_IN input) {
    PS_IN output = (PS_IN)0;

    input.pos.w = 1.0f;
    input.normal.w = 0.0f;

    output.worldPos = mul(input.pos, cb_mesh.world);
    output.pos = mul(input.pos, cb_mesh.wvp);
    output.color = input.color;
    output.normal = normalize(mul(input.normal, cb_mesh.world));
    output.normal.a = 1;
    output.uv = float4(input.uv.xy, 1, 1);

    return output;
}

struct PSOpaque {
    float4 diffuseRGB : SV_Target0;
    float4 normal : SV_Target1;
    float4 vertexColor : SV_Target2;
    float4 worldPos : SV_Target3;
    float4 matParams : SV_Target4;
    uint actorId : SV_Target5;
};

float CheapContrast(float inp, float contrast) {
    float lerped = lerp(-contrast, contrast + 1, inp);
    return clamp(lerped, 0, 1);
}

float CameraDepthFade(PS_IN input, float fadeOffset = 200, float fadeLength = 500) {
    float cameraDist = length(input.worldPos.rgb - cb_camera.position);
    return clamp((cameraDist - fadeOffset) / fadeLength, 0, 1);
}

float2 GrideLines(float3 wpos, float3 normal, float size, float lineWidth) {
    float3 size3 = float3(size, size, size);
    float3 lineWidth3 = float3(lineWidth, lineWidth, lineWidth);

    float3 step_v = step(pow(fmod(wpos, size3), float3(2, 2, 2)), lineWidth3);

    float normal_r = abs(normal.r);
    float normal_b = abs(normal.b);

    float2 lerp1 = lerp(step_v.rb, step_v.gb, float2(normal_r, normal_r));
    float2 lerp2 = lerp(lerp1, step_v.rg, float2(normal_b, normal_b));

    return lerp2;
}

float3 Checker (
    float3 wpos, float3 normal,
    float size,
    float3 wallsTintColor,
    float3 floorTintColor,
    float contrast,
    float ptContrast /* Projection transition contrast*/ )
{
    float3 size3 = float3(size, size, size);
    float3 zero3 = float3(0, 0, 0);
    float3 one3 = float3(1, 1, 1);

    // Generationg a world space checher pattern
    float3 fmod1 = fmod(wpos, size3) - size3 / 2;
    float3 fmod2 = fmod(-wpos, size3) -  size3 / 2;
    float3 clampedWpos = clamp(wpos, zero3, one3);
    float3 lerped = lerp(1 - fmod2, fmod1, clampedWpos);

    // Splitting out pattern projections for X, Y, and Z directions
    float clamped1 = clamp(lerped.r * lerped.b, 0, 1); // Y
    float clamped2 = clamp(lerped.g * lerped.b, 0, 1); // X
    float clamped3 = clamp(lerped.r * lerped.g, 0, 1); // Z

    // Creating masks based on vertex normal
    float cheapedContrast_r = CheapContrast(abs(normal.r), ptContrast);
    float cheapedContrast_b = CheapContrast(abs(normal.b), ptContrast);

    // Blending projections together
    float lerped2 = lerp(clamped1, clamped2, cheapedContrast_r);
    float lerped3 = lerp(lerped2, clamped3, cheapedContrast_b);

    // Adding colour and contrast
    float lerped4 = lerp(contrast, 1, lerped3);
    float3 lerped4_3 = float3(lerped4, lerped4, lerped4);

    float3 cheapedContrast_b_3 = float3(cheapedContrast_b, cheapedContrast_b, cheapedContrast_b);
    float3 lerpedColor = lerp(wallsTintColor, floorTintColor, cheapedContrast_b_3);
    float3 res = lerped4_3 * lerpedColor;

    return res;
}

float3 GetColor(PS_IN input) {
    float subsCount = 10;
    float size1 = 100;
    float size2 = size1 / subsCount;
    float lineWidth1 = size1 / 100;
    float lineWidth2 = lineWidth1 / subsCount;
    float line2Intense = 0.2;
    float contrast = 0.6;
    float ptContrast = 0;
    float3 wallsTintColor = float3(0.5, 0.5, 0.5);
    float3 floorTintColor = float3(0.26, 0.26, 0.26);

    float2 lines1 = GrideLines(input.worldPos.rgb, input.normal.rgb, size1, lineWidth1);
    float2 lines2 = GrideLines(input.worldPos.rgb, input.normal.rgb, size2, lineWidth2);
    float3 checker = Checker(input.worldPos.rgb, input.normal.rgb, size1 * 2, wallsTintColor, floorTintColor, contrast, ptContrast);

    float cameraFade1 = CameraDepthFade(input, 500, 2000);
    float cameraFade2 = CameraDepthFade(input, 200, 500);
    float2 lines = lines1 * (1 - cameraFade1) + lines2 * (1 - cameraFade2) * line2Intense;

    float dot1 = dot(lines, float2(1, 1));
    float3 lerped = lerp(checker, float3(0.6, 0.6, 0.6), dot1);

    return lerped;
}

PSOpaque PSMain(PS_IN input) {
    PSOpaque output = (PSOpaque)0;

    float2 uv = float2(input.uv.x, 1-input.uv.y);

    output.diffuseRGB.rgb = GetColor(input);
    output.diffuseRGB.a = 1;

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
