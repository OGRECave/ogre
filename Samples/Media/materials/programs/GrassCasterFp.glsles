#version 100

precision mediump int;
precision mediump float;

uniform sampler2D diffuseMap;

varying vec4 oUv0;
varying vec2 oDepth;

//////////////////////// GRASS SHADOW CASTER
void main()
{	
	float alpha = texture2D(diffuseMap, oUv0.xy).a;

    // Do manual alpha rejection because it is not built into OpenGL ES 2
    if (alpha < 0.588)
    {
        discard;
    }

	if (alpha > 0.001)
    {
       gl_FragColor = vec4(1.0, 1.0, 1.0, 0.0);
    }
    else
    {
        float finalDepth = oDepth.x / oDepth.y;
        // just smear across all components 
        // therefore this one needs high individual channel precision
        gl_FragColor = vec4(vec3(finalDepth), 1.0);
    }
}
