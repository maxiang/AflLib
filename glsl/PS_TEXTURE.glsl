uniform sampler2D texture0;
precision mediump float;
varying vec2 v_texCoord;
varying vec4 v_color;
varying vec4 v_light;
void main()
{
	gl_FragColor = texture2D(texture0,v_texCoord);
}
