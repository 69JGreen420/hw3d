// We are creating homogenius coordinates for the matrix transformation of the triangle
struct VSInput
{
    float2 pos : Position;
    float3 color : Color;
};

struct PSInput
{
    float4 pos : SV_POSITION;
    float3 color : Color;
};

PSInput main(VSInput input)
{
    PSInput output;
    // position -> 4D clip space
    output.pos = float4(input.pos.x, input.pos.y, 0.0f, 1.0f);
    // pass color through to pixel shader
    output.color = input.color;
    return output;
}
