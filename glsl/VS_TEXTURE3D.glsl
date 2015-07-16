#include "default.glsli"

uniform mat4 View;
uniform mat4 Projection;
uniform mat4 ViewProj;
uniform mat4 World;
uniform vec4 Scale;

attribute vec4 POSITION;
attribute vec2 TEXCOORD;
varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
	//中心位置の確定
	gl_Position = vec4(0, 0, 0, 1)*World;
	gl_Position = computeSphere(gl_Position,View);

	gl_Position.xyz += vec3(POSITION.x, -POSITION.y, POSITION.z);
	gl_Position = gl_Position*Projection;
	v_color = vec4(1.0);
	v_texCoord = TEXCOORD*Scale.xy + Scale.zw;

	//離れたらフェード
	float PDF = 1500.0;
	if (gl_Position.z > PDF)
		v_color.w *= (1.0 - (gl_Position.z - PDF) / PDF * 0.5);

}
