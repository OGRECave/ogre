// a very simple 4x4 box filter
// the kernel has the following form
//   o o o o
//   o o o o
//   o o x o
//   o o o o 
// where x marks the fragment position and the o marks a sampling point
#version 120

varying vec2 uv;

uniform sampler2D sOcclusion;
uniform vec4 screenSize;
uniform float farClipDistance;
	
void main()
{
    float color = 0;
    for (int x = -2; x < 2; x++)
    for (int y = -2; y < 2; y++)
    {
        color += texture2D(sOcclusion, vec2(uv.x + x * screenSize.z, uv.y + y * screenSize.w)).x;
    }
    color /= 16;
        
    gl_FragColor = vec4(color, color, color, 1);
}