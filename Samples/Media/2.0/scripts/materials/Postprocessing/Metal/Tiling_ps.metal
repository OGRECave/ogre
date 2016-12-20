#include <metal_stdlib>
using namespace metal;

struct PS_INPUT
{
	float2 uv0;
};

struct Params
{
	float NumTiles;
	float Threshold;
};

fragment half4 main_metal
(
	PS_INPUT inPs [[stage_in]],
	texture2d<half>		RT				[[texture(0)]],
	sampler				samplerState	[[sampler(0)]],

	constant Params &p [[buffer(PARAMETER_SLOT)]]
)
{
	half3 EdgeColor = {0.7h, 0.7h, 0.7h};

	half2 uv0 = (half2)inPs.uv0;

	half size = 1.0h/(half)p.NumTiles;
	half2 Pbase = uv0 - fmod(uv0, size);
	half2 PCenter = Pbase + (size/2.0h);
	half2 st = (uv0 - Pbase)/size;
    half4 c1 = (half4)0;
    half4 c2 = (half4)0;
    half4 invOff = half4((1-EdgeColor),1);
    if (st.x > st.y) { c1 = invOff; }
	half threshholdB =  1.0h - p.Threshold;
    if (st.x > threshholdB) { c2 = c1; }
	if (st.y > threshholdB) { c2 = c1; }
    half4 cBottom = c2;
    c1 = (half4)0;
    c2 = (half4)0;
    if (st.x > st.y) { c1 = invOff; }
	if (st.x < p.Threshold) { c2 = c1; }
	if (st.y < p.Threshold) { c2 = c1; }
    half4 cTop = c2;
	half4 tileColor = RT.sample( samplerState, (float2)PCenter );
    half4 result = tileColor + cTop - cBottom;
    return result;
}
