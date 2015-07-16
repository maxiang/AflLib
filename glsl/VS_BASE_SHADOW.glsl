#include "default.glsli"

const vec2 ShadowLong = vec2(50.0,1000.0);

uniform mat4 View;
uniform mat4 Projection;
uniform mat4 ViewProj;
uniform mat4 World;

attribute vec4 POSITION;
attribute vec3 NORMAL;
attribute vec4 COLOR;
attribute vec2 TEXCOORD;

varying vec2 v_texCoord;
varying vec4 v_color;


void main()
{
	gl_Position = POSITION*World;
	vec3 s_nor = normalize(NORMAL*mat3(World));
	float LN = dot(s_nor,LightDir);
	float scale = (LN<0.0)?ShadowLong.x:ShadowLong.y;
	gl_Position.xyz += LightDir*scale;
	//gl_Position =  gl_Position*ViewProj;
	gl_Position = computeSphere(gl_Position,View);
	gl_Position = gl_Position*Projection;
	
	v_texCoord = TEXCOORD;
	v_color = COLOR;
	
//	if(gl_Position.z > PDF)
//		v_color.w = v_color.w * (1.0-(gl_Position.z-PDF)/PDF2);
	
}
