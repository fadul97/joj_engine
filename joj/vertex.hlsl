cbuffer cbPerObject : register(b0)
{
    float4x4 WorldViewProj;
}

struct VertexIn
{
    float3 PosL : POSITION;
    float4 Color : COLOR;
};

struct VertexOut
{
    float4 PosH : SV_POSITION;
    float4 Color : COLOR;
};

VertexOut main(VertexIn vin)
{
    VertexOut vout;

    // Transform the position to the homogeneous clip space
    vout.PosH = mul(float4(vin.PosL, 1.0f), WorldViewProj);

    // Just pass the color from the vertex to the pixel shader
    vout.Color = vin.Color;

    return vout;
}