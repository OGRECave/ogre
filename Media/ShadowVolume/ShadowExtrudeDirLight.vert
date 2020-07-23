#include <OgreUnifiedShader.h>

// Directional light extrude
uniform mat4 worldviewproj_matrix;
uniform vec4 light_position_object_space; // homogenous, object space

MAIN_PARAMETERS
IN(vec4 uv0, TEXCOORD0)
IN(vec4 position, POSITION)
MAIN_DECLARATION
{
    // Extrusion in object space
    // Vertex unmodified if w==1, extruded if w==0
    vec4 newpos =
        (uv0.xxxx * (position + light_position_object_space)) - light_position_object_space;

    gl_Position = mul(worldviewproj_matrix, newpos);
}