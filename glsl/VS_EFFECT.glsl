#include "default.glsli"


uniform mat4 View;
uniform mat4 Projection;
uniform mat4 ViewProj;
uniform mat4 World;

attribute vec4 POSITION;
attribute float SIZE;
attribute float ROTATION;
attribute vec4 COLOR;
attribute vec2 TEXCOORD;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
	//疑似球体化
	gl_Position = POSITION*World;
	gl_Position = computeSphere(gl_Position,View);

	float sin0 = sin(ROTATION);
	float cos0 = cos(ROTATION);

	vec2 ss = (TEXCOORD.xy - 0.5)*SIZE;
	gl_Position.xy += vec2(ss.x*cos0-ss.y*sin0,-(ss.x*sin0+ss.y*cos0));
	gl_Position *= Projection;

	v_texCoord = TEXCOORD;
	v_color = COLOR*4.0;
}


