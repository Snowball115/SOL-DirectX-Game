cbuffer CB0 
{
	matrix WVPMatrix;	// World view projection
	matrix WVMatrix;	// World view (used for reflection)
	float4 directional_light_vector;
	float4 directional_light_colour;
	float4 ambient_light_colour;
};

TextureCube cube0;
SamplerState sampler0;

struct VOut
{
	float4 position : SV_POSITION;
	float4 color	: COLOR;
	float3 texcoord	: TEXCOORD;
};

VOut ModelVS(float4 position : POSITION, float3 texcoord : TEXCOORD, float3 normal : NORMAL)
{
	VOut output;

    output.color = float4(1.0f, 1.0f, 1.0, 1.0);

	output.position = mul(WVPMatrix, position);

	//float diffuse_amount = dot(directional_light_vector, normal);
	//diffuse_amount = saturate(diffuse_amount);
	//output.color = ambient_light_colour + (directional_light_colour * diffuse_amount);

	// Position relative to camera
	float3 wvpos = mul(WVMatrix, position);
	// Surface normal relative to camera
	float3 wvnormal = mul(WVMatrix, normal);
	wvnormal = normalize(wvnormal);
	// Obtain the reverse eye vector
	float3 eyer = -normalize(wvpos);
	// Compute the reflection vector
	output.texcoord = 2.0f * dot(eyer, wvnormal) * wvnormal - eyer;

	return output;
}

float4 ModelPS(float4 position : SV_POSITION, float4 color : COLOR, float3 texcoord : TEXCOORD) : SV_TARGET
{
	return cube0.Sample(sampler0, texcoord) * color;
}