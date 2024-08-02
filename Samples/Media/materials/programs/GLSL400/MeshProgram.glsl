#version 450

#ifndef VULKAN
#extension GL_NV_mesh_shader : require
#define gl_MeshVerticesEXT gl_MeshVerticesNV
#define SetMeshOutputsEXT(verts, prims) gl_PrimitiveCountNV = (prims)
#else
#extension GL_EXT_mesh_shader : require
#endif

layout(local_size_x=1) in;
layout(max_vertices=4, max_primitives=2, triangles) out;


layout(location=0) out PerVertexData
{
    vec3 color;
} v_out[];


void main()
{
    SetMeshOutputsEXT(4, 2);

    float frac = gl_WorkGroupID.x / 6.0;

    float xmin = -1.0 + frac * 2.0;
    float xmax = xmin + 2.0 / (7.0);

    gl_MeshVerticesEXT[0].gl_Position = vec4(xmin, -0.5, 0.0, 1.0); // Upper Left
    gl_MeshVerticesEXT[1].gl_Position = vec4(xmax, -0.5, 0.0, 1.0); // Upper Right
    gl_MeshVerticesEXT[2].gl_Position = vec4(xmin,  0.5, 0.0, 1.0); // Bottom Left
    gl_MeshVerticesEXT[3].gl_Position = vec4(xmax,  0.5, 0.0, 1.0); // Bottom Right

    v_out[0].color = vec3(1.0, 0.0, 0.0);
    v_out[1].color = vec3(0.0, 1.0, 0.0);
    v_out[2].color = vec3(0.0, 0.0, 1.0);
    v_out[3].color = vec3(1.0, 1.0, 0.0);

#ifdef VULKAN
    gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0, 1, 2);
    gl_PrimitiveTriangleIndicesEXT[1] = uvec3(2, 1, 3);
#else
    gl_PrimitiveIndicesNV[0] = 0;
    gl_PrimitiveIndicesNV[1] = 1;
    gl_PrimitiveIndicesNV[2] = 2;
    gl_PrimitiveIndicesNV[3] = 2;
    gl_PrimitiveIndicesNV[4] = 1;
    gl_PrimitiveIndicesNV[5] = 3;
#endif
}