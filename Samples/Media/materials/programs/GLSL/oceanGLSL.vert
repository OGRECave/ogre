// oceanGLSL.vert
// vertex program for Ocean water simulation
// 05 Aug 2005
// adapted for Ogre by nfz
// converted from HLSL to GLSL
// original shader source from Render Monkey 1.6 Reflections Refractions.rfx

// 06 Aug 2005: moved uvw calculation from fragment program into vertex program 

uniform vec3 scale;
uniform vec3 eyePosition;
uniform vec2 waveSpeed;
uniform float noiseSpeed;
uniform float time_0_X;
uniform mat4 worldViewProj;

varying vec3 uvw;
varying vec3 normal;
varying vec3 vVec;

void main(void)
{
   gl_Position = worldViewProj * gl_Vertex;
   
   //  the view vector needs to be in vertex space
   vVec = gl_Vertex.xyz - eyePosition;
   normal = gl_Normal;
   // uvw is the calculated uvw coordinates based on vertex position
   uvw = gl_Vertex.xyz * scale.xyz;
   uvw.xz += waveSpeed * time_0_X;
   uvw.y += uvw.z + noiseSpeed * time_0_X;
   
}
