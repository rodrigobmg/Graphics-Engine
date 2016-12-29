#include "Samplers.hlsli"

struct VertexOutput
{
    float4 PositionH : SV_POSITION;
    float2 TextureCoordinates : TEXCOORD;
};

Texture2D TextureMap : register(t0);

float4 main(VertexOutput input) : SV_Target
{
    return float4(TextureMap.Sample(SamplerAnisotropicClamp, input.TextureCoordinates).rrr, 1.0f);
}