cbuffer CBUffer0 
{
	matrix WVPMatrix;	// 64 bytes
	float4 directional_light_vector;
	float4 directional_light_colour;
	float4 ambient_light_colour;
	float scale;		// 4 bytes
	float transformX;	// 4 bytes
	float transformY;	// 4 bytes
	float packing;		// 4
}

Texture2D texture0;
SamplerState sampler0;

struct VOut
{
	float4 position : SV_POSITION;
	float4 color	: COLOR; //Note the spelling of this and all instances below
	float2 texcoord : TEXCOORD;
};

VOut VShader(float4 position : POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD, float3 normal : NORMAL)
{
	VOut output;

	output.position = mul(WVPMatrix, position);

	/*float diffuse_amount = dot(directional_light_vector, normal);
	diffuse_amount = saturate(diffuse_amount);
	output.color = ambient_light_colour + (directional_light_colour * diffuse_amount);*/

	output.texcoord = texcoord;

	return output;
}

float4 PShader(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	color.rgba = float4(0.1f, 0.5f, 0.9f, 1.0f);

	//return color;
	return color * texture0.Sample(sampler0, texcoord);
}