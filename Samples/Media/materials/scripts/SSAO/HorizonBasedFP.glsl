#version 120
varying vec2 oUv0;

uniform sampler2D sMRT1;
uniform sampler2D sMRT2;
uniform sampler2D sRand;
	
uniform vec4 cViewportSize; // (viewport_width, viewport_height, inverse_viewport_width, inverse_viewport_height)
uniform float cFov; // vertical field of view in radians
uniform float cSampleInScreenspace; // whether to sample in screen or world space
uniform float cSampleLengthScreenSpace; // The sample length in screen space [0, 1]
uniform float cSampleLengthWorldSpace; // the sample length in world space in units
uniform float cAngleBias; // angle bias to avoid shadows in low tessellated curvatures [0, pi/2]

void main()
{
    const float pi = 3.1415926535897932384626433832795028841971693993751;

    const float numSteps = 7; // number of samples/steps along a direction
    const float numDirections = 4; // number of sampling directions in oUv0 space
    
    vec3 p = texture2D(sMRT2, oUv0).xyz; // the current fragment in view space
    vec3 pointNormal = texture2D(sMRT1, oUv0).xyz; // the fragment normal

    float Ruv = 0; // radius of influence in screen space
    float R = 0; // radius of influence in world space
    if (cSampleInScreenspace == 1)
    {
        Ruv = cSampleLengthScreenSpace;
        R = tan(Ruv * cFov) * -p.z;
    }
    else
    {
        Ruv = atan(cSampleLengthWorldSpace / -p.z) / cFov; // the radius of influence projected into screen space
        R = cSampleLengthWorldSpace;
    }

    // if the radius of influence is smaller than one fragment we exit early,
    // since all samples would hit the current fragment.
    if (Ruv < (1 / cViewportSize.x))
    {
        gl_FragColor = vec4(1.0, 1.0, 1.0, 1.0);
        return;
    }

    float occlusion = 0; // occlusion of the fragment
    
	// the matrix to create the sample directions
    // the compiler should evaluate the directions at compile time
	mat2 directionMatrix = mat2( cos( (2 * pi) / numDirections ), -sin( (2 * pi) / numDirections ),
								 sin( (2 * pi) / numDirections ),  cos( (2 * pi) / numDirections ));
						 
    vec2 deltaUV = vec2(1, 0) * (Ruv / (numSteps + 1)); // The step vector in view space. scale it to the step size
           
    // we don't want to sample to the perimeter of R since those samples would be 
    // omitted by the distance attenuation (W(R) = 0 by definition)
    // Therefore we add a extra step and don't use the last sample.
    vec3 randomValues = texture2D(sRand, (oUv0 * cViewportSize.xy) / 4).xyz; //4px tiles
	mat2 rotationMatrix = mat2( (randomValues.x - 0.5) * 2, -(randomValues.y - 0.5) * 2,
								(randomValues.y - 0.5) * 2,  (randomValues.x - 0.5) * 2);

    float jitter = randomValues.z;

    for (int i = 0; i < numDirections; i++)
    {
        deltaUV = directionMatrix * deltaUV; // rotate the deltaUV vector by 1/numDirections
        vec2 sampleDirection = rotationMatrix * deltaUV; // now rotate this vector with the random rotation

        float oldAngle = cAngleBias;

        for (int j = 1; j <= numSteps; j++) // sample along a direction, needs to start at one, for the sake of the next line
        {
            vec2 sampleUV = oUv0 + ((jitter + j) * sampleDirection); // jitter the step a little bit
            
            vec3 sample = texture2D(sMRT2, sampleUV).xyz; // the sample in view space
            vec3 sampleVector = (sample - p);
            float gamma = (pi / 2) - acos(dot(pointNormal, normalize(sampleVector))); //the angle between the fragment tangent and the sample

            if (gamma > oldAngle) 
            {
                float attenuation = clamp(1 - (pow((length(sampleVector) / R), 2)), 0.0, 1.0);
                occlusion += attenuation * (sin(gamma) - sin(oldAngle));
                oldAngle = gamma;
            }
        }
    }

    // ??? should step samples that fall under the horizontal be considered in the following line??? 
    occlusion /= (numDirections * numSteps);
    gl_FragColor = 1 - vec4(occlusion, occlusion, occlusion, 1) * 2 * pi;
}
