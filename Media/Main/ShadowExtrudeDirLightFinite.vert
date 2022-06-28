#include <OgreUnifiedShader.h>

// Directional light extrude - FINITE
uniform mat4 worldviewproj_matrix;
uniform vec4 light_position_object_space; // homogenous, object space
uniform float shadow_extrusion_distance;  // how far to extrude

MAIN_PARAMETERS
IN(vec4 uv0, TEXCOORD0)
IN(vec4 position, POSITION)
MAIN_DECLARATION
{
    // Extrusion in object space
    // Vertex unmodified if w==1, extruded if w==0
    vec3 extrusionDir = - light_position_object_space.xyz;
    extrusionDir = normalize(extrusionDir);

    vec4 newpos = vec4(position.xyz +
        ((1.0 - uv0.x) * shadow_extrusion_distance * extrusionDir), 1.0);

    gl_Position = mul(worldviewproj_matrix, newpos);
}