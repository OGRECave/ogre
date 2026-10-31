OGRE_NATIVE_GLSL_VERSION_DIRECTIVE
#include <OgreUnifiedShader.h>
#include <OgreUnifiedCompute.h>

writeonly IMAGE2D(rgba8, image_data, 0);

OGRE_COMPUTE_UNIFORMS(
uniform float roll;
)

// source/ details: http://wili.cc/blog/opengl-cs.html
MAIN_COMPUTE(16, 16, 1)
{
    ivec2 storePos = ivec2(gl_GlobalInvocationID.xy);

    float localCoef = length(vec2(ivec2(gl_LocalInvocationID.xy)-8)/8.0);
    float globalCoef = sin(float(gl_WorkGroupID.x+gl_WorkGroupID.y)*0.1 + roll)*0.5;
    imageStore(image_data, storePos, vec4(vec2_splat(1.0-globalCoef*localCoef),1.0,1.0));
}
