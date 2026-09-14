struct VSOut
{
    float3 color : Color;
    float4 pos : SV_Position;
};

// We are creating homogenius coordinates for the matrix transformation of the triangle
// We don't need semantics at the end as it's passed into the struct
VSOut main( float2 pos : Position, float3 color : Color )
{
    VSOut vso;
    // We return the 4-dimensional value for the 2D triangle
    // 4D is represented by (X, Y, Z, W)
    vso.pos = float4(pos.x, pos.y, 0.0f, 1.0f);
    vso.color = color; // Colour is determined by the rasterizer
	
    return vso;
}