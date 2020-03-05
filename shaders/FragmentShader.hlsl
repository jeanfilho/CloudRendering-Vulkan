#pragma shader_stage(fragment)

struct PixelInputType
{
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

float4 main() : SV_TARGET
{
	return input.color;;
}