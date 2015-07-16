#include "default.glsli"


uniform mat4 View;
uniform mat4 Projection;
uniform mat4 ViewProj;
uniform mat4 World;

attribute vec4 POSITION;
attribute vec3 NORMAL;
attribute vec4 COLOR;

varying vec2 v_texCoord;
varying vec4 v_color;

void main()
{
	//íÜêSà íuÇÃämíË
	gl_Position = vec4(0, 0, 0, 1)*World;
	gl_Position = gl_Position * View;
	gl_Position.xyz += vec3(POSITION.x, -POSITION.y, POSITION.z);
	gl_Position = gl_Position*Projection;
	v_color = COLOR;

}
