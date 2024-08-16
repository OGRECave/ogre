#ifdef USE_LAYOUT
#version 150
#extension GL_EXT_geometry_shader4 : require

out vec4 colour;
layout(triangles) in;
layout(line_strip, max_vertices = 6) out;
#else
#version 120
#extension GL_EXT_geometry_shader4 : require

varying out vec4 colour;
#endif

uniform vec4 origColour;
uniform vec4 cloneColour;

void main(void)
{
    /////////////////////////////////////////////////////////////
    //This example has two parts
    //  step a) draw the primitive pushed down the pipeline
    //           there are gl_Vertices # of vertices
    //           put the vertex value into gl_Position
    //           use EmitVertex => 'create' a new vertex
    //          use EndPrimitive to signal that you are done creating a primitive!
    //  step b) create a new piece of geometry (I.E. WHY WE ARE USING A GEOMETRY SHADER!)
    //          I just do the same loop, but swizzle the x and y values
    //  result => the line we want to draw, and the same line, but along the other axis

    //Pass-thru!
    int i;
    for (i = 0; i < gl_VerticesIn; i++) {
        gl_Position = gl_PositionIn[i];
        colour = origColour;
        EmitVertex();
    }
    EndPrimitive();

    //New piece of geometry!  We just swizzle the x and y terms
    for (i = 0; i < gl_VerticesIn; i++){
        gl_Position = gl_PositionIn[i];
        gl_Position.xy = gl_Position.yx;
        colour = cloneColour;
        EmitVertex();
    }
    EndPrimitive();
}
