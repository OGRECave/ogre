uniform sampler2D RT;
uniform float NumTiles;
uniform float Threshold;
varying vec2 oUv0;

void main()
{
	vec3 EdgeColor = vec3(0.7, 0.7, 0.7);

    float size = 1.0/NumTiles;
    vec2 Pbase = oUv0 - mod(oUv0, vec2(size));
    vec2 PCenter = vec2(Pbase + (size/2.0));
    vec2 st = (oUv0 - Pbase)/size;
    vec4 c1 = vec4(0.0);
    vec4 c2 = vec4(0.0);
    vec4 invOff = vec4((1.0-EdgeColor),1.0);
    if (st.x > st.y) { c1 = invOff; }
    float threshholdB =  1.0 - Threshold;
    if (st.x > threshholdB) { c2 = c1; }
    if (st.y > threshholdB) { c2 = c1; }
    vec4 cBottom = c2;
    c1 = vec4(0.0);
    c2 = vec4(0.0);
    if (st.x > st.y) { c1 = invOff; }
    if (st.x < Threshold) { c2 = c1; }
    if (st.y < Threshold) { c2 = c1; }
    vec4 cTop = c2;
    vec4 tileColor = vec4(texture2D(RT, PCenter));
    vec4 result = tileColor + cTop - cBottom;
    gl_FragColor = result;
}
