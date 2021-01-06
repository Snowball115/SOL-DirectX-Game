cbuffer CBuffer0 
{
	matrix wvp;
	float4 color;
};

struct VOut 
{
	float4 position : SV_POSITION;
	float4 pcolor : COLOR;
	float2 texcoord : TEXCOORD;
};

VOut VShader(float4 position : POSITION) 
{
	VOut output;
	output.position = mul(wvp, position);
	output.pcolor = color;
	output.texcoord = position.xy;

	return output;
}

float4 PShader(float4 position : SV_POSITION, float4 pcolor : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	float distsq = (texcoord.x * texcoord.x) + (texcoord.y * texcoord.y);
	clip(1.0f - distsq);
	return pcolor;
}