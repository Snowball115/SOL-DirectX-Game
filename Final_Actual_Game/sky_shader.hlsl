cbuffer CBUffer0 
{
	matrix WVPMatrix;	// 64 bytes
}

TextureCube cube0;
SamplerState sampler0;

struct VOut
{
	float4 position : SV_POSITION;
	float3 texcoord : TEXCOORD;
};

VOut SkyVS(float4 position : POSITION, float3 texcoord : TEXCOORD)
{
	VOut output;

	output.texcoord = position.xyz;

	output.position = mul(WVPMatrix, position);

	return output;
}

float4 SkyPS(float4 position : SV_POSITION, float3 texcoord : TEXCOORD) : SV_TARGET
{
	//color.rgba = float4(0.1f, 0.5f, 0.9f, 1.0f);

	return cube0.Sample(sampler0, texcoord);
}