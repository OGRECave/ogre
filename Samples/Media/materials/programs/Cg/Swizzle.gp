!!NVgp4.0
PRIMITIVE_IN TRIANGLES;
PRIMITIVE_OUT TRIANGLE_STRIP;
VERTICES_OUT 6;
# cgc version 2.0.0015, build date May 15 2008
# command line args: -profile gpu_gp
# source file: gs_simple.cg
#vendor NVIDIA Corporation
#version 2.0.0.15
#profile gpu_gp
#program geometry_swizzle
#semantic geometry_swizzle.position : POSITION
#var float4 position.__data[0] : $vin.VERTEX[0].POSITION : HPOS[0][32] : 0 : 1
#var float4 position.__data[1] : $vin.VERTEX[1].POSITION : HPOS[1][32] : 0 : 1
#var float4 position.__data[2] : $vin.VERTEX[2].POSITION : HPOS[2][32] : 0 : 1
ATTRIB vertex_position = vertex.position;
TEMP RC, HC;
MOV.F result.position, vertex[0].position;
MOV.F result.color, {1, 0}.xyyx;
EMIT;
MOV.F result.position, vertex[1].position;
MOV.F result.color, {1, 0}.xyyx;
EMIT;
MOV.F result.position, vertex[2].position;
MOV.F result.color, {1, 0}.xyyx;
EMIT;
ENDPRIM;
MOV.F result.position, vertex[0].position.yxzw;
MOV.F result.color, {0, 1}.xxyy;
EMIT;
MOV.F result.position, vertex[1].position.yxzw;
MOV.F result.color, {0, 1}.xxyy;
EMIT;
MOV.F result.position, vertex[2].position.yxzw;
MOV.F result.color, {0, 1}.xxyy;
EMIT;
ENDPRIM;
END
# 20 instructions, 0 R-regs