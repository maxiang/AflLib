precision highp float;

//precision mediump int;

float LightPri = 0.5;

uniform sampler2D texture0;
uniform sampler2D texture1;

varying vec2 v_texCoord[2];
varying vec3 v_light;
varying float v_Distance;

void main()
{
	float a = 0.1+v_Distance/7000000.0;
	if(a > 1.0)
		a = 1.0;


	vec4 color = texture2D(texture0,v_texCoord[0]);
	vec3 color2 = vec3(texture2D(texture1,v_texCoord[1]));

	vec3 vect = color2 * 2.0 - 1.0;
	vect = normalize(vect);

	float bright = dot(v_light,vect);
	bright = bright*LightPri+(1.0-LightPri);
	gl_FragColor = color * vec4(vec3(bright),a);
}
