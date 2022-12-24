
struct PS_IN {
    float4 pos : SV_POSITION;
    float4 uv : TEXCOORD;
};

Texture2D Texture : register(t0);
SamplerState Sampler : register(s0);

float4 SampleSelection(float2 uv){ return Texture.Sample(Sampler, uv.xy); }

float2 GetTexelSize() {
    uint width, height;
    Texture.GetDimensions(width, height);
    return float2(1.0f / (float)width, 1.0f / (float)height);
}

float4 MakeBlur(PS_IN input) {
    float2 uv = input.uv.xy;
    float2 texelSize = GetTexelSize() * 2;

    float4 color = SampleSelection(uv);
    color += SampleSelection(uv + float2(-1, 1) * texelSize);
    color += SampleSelection(uv + float2( 1, 1) * texelSize);
    color += SampleSelection(uv + float2( 1,-1) * texelSize);
    color += SampleSelection(uv + float2(-1,-1) * texelSize);

    color += SampleSelection(uv + float2( 0,  1) * texelSize);
    color += SampleSelection(uv + float2( 0, -1) * texelSize);
    color += SampleSelection(uv + float2( 1,  0) * texelSize);
    color += SampleSelection(uv + float2(-1,  0) * texelSize);

    return color / 9.0f;
}

PS_IN VSMain(uint id: SV_VertexID) {
    PS_IN output = (PS_IN)0;

    float2 uv = float2(id % 2, (id % 4) >> 1);
    output.uv = float4(uv.x, uv.y, 0, 0);
    output.pos = float4((output.uv.x -0.5f) * 2, -(output.uv.y - 0.5f) * 2, 0, 1);
    return output;
}

float4 PSMain(PS_IN input) : SV_Target {
    return MakeBlur(input);
}
