cbuffer WorldBuffer : register (b0)
{
     float4x4 world;
};

cbuffer SceneBuffer : register (b1)
{
     float4x4 viewProj;
};

struct VSInput
{
     float3 position : POSITION;
     float4 color : COLOR;
};

struct VSOutput
{
     float4 position : SV_Position;
     float4 color : COLOR;
};

VSOutput main(VSInput input)
{
     VSOutput output;
     output.position = mul(viewProj, mul(world, float4(input.position, 1.0f)));
     output.color = input.color;

     return output;
}
