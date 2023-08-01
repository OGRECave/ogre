// Real-Time Polygonal-Light Shading with Linearly Transformed Cosines
// by Eric Heitz, Jonathan Dupuy, Stephen Hill and David Neubelt
// code: https://github.com/selfshadow/ltc_code/
// also: https://github.com/mrdoob/three.js/blob/master/src/renderers/shaders/ShaderChunk/lights_physical_pars_fragment.glsl.js
// adapted for Ogre by Pavel Rojtberg

#define LUT_SIZE  64.0
#define LUT_SCALE ((LUT_SIZE - 1.0)/LUT_SIZE)
#define LUT_BIAS  (0.5/LUT_SIZE)

vec3 IntegrateEdgeVec(vec3 v1, vec3 v2)
{
    float x = dot(v1, v2);
    float y = abs(x);

    float a = 0.8543985 + (0.4965155 + 0.0145206*y)*y;
    float b = 3.4175940 + (4.1616724 + y)*y;
    float v = a / b;

    float theta_sintheta = (x > 0.0) ? v : 0.5*inversesqrt(max(1.0 - x*x, 1e-7)) - v;

    return cross(v1, v2)*theta_sintheta;
}

float LTC_Evaluate(vec3 N, vec3 V, vec3 P, mat3 Minv, vec3 points[4], sampler2D ltc_2)
{
    vec3 dir = points[0] - P;
    vec3 lightDir = cross(points[1] - points[0], points[3] - points[0]);
    if(dot(dir, lightDir) < 0.0)
        return 0.0;

    // construct orthonormal basis around N
    vec3 T1, T2;
    T1 = normalize(V - N*dot(V, N));
    T2 = cross(N, T1);

    // rotate area light in (T1, T2, N) basis
    Minv = mul(Minv, mtxFromRows(T1, T2, N));

    // polygon
    vec3 L[4];
    L[0] = mul(Minv, points[0] - P);
    L[1] = mul(Minv, points[1] - P);
    L[2] = mul(Minv, points[2] - P);
    L[3] = mul(Minv, points[3] - P);

    // project rect onto sphere
    L[0] = normalize(L[0]);
    L[1] = normalize(L[1]);
    L[2] = normalize(L[2]);
    L[3] = normalize(L[3]);

    vec3 vsum = vec3_splat(0.0);

    vsum += IntegrateEdgeVec(L[0], L[1]);
    vsum += IntegrateEdgeVec(L[1], L[2]);
    vsum += IntegrateEdgeVec(L[2], L[3]);
    vsum += IntegrateEdgeVec(L[3], L[0]);

    float len = length(vsum);
    float z = vsum.z/len;

    // clipless approximation: tabulated horizon-clipped sphere
    // visually better than alternatives, but produces artifacts at low roughness values
    vec2 uv = vec2(z*0.5 + 0.5, len);
    uv = uv*LUT_SCALE + LUT_BIAS;

    float scale = texture2D(ltc_2, uv).w;
    return len*scale;
}

void InitRectPoints(vec3 center, vec3 ex, vec3 ey, out vec3 points[4])
{
    points[0] = center - ex - ey;
    points[1] = center + ex - ey;
    points[2] = center + ex + ey;
    points[3] = center - ex + ey;
}

void evaluateRectLight(sampler2D ltc_1, sampler2D ltc_2, float roughness, vec3 N, vec3 pos, vec3 lpos, vec3 halfwidth, vec3 halfheight,
                       inout vec3 scol, inout vec3 dcol)
{
    vec3 points[4];
    InitRectPoints(lpos, halfwidth, halfheight, points);

    vec3 V = -normalize(pos);

    float ndotv = saturate(dot(N, V));
    vec2 uv = vec2(roughness, sqrt(1.0 - ndotv));
    uv = uv*LUT_SCALE + LUT_BIAS;

    vec4 t1 = texture2D(ltc_1, uv);

    mat3 Minv = mtxFromCols(
        vec3(t1.x, 0.0, t1.y),
        vec3( 0.0, 1.0,  0.0),
        vec3(t1.z, 0.0, t1.w)
    );

    float spec = LTC_Evaluate(N, V, pos, Minv, points, ltc_2);

    // LTC Fresnel Approximation by Stephen Hill
    // http://blog.selfshadow.com/publications/s2016-advances/s2016_ltc_fresnel.pdf
    vec4 t2 = texture2D(ltc_2, uv);
    scol = (scol*t2.x + (1.0 - scol)*t2.y)*spec;

    mat3 Meye = mat3(
        vec3(1.0, 0.0, 0.0),
        vec3(0.0, 1.0, 0.0),
        vec3(0.0, 0.0, 1.0)
    );

    dcol *= LTC_Evaluate(N, V, pos, Meye, points, ltc_2);
}