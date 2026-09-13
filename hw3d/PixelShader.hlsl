// A colour is represnted as 4D (RGB and Alpha)
struct PSInput
{
	float4 pos : SV_POSITION;
	float3 color : Color;
};

float4 main( PSInput input ) : SV_TARGET
{
	// Output the interpolated vertex color with full alpha
	return float4(input.color, 1.0f);
}
