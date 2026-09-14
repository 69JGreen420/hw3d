// A colour is represnted as 4D (RGB and Alpha)
// However we are not using alpha
float4 main(float3 color : Color) : SV_TARGET
{
	// Since colour is now determined by the rasterizer, we can return colour
	// Note that we need to expand the return value into a 4D element
    return float4(color, 1.0f);
}