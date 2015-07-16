precision mediump float;
varying vec4 v_color;
void main()
{
	gl_FragColor = v_color;
	if( v_color.a < 0.1 )
		discard;
}
