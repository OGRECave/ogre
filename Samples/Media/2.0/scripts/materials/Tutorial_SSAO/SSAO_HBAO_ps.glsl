#version 330

uniform sampler2D depthTexture;
uniform sampler2D noiseTexture;
uniform sampler2D sampleTexture;

in block
{
   vec2 uv0;
   vec3 cameraDir;
} inPs;

uniform vec2 projectionParams;

// defines for SSAO
uniform int kernelSize;
uniform float kernelRadius;
uniform vec2 noiseScale;
uniform mat4 projection;

out float fragColour;

vec3 reconstructNormal(vec3 posInView)
{
	vec3 dNorm = cross(normalize(dFdy(posInView)), normalize(dFdx(posInView)));
	return dNorm;
}

void main()
{
    float fDepth = texture( depthTexture, inPs.uv0).x;
	float linearDepth = projectionParams.y / (fDepth - projectionParams.x);

    vec3 fragPos = inPs.cameraDir * linearDepth;
    vec3 normal = reconstructNormal(fragPos);
   
    vec3 randomVec = texture(noiseTexture, inPs.uv0 * noiseScale).xyz;
    randomVec.xy = randomVec.xy * 2.0 - 1.0;
   
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);
   
    float occlusion = 0.0;
    for(int i = 0; i < 8; ++i)
    {
		for(int a = 0; a < 8; ++a)
		{
			vec3 sNoise = texture(sampleTexture, vec2((1.0/8.0)*(float(i)+0.5), (1.0/8.0)*(float(a)+0.5))).xyz;
			sNoise.xy = sNoise.xy * 2.0 - 1.0;
			sNoise = normalize(sNoise);
         
			// get sample position
			vec3 sample = TBN * sNoise; // From tangent to view-space
			sample = fragPos + sample * kernelRadius; 
        
			// project sample position (to sample texture) (to get position on screen/texture)
			vec4 offset = vec4(sample, 1.0);
			offset = projection * offset; // from view to clip-space
			offset.xyz /= offset.w; // perspective divide
			offset.xyz = offset.xyz * 0.5 + 0.5; // transform to range 0.0 - 1.0
			
			offset.y = 1.0 - offset.y;

			// get sample depth
			float fSampleDepth = texture(depthTexture, offset.xy).x; // Get depth value of sample
			float newLinearDepth = projectionParams.y / (fSampleDepth - projectionParams.x);
			vec3 sampleFragPos = inPs.cameraDir * newLinearDepth;
			float sampleDepth = sampleFragPos.z;
         
			// range check & accumulate
			float rangeCheck = smoothstep(0.0, 1.0, kernelRadius / abs(fragPos.z - sampleDepth));
			occlusion += (sampleDepth >= sample.z ? 1.0 : 0.0) * rangeCheck; 
		}      
    }
    occlusion = 1.0 - (occlusion / kernelSize);
   
    fragColour = occlusion;
}