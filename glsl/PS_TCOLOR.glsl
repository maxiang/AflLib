uniform sampler2D texture0;
precision mediump float;
varying vec2 v_texCoord;
varying vec4 v_color;
void main()
{
	gl_FragColor = texture2D(texture0,v_texCoord) * v_color;
//	if( gl_FragColor.a < 0.01 )
//		discard;
}
