#version 120
#extension GL_EXT_geometry_shader4 : enable

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
    for (i = 0; i < gl_VerticesIn; i++) {
        gl_Position = gl_PositionIn[i];
        gl_FrontColor = origColour;
        EmitVertex();
    }
    EndPrimitive();

    //New piece of geometry!  We just swizzle the x and y terms
    for (i = 0; i < gl_VerticesIn; i++){
        gl_Position = gl_PositionIn[i];
        gl_Position.xy = gl_Position.yx;
        gl_FrontColor = cloneColour;
        EmitVertex();
    }
    EndPrimitive();

}
