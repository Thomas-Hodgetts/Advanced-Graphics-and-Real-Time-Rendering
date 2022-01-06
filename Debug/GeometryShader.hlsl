struct VS_OUTPUT2
{
	float4 pos : SV_POSITION;
	float2 texCoord: TEXCOORD;
};

[maxvertexcount(3)]
void main(triangle VS_OUTPUT2 input[3] : SV_POSITION, inout TriangleStream<VS_OUTPUT2> output)
{
	VS_OUTPUT2 vs2 = (VS_OUTPUT2)0;
	for (uint i = 0; i < 3; i++)
	{
		vs2 = input[i];
		output.Append(vs2);
	}
}
