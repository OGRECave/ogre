#version 330

uniform sampler2D depthTexture;
uniform sampler2D noiseTexture;

in block
{
   vec2 uv0;
   vec3 cameraDir;
} inPs;


uniform vec2 projectionParams;
uniform float invKernelSize;
uniform float kernelRadius;
uniform vec2 noiseScale;
uniform mat4 projection;

uniform vec4 sampleDirs[64];

out float fragColour;

vec3 getScreenSpacePos(vec2 uv, vec3 cameraNormal)
{
	float fDepth = texture( depthTexture, uv).x;
	float linearDepth = projectionParams.y / (fDepth - projectionParams.x);
	return (cameraNormal * linearDepth);
}

vec3 reconstructNormal(vec3 posInView)
{
	vec3 dNorm = cross(normalize(dFdy(posInView)), normalize(dFdx(posInView)));
	return dNorm;
}

vec3 getRandomVec(vec2 uv)
{
	vec3 randomVec = texture(noiseTexture, uv * noiseScale).xyz;
	return randomVec;
}

void main()
{
    vec3 viewPosition = getScreenSpacePos(inPs.uv0, inPs.cameraDir);
    vec3 viewNormal = reconstructNormal(viewPosition);
    vec3 randomVec = getRandomVec(inPs.uv0);
   
    vec3 tangent = normalize(randomVec - viewNormal * dot(randomVec, viewNormal));
    vec3 bitangent = cross(viewNormal, tangent);
    mat3 TBN = mat3(tangent, bitangent, viewNormal);
   
    float occlusion = 0.0;
    for(int i = 0; i < 8; ++i)
    {
		for(int a = 0; a < 8; ++a)
		{
			vec3 sNoise = sampleDirs[(a << 2u) + i].xyz;
         
			// get sample position
			vec3 sample = TBN * sNoise; // From tangent to view-space
			sample = viewPosition + sample * kernelRadius; 
        
			// project sample position
			vec4 offset = vec4(sample, 1.0);
			offset = projection * offset; // from view to clip-space
			offset.xyz /= offset.w; // perspective divide
			offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
			offset.y = 1.0 - offset.y;

			float sampleDepth = getScreenSpacePos(offset.xy, inPs.cameraDir).z;

			float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(viewPosition.z - sampleDepth));
			occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck;
			
		}      
    }
    occlusion = 1.0 - (occlusion * invKernelSize);
   
    fragColour = occlusion;
}
