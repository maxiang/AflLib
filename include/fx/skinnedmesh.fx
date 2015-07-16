float3 LightDir = {0.1f,-0.5f,-0.5f};
float4 MaterialDiffuse = {0.8f,0.8f,0.8f,1.0f};
float4 MaterialAmbient  = {0.1f, 0.1f, 0.1f, 1.0f};
float2 ShadowLong = {1.0f,600.0f};

float4x4 View: 		VIEW;
float4x4 ViewProj: 		VIEWPROJ;
float4x4 World:			WORLD;
float4x4 Proj : 		PROJECTION;
float4x4 WorldViewProj : 	WORLDVIEWPROJ;
float4x4 WorldView : 		WORLDVIEW;

static const int MAX_MATRICES = 32;
float4x3    WorldMatrixArray[MAX_MATRICES] : WORLDMATRIXARRAY;


struct VS_INPUT
{
    float4 Pos             : POSITION;
    float3 Normal          : NORMAL;
    float4 Color           : COLOR0;
    float3 Tex0            : TEXCOORD0;
};
struct VS_INPUT_BONE
{
    float4 Pos             : POSITION;
    float3 BlendWeights    : BLENDWEIGHT;
    float4 BlendIndices    : BLENDINDICES;
    float3 Normal          : NORMAL;
    float4 Color           : COLOR0;
    float3 Tex0            : TEXCOORD0;
};
struct VS_OUTPUT
{
    float4 Pos             : POSITION;
    float4 Color           : COLOR0;
    float3 Tex0            : TEXCOORD0;
};
struct VSS_OUTPUT
{
    float4 Pos             : POSITION;
};

VS_OUTPUT VS_NL(VS_INPUT i)
{
	VS_OUTPUT o;
	
	o.Pos = mul(i.Pos,WorldViewProj);

    o.Color =  i.Color;
	o.Tex0 = i.Tex0;

	return o;
}

VS_OUTPUT VS(VS_INPUT i)
{
	float3 Light = -LightDir;
 	float3 Normal = normalize(mul(i.Normal.xyz,(float3x3)World));
   
	VS_OUTPUT o;
	
	o.Pos = mul(i.Pos,WorldViewProj);

    o.Color.xyz =  MaterialAmbient.xyz + max(0.0f,dot(Normal,Light)) * i.Color.xyz;
	o.Color.w = i.Color.w;
	o.Tex0 = i.Tex0;

	return o;
}
VS_OUTPUT VS4(VS_INPUT_BONE i)
{
    float3 Light = -LightDir;
    
	VS_OUTPUT o = (VS_OUTPUT)0;
    float3      Pos = 0.0f;
    float3      Normal = 0.0f;    
    float       LastWeight = 1.0f;
     
    float BlendWeightsArray[3] = (float[3])i.BlendWeights;
    int4 IndexVector = D3DCOLORtoUBYTE4(i.BlendIndices);

     LastWeight = LastWeight - BlendWeightsArray[0];
     Pos += mul(i.Pos, WorldMatrixArray[IndexVector[0]]) * BlendWeightsArray[0];
     Normal += mul(i.Normal, WorldMatrixArray[IndexVector[0]]) * BlendWeightsArray[0];

     LastWeight = LastWeight - BlendWeightsArray[1];
     Pos += mul(i.Pos, WorldMatrixArray[IndexVector[1]]) * BlendWeightsArray[1];
     Normal += mul(i.Normal, WorldMatrixArray[IndexVector[1]]) * BlendWeightsArray[1];

     LastWeight = LastWeight - BlendWeightsArray[2];
     Pos += mul(i.Pos, WorldMatrixArray[IndexVector[2]]) * BlendWeightsArray[2];
     Normal += mul(i.Normal, WorldMatrixArray[IndexVector[2]]) * BlendWeightsArray[2];

    Pos += mul(i.Pos, WorldMatrixArray[IndexVector[3]]) * LastWeight;
    Normal +=mul(i.Normal, (float3x3)WorldMatrixArray[IndexVector[3]]) * LastWeight; 
	
	
	o.Pos = float4(Pos,1.0f);
	o.Pos = mul(o.Pos,ViewProj);
	
    o.Color.xyz = MaterialAmbient.xyz + max(0.0f,dot(Normal,Light)) * i.Color.xyz;
	o.Color.w = i.Color.w;
	o.Tex0 = i.Tex0;

	return o;
}



VSS_OUTPUT VSS(VS_INPUT i)
{
 	float3 Normal = normalize(mul(i.Normal.xyz,(float3x3)World));
	VSS_OUTPUT o;
	
	o.Pos = mul(i.Pos,World);
	float LN = dot(Normal,LightDir);
	float scale = (LN<0.0f)?ShadowLong[0]:ShadowLong[1];
	o.Pos.xyz = o.Pos.xyz + normalize(LightDir)*scale;

	o.Pos = mul(o.Pos,ViewProj);
	
  	return o;
}
VSS_OUTPUT VSS4(VS_INPUT_BONE i)
{
    float3 Light = normalize(-LightDir);
    
	VSS_OUTPUT o;
    float3      Pos = 0.0f;
    float3      Normal = 0.0f;    
    float       LastWeight = 1.0f;
     
    float BlendWeightsArray[3] = (float[3])i.BlendWeights;
    int4 IndexVector = D3DCOLORtoUBYTE4(i.BlendIndices);

     LastWeight = LastWeight - BlendWeightsArray[0];
     Pos += mul(i.Pos, WorldMatrixArray[IndexVector[0]]) * BlendWeightsArray[0];
     Normal += mul(i.Normal, WorldMatrixArray[IndexVector[0]]) * BlendWeightsArray[0];

     LastWeight = LastWeight - BlendWeightsArray[1];
     Pos += mul(i.Pos, WorldMatrixArray[IndexVector[1]]) * BlendWeightsArray[1];
     Normal += mul(i.Normal, WorldMatrixArray[IndexVector[1]]) * BlendWeightsArray[1];

     LastWeight = LastWeight - BlendWeightsArray[2];
     Pos += mul(i.Pos, WorldMatrixArray[IndexVector[2]]) * BlendWeightsArray[2];
     Normal += mul(i.Normal, WorldMatrixArray[IndexVector[2]]) * BlendWeightsArray[2];

    Pos += mul(i.Pos, WorldMatrixArray[IndexVector[3]]) * LastWeight;
    Normal +=mul(i.Normal, (float3x3)WorldMatrixArray[IndexVector[3]]) * LastWeight; 
	
	
	o.Pos = float4(Pos,1.0f);

	float LN = dot(Normal,LightDir);
	float scale = (LN<0)?ShadowLong[0]:ShadowLong[1];
	o.Pos.xyz = o.Pos.xyz + normalize(LightDir)*scale;
	
	o.Pos = mul(o.Pos,ViewProj);

	return o;
}



technique t0
{
    pass P0
    {
        VertexShader = compile vs_1_1 VS();
    }
}
technique t4
{
    pass P0
    {
        VertexShader = compile vs_1_1 VS4();
    }
}

technique shadow0
{
    pass P0
    {
        VertexShader = compile vs_1_1 VSS();
    }
}
technique shadow4
{
    pass P0
    {
        VertexShader = compile vs_1_1 VSS4();
    }
}

