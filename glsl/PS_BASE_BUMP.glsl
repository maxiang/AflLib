#include "default.glsli"

precision mediump float;

uniform sampler2D texture0;
uniform sampler2D texture1;

varying vec2 v_texPoint;
varying vec2 v_texCoord[2];
varying vec3 v_light;

void main()
{
	vec4 color = texture2D(texture0,v_texCoord[0]);
	vec3 color2 = vec3(texture2D(texture1,v_texCoord[1]));

	vec3 vect = color2 * 2.0 - 1.0;
	vect = normalize(vect);

	float bright = 1.0-dot(v_light,vect);
	bright = 1.0 - bright*bright;
	bright = bright*LightPri+(1.0-LightPri);
	gl_FragColor = color * vec4(vec3(bright),0.99);
}
