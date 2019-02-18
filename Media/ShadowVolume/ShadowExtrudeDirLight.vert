// Directional light extrude
attribute vec4 uv0;
attribute vec4 position;

uniform mat4 worldviewproj_matrix;
uniform vec4 light_position_object_space; // homogenous, object space

void main()
{
    // Extrusion in object space
    // Vertex unmodified if w==1, extruded if w==0
    vec4 newpos =
        (uv0.xxxx * (position + light_position_object_space)) - light_position_object_space;

    gl_Position = worldviewproj_matrix * newpos;
}