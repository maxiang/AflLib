#include "default.glsli"


uniform float Light;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 ViewProj;
uniform mat4 World;

attribute vec4 POSITION;
attribute vec3 NORMAL;
attribute vec3 TANGENT;
attribute vec3 BINORMAL;
attribute vec2 TEXCOORD;

varying vec2 v_texPoint;
varying vec2 v_texCoord[2];
varying vec3 v_light;
varying vec4 v_color;



void main()
{
	gl_Position = POSITION*World;
	gl_Position = computeSphere(gl_Position,View);
	gl_Position = gl_Position*Projection;

	v_texCoord[0] = TEXCOORD;
	v_texCoord[1] = TEXCOORD;


	if(Light != 0.0)
	{
		if(gl_Position.z < PDN)
			v_color.w = (1.0-(PDN-gl_Position.z)/PDN*0.8);
		else if(gl_Position.z > PDF)
			v_color.w = (1.0-(gl_Position.z-PDF)/PDF*2.0);
		else
			v_color.w = 1.0;
	}
	mat3 nw = mat3(World);
	vec3 Nor = normalize(NORMAL*nw);
	v_light = -LightDir * InvTangentMatrix( TANGENT, BINORMAL, Nor );
	
}
