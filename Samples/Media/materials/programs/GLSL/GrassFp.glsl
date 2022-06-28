////////////////////////////// MOVING GRASS
// Vertex program to wave some grass about
// Assumes UV texture coords of v==0 indicates the top of the grass
uniform sampler2D diffuseMap;
uniform float alphaThresh;

varying vec4 oUv0;
varying vec4 oColour;

void main()
{	
    vec4 texColor = texture2D(diffuseMap, oUv0.xy);

    // Do manual alpha rejection because it is not built into OpenGL ES 2
    if (texColor.a < alphaThresh)
    {
        discard;
    }

    gl_FragColor = vec4(texColor.rgb * oColour.rgb, 1.0);
}
