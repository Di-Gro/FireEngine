
struct PS_IN {
    float4 pos : SV_POSITION;
    float4 uv : TEXCOORD;
};

Texture2D Selection : register(t0);
SamplerState Sampler1 : register(s0);

Texture2D SelectionBlur : register(t1);
SamplerState Sampler2 : register(s1);

PS_IN VSMain(uint id: SV_VertexID) {
    PS_IN output = (PS_IN)0;

    float2 uv = float2(id % 2, (id % 4) >> 1);
    output.uv = float4(uv.x, uv.y, 0, 0);
    output.pos = float4((output.uv.x -0.5f) * 2, -(output.uv.y - 0.5f) * 2, 0, 1);
    return output;
}

float4 PSMain(PS_IN input) : SV_Target {

    float2 screenPos = input.pos.xy;
    float2 uv = input.uv.xy;

    float3 color = float3(1, 0, 0);
    float intensity = 1;

    float prepass = Selection.Sample(Sampler1, uv).r;
    float blured = SelectionBlur.Sample(Sampler2, uv).r;
    float diff = max(0, (blured - prepass) * 2);
    float alpha = diff * intensity;

    return float4(color.rgb, alpha);
}


