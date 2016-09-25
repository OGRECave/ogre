#version 430

out vec4 fragColour;

layout(std430, binding = 1) buffer PixelBuffer
{
    readonly uint data[];
} pixelBuffer;

in vec4 gl_FragCoord;

uniform uvec2 texResolution;

void main()
{
    uint idx = uint(gl_FragCoord.y) * texResolution.x + uint(gl_FragCoord.x);
    fragColour = unpackUnorm4x8( pixelBuffer.data[idx] );
}
