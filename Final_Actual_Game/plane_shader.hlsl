cbuffer CB0 
{
	matrix WVPMatrix;
	matrix WVMatrix;	// need this because has to be same size as reflect_shader to switch between these two shaders easily
	float4 directional_light_vector;
	float4 directional_light_colour;
	float4 ambient_light_colour;
};

Texture2D texture0;
SamplerState sampler0;

struct VOut
{
	float4 position : SV_POSITION;
	float4 color	: COLOR;
	float2 texcoord	: TEXCOORD;
};

VOut ModelVS(float4 position : POSITION, float2 texcoord : TEXCOORD, float3 normal : NORMAL)
{
	VOut output;

	//float4 default_color = { 1.0f, 1.0f, 1.0, 1.0 };
	//output.color = default_color;

    position.y += position.y;
	output.position = mul(WVPMatrix, position);

	float diffuse_amount = dot(directional_light_vector, normal);
	diffuse_amount = saturate(diffuse_amount);
	output.color = ambient_light_colour + (directional_light_colour * diffuse_amount);

    output.texcoord = texcoord * 20;

	return output;
}

float4 ModelPS(float4 position : SV_POSITION, float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	return texture0.Sample(sampler0, texcoord) * color;
}