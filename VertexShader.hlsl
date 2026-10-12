// TODO: Part 2b 
[[vk::push_constant]] 
cbuffer SHADER_VARS
{
    matrix worldMatrix;
    float4 padding[4];
};

struct VertexOutput
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

VertexOutput main(float2 pos : POSITION, float4 color : COLOR)
{
    VertexOutput output;
    
    // Transform the position by the world matrix
    float4 position = float4(pos, 0, 1);
    output.position = mul(position, worldMatrix);
    
    // Pass the color to the fragment shader
    output.color = color;
    
    return output;
}