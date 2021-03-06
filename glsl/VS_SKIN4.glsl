#include "default.glsli"

const int MAX_MATRICES = 24;


uniform mat4 View;
uniform mat4 Projection;
uniform mat4 ViewProj;
uniform mat4 World;
uniform mat4 WorldMatrixArray[MAX_MATRICES];


attribute vec4 POSITION;
attribute vec3 NORMAL;
attribute vec4 COLOR;
attribute vec2 TEXCOORD;

attribute float BLENDWEIGHT0;
attribute float BLENDWEIGHT1;
attribute float BLENDWEIGHT2;

attribute float BLENDINDICES0;
attribute float BLENDINDICES1;
attribute float BLENDINDICES2;
attribute float BLENDINDICES3;

varying vec2 v_texCoord;
varying vec4 v_color;


void main()
{
	int index;
	vec4 Pos;
	vec3 Nor;
	mat4 matrix;


	matrix = WorldMatrixArray[int(BLENDINDICES0)];
	Pos = POSITION * matrix * BLENDWEIGHT0;
	Nor  = NORMAL * mat3(matrix) * BLENDWEIGHT0;
	
	float LastWeight = 1.0 - BLENDWEIGHT0;
	index = int(BLENDINDICES1);
	if(index < 24)
	{
		matrix = WorldMatrixArray[index];
		Pos += (POSITION * matrix) * BLENDWEIGHT1;
		Nor += NORMAL * mat3(matrix) * BLENDWEIGHT1;
		LastWeight -= BLENDWEIGHT1;
	}
	index = int(BLENDINDICES2);
	if(index < 24)
	{
		matrix = WorldMatrixArray[index];
		Pos += (POSITION * matrix) * BLENDWEIGHT2;
		Nor += NORMAL * mat3(matrix) * BLENDWEIGHT2;
		LastWeight -= BLENDWEIGHT2;
	}
	gl_Position = computeSphere(Pos,View);
	gl_Position = gl_Position*Projection;

	v_texCoord = TEXCOORD;
	v_color.w = COLOR.w;
	v_color.xyz = max(vec3(0.1),dot(Nor,-LightDir)*vec3(COLOR));
	if(gl_Position.z < PDN)
		v_color.w *= (1.0-(PDN-gl_Position.z)/PDN*0.7);
	else if(gl_Position.z > PDF)
		v_color.w *= (1.0-(gl_Position.z-PDF)/PDF*2.0);
}
