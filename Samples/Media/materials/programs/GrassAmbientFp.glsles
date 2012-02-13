#version 100

precision mediump int;
precision mediump float;

uniform sampler2D diffuseMap;

varying vec4 oUv0;
varying vec4 oColour;

/// grass_vp ambient
void main()
{	
    vec4 texColor = texture2D(diffuseMap, oUv0.xy);
    // Do manual alpha rejection because it is not built into OpenGL ES 2
    if (texColor.a < 0.588)
    {
        discard;
    }

    gl_FragColor = vec4(oColour.rgb, texColor.a);
}
