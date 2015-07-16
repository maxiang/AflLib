attribute vec3 POSITION;
attribute vec2 TEXCOORD;
varying vec2 v_texCoord;

void main()
{
	gl_Position = vec4(POSITION,1);
	v_texCoord = TEXCOORD;
}
