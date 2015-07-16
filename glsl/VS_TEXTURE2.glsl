uniform vec4 Scale;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 ViewProj;
uniform mat4 World;

attribute vec4 POSITION;
attribute vec4 COLOR;
attribute vec2 TEXCOORD;

varying vec2 v_texCoord;
varying vec4 v_color;


void main()
{
	gl_Position = POSITION;
	gl_Position.xy *= Scale.xy;
	gl_Position *= World;
	gl_Position = gl_Position*ViewProj;
	v_texCoord = TEXCOORD + Scale.zw;
	v_color = COLOR;
}
