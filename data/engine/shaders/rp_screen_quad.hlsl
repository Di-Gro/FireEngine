
struct PS_IN {
    float4 pos : SV_POSITION;
    float4 uv : TEXCOORD;
};

// Res_Material_PS
Texture2D DiffuseMap : register(t8);
SamplerState Sampler : register(s8);


PS_IN VSMain(uint id: SV_VertexID) {
    PS_IN output = (PS_IN)0;

    float2 uv = float2(id % 2, (id % 4) >> 1);
    output.uv = float4(uv.x, uv.y, 0, 0);
    output.pos = float4((output.uv.x -0.5f) * 2, -(output.uv.y - 0.5f) * 2, 0, 1);

    // float2 uv = float2(id % 1, (id % 2) >> 1);
    // output.uv = float4(uv.x, uv.y, 0, 0);
    // output.pos = float4(uv * float2(2, -2) + float2(-1, 1), 0, 1);

    // float2 uv = float2((id << 1) & 2, id & 2);
    // output.uv = float4(uv.x, uv.y, 0, 0);
    // output.pos = float4(uv.x * 2 - 1, -uv.y * 2 + 1, 0, 1);
    return output;
}

float4 PSMain(PS_IN input) : SV_Target {
    // return float4(0,1,1,1);
    return DiffuseMap.Sample(Sampler, float2(input.uv.x, input.uv.y));
}

