OGRE_NATIVE_GLSL_VERSION_DIRECTIVE
#include <OgreUnifiedShader.h>

IN(vec4 col, COLOR)
IN(vec3 con, TEXCOORD0)
IN(vec2 pixf, TEXCOORD1)

// Original CUDA implementation: https://github.com/graphdeco-inria/diff-gaussian-rasterization/blob/main/cuda_rasterizer/forward.cu#L263
void main() {
    vec2 d = (gl_PointCoord*2.0 - 1.0) * -pixf;
	d.x *= -1.0;
    // Resample using conic matrix (cf. "Surface
    // Splatting" by Zwicker et al., 2001)
    float power = -0.5 * (con.x * d.x * d.x + con.z * d.y * d.y) - con.y * d.x * d.y;

    if (power > 0.) {
        discard;
    }

    // Eq. (2) from 3D Gaussian splatting paper.
    float alpha = min(.99, col.a * exp(power));

    // we render in reverse order, so we can use normal alpha blending
    gl_FragColor = vec4(col.rgb, alpha);
}