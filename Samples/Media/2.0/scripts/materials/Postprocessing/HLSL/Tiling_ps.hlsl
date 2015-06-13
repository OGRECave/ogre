Texture2D RT				: register(t0);
SamplerState samplerState	: register(s0);

float4 main
(
	float2 uv0 : TEXCOORD0,
	uniform half NumTiles,
	uniform half Threshold
) : SV_Target
{
	half3 EdgeColor = {0.7, 0.7, 0.7};

    half size = 1.0/NumTiles;
	half2 Pbase = uv0 - fmod(uv0, size.xx);
    half2 PCenter = Pbase + (size/2.0).xx;
	half2 st = (uv0 - Pbase)/size;
    half4 c1 = (half4)0;
    half4 c2 = (half4)0;
    half4 invOff = half4((1-EdgeColor),1);
    if (st.x > st.y) { c1 = invOff; }
    half threshholdB =  1.0 - Threshold;
    if (st.x > threshholdB) { c2 = c1; }
    if (st.y > threshholdB) { c2 = c1; }
    half4 cBottom = c2;
    c1 = (half4)0;
    c2 = (half4)0;
    if (st.x > st.y) { c1 = invOff; }
    if (st.x < Threshold) { c2 = c1; }
    if (st.y < Threshold) { c2 = c1; }
    half4 cTop = c2;
    half4 tileColor = RT.Sample( samplerState, PCenter );
    half4 result = tileColor + cTop - cBottom;
    return result;
}
