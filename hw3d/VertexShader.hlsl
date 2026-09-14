// We are creating homogenius coordinates for the matrix transformation of the triangle
float4 main( float2 pos : Position ) : SV_Position
{
	// We return the 4-dimensional value for the 2D triangle
    // 4D is represented by (X, Y, Z, W)
    return float4(pos.x, pos.y, 0.0f, 1.0f);
}