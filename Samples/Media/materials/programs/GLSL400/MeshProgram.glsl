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

layout(binding=0, row_major) uniform OgreUniforms
{
    mat4 MVP;
    float t;
};

#ifdef VULKAN
// SSBOs cannot be used with Vulkan yet
float data[] = {
    -100, -100, 0,  // pos
    0,0,1,          // normal
    0,1,            // texcoord
    100, -100, 0,
    0,0,1,
    1,1,
    100,  100, 0,
    0,0,1,
    1,0,
    -100,  100, 0 ,
    0,0,1,
    0,0
};
#else
//![vertexbuffer]
// buffer at index 0, which is expected to contain float data
layout(binding = 3) readonly buffer VertexDataIn
{
    float data[];
};
//![vertexbuffer]
#endif

void main()
{
    SetMeshOutputsEXT(4, 2);

    float frac = gl_WorkGroupID.x / 6.0;

    for (int i = 0; i < 4; i++)
    {
        vec4 pos = vec4(data[i * 8 + 0], data[i * 8 + 1], data[i * 8 + 2], 1);
        pos.x = pos.x + 250.0 * gl_WorkGroupID.x - 625.0;

        gl_MeshVerticesEXT[i].gl_Position = MVP * pos;
        v_out[i].color = vec3(data[i * 8 + 6], data[i * 8 + 7], sin(mod((t + frac) * 3.14, 3.14)));
    }

#ifdef VULKAN
    gl_PrimitiveTriangleIndicesEXT[0] = uvec3(0, 1, 2);
    gl_PrimitiveTriangleIndicesEXT[1] = uvec3(0, 2, 3);
#else
    gl_PrimitiveIndicesNV[0] = 0;
    gl_PrimitiveIndicesNV[1] = 1;
    gl_PrimitiveIndicesNV[2] = 2;
    gl_PrimitiveIndicesNV[3] = 0;
    gl_PrimitiveIndicesNV[4] = 2;
    gl_PrimitiveIndicesNV[5] = 3;
#endif
}