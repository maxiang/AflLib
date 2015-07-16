float3 LightDir = {0.1f,-0.5f,-0.5f};
float4 MaterialDiffuse : MATERIALDIFFUSE = {0.3f, 0.3f, 0.3f, 1.0f};
float4 MaterialAmbient  = {0.1f, 0.1f, 0.1f, 1.0f};

float4x4    World;
float4x4    View;
float4x4    ViewProj;

struct VS_INPUT
{
    float4  Pos             : POSITION;
    float3  Normal          : NORMAL;
    float3  Tex0            : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4 Pos             : POSITION;
    float4 Color           : COLOR0;
    float3 Tex0            : TEXCOORD0;
};

VS_OUTPUT VS(VS_INPUT i)
{
    float3 Normal;
    float3 Light = -LightDir;
    
	VS_OUTPUT o;
	float3 sPos;
	o.Pos = mul(i.Pos,World);
	sPos = mul(o.Pos,View);
	sPos.xyz *= sPos.xyz;
	o.Pos.z -= (sPos.x + sPos.y+sPos.z)/100000;
	o.Pos = mul(o.Pos,ViewProj);
	
	Normal = normalize(mul(i.Normal.xyz,(float3x3)World));
   
    o.Color.xyz = MaterialAmbient.xyz + max(0.3,dot(Normal,Light)) * MaterialDiffuse.xyz;
    o.Color.w = MaterialDiffuse.z;
	o.Tex0 = i.Tex0;

	return o;
}

technique field
{
    pass P0
    {
        VertexShader = compile vs_1_1 VS();
    }
}


