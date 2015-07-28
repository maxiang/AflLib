#include "default.glsli"

uniform float Light;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 ViewProj;
uniform mat4 World;

attribute vec4 POSITION;
attribute vec3 NORMAL;
attribute vec4 COLOR;
attribute vec2 TEXCOORD;

varying vec2 v_texCoord;
varying vec4 v_color;
varying vec4 v_light;

void main()
{
	gl_Position = POSITION*World;
	gl_Position = computeSphere(gl_Position,View);
	gl_Position *= Projection;
	v_texCoord = TEXCOORD;


	vec3 Nor = normalize(NORMAL*mat3(World));
	float light = dot(Nor,normalize(-LightDir));

	light = light*LightPri+(1.0-LightPri);
	light = max(light,0.2); 
	v_light = vec4(light);
	v_light.w = 1.0;
	v_color.xyz = COLOR.xyz*light;
	v_color.w = COLOR.w;
	
	if(gl_Position.z < PDN)
		v_color.w *= (1.0-(PDN-gl_Position.z)/PDN*0.7);
	else if(gl_Position.z > PDF)
		v_color.w *= (1.0-(gl_Position.z-PDF)/PDF*2.0);
		

}
