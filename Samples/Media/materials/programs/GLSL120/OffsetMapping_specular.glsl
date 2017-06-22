#version 120

uniform vec4 lightDiffuse;
uniform vec4 lightSpecular;
uniform vec4 scaleBias;
uniform float exponent;
uniform sampler2D normalHeightMap;

varying vec3 tangentEyeDir;
varying vec3 tangentLightDir[2];
varying vec4 shadowUV[2];
varying vec4 oUv0;

vec4 lit(float NdotL, float NdotH, float m) {

  float ambient = 1.0;
  float diffuse = max(NdotL, 0.0);
  float specular = step(0.0, NdotL) * max(NdotH * m, 0.0);

  return vec4(ambient, diffuse, specular, 1.0);
}

vec3 expand(vec3 v)
{
	return (v - 0.5) * 2.0;
}

/*
  Pixel Shader for doing bump mapping with parallax plus diffuse and specular lighting by masterfalcon
*/
void main()
{
	float height = texture2D(normalHeightMap, oUv0.xy).a;
	float displacement = (height * scaleBias.x) + scaleBias.y;
	vec2 newTexCoord = ((tangentEyeDir * displacement) + oUv0.xyz).xy;
	vec3 bumpVec = expand(texture2D(normalHeightMap, newTexCoord).xyz);
	vec3 N = normalize(bumpVec);

	vec3 halfAngle = normalize(tangentEyeDir + tangentLightDir[0]); 
	float NdotL = dot(normalize(tangentLightDir[0]), N);
	float NdotH = dot(normalize(halfAngle), N); 

	vec4 Lit = lit(NdotL, NdotH, exponent);
	
	gl_FragColor = lightDiffuse * Lit.y + lightSpecular * Lit.z;
}
