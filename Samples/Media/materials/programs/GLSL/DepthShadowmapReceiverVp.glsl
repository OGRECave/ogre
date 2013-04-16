uniform mat4 world;
uniform mat4 worldIT;
uniform mat4 worldViewProj;
uniform mat4 texViewProj;
uniform vec4 lightPosition;
uniform vec4 lightColour;
uniform vec4 shadowDepthRange;


void main()
{
	gl_Position = ftransform();
	
	vec4 worldPos = world * gl_Vertex;

	vec3 worldNorm = (worldIT * vec4(gl_Normal, 1)).xyz;

	// calculate lighting (simple vertex lighting)
	vec3 lightDir = normalize(
		lightPosition.xyz -  (worldPos.xyz * lightPosition.w));

	gl_FrontColor = lightColour * max(dot(lightDir, worldNorm), 0.0);

	// calculate shadow map coords
	gl_TexCoord[0] = texViewProj * worldPos;
#if LINEAR_RANGE
	// adjust by fixed depth bias, rescale into range
	gl_TexCoord[0].z = (gl_TexCoord[0].z - shadowDepthRange.x) * shadowDepthRange.w;
#endif

}

