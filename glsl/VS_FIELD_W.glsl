precision highp float;

const vec3 LightDir = normalize(vec3(0.1,-0.5,-0.5));


uniform mat4 View;
uniform mat4 Projection;
uniform mat4 ViewProj;
uniform mat4 World;
uniform vec4 Noize;

attribute vec3 POSITION;
attribute vec3 NORMAL;
attribute vec3 TANGENT;
attribute vec3 BINORMAL;
attribute vec2 TEXCOORD;
attribute vec2 TEXCOORD_;

varying vec2 v_texCoord[2];
varying vec3 v_light;
varying float v_Distance;

mat3 InvTangentMatrix(vec3 tangent,vec3 binormal,vec3 normal )
{
	return mat3(
		tangent.x ,binormal.x,normal.x,
		tangent.y ,binormal.y,normal.y,
		tangent.z ,binormal.z,normal.z
	);
}


void main()
{
	gl_Position = vec4(POSITION.xyz,1) * World;

	vec4 sPos;
	sPos = gl_Position * View;
	sPos *= sPos;
	v_Distance = (sPos.x+sPos.y+sPos.z);
	gl_Position.z -= (sPos.x + sPos.y + sPos.z)/100000.0;
	gl_Position = gl_Position * ViewProj;

	v_texCoord[0] = TEXCOORD;
	v_texCoord[1] = TEXCOORD_;

	v_light = -LightDir * InvTangentMatrix( TANGENT, BINORMAL, NORMAL );
}

