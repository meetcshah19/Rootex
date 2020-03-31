cbuffer CBuf : register(b0)
{
	matrix M;
};

cbuffer CBuf : register(b1)
{
	matrix V;
};

cbuffer CBuf : register(b2)
{
	matrix P;
};

float4 main(float3 pos : POSITION) : SV_POSITION
{
	return mul(float4(pos, 1.0f), mul(M, mul(V, P)));
}
