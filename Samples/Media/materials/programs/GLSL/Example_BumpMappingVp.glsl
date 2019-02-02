/* Bump mapping vertex program
   In this program, we want to calculate the tangent space light vector
   on a per-vertex level which will get passed to the fragment program,
   or to the fixed function dot3 operation, to produce the per-pixel
   lighting effect.
*/
// parameters
uniform vec4 lightPosition; // object space
uniform mat4 worldViewProj;

attribute vec4 vertex;
attribute vec3 normal;
attribute vec3 tangent;
attribute vec2 uv0;

varying vec2 oUv0;
varying vec3 oTSLightDir;

void main()
{
    // Calculate output position
    gl_Position = worldViewProj * vertex;

    // Pass the main uvs straight through unchanged
    oUv0 = uv0;

    // Calculate tangent space light vector
    // Get object space light direction
    // Non-normalised since we'll do that in the fragment program anyway
    vec3 lightDir = lightPosition.xyz - (vertex * lightPosition.w).xyz;

    // Calculate the binormal (NB we assume both normal and tangent are
    // already normalised)

    // Fixed handedness
    vec3 binormal = cross(normal, tangent);

    // Form a rotation matrix out of the vectors, column major for glsl es
    mat3 rotation = mat3(vec3(tangent[0], binormal[0], normal[0]),
                         vec3(tangent[1], binormal[1], normal[1]),
                         vec3(tangent[2], binormal[2], normal[2]));

    // Transform the light vector according to this matrix
    oTSLightDir = rotation * lightDir;
}
