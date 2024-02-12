struct VSOutput
{
     float4 position : SV_Position;
     float4 color : COLOR;
};

float4 main(VSOutput input) : SV_Target0
{
     return input.color;
}
