/******************************************************************************
Copyright (c) W.J. van der Laan
Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:
The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/
/** Deferred shading framework
	// W.J. :wumpus: van der Laan 2005 //
	
	Material shader: Normal mapped
*/
struct VS_OUTPUT 
{
   float4 pos: POSITION;
   float4 normal: TEXCOORD0;
   float2 texCoord0: TEXCOORD1;
   float depth: TEXCOORD2;
   float3 tangent: TEXCOORD3;
   float3 binormal: TEXCOORD4;
};
float4x4 worldViewProj;
float4x4 world;
float4x4 worldView;
VS_OUTPUT main(
	float4 Pos: POSITION, 
	float3 normal: NORMAL,
	float2 texCoord0: TEXCOORD0,
	float3 tangent: TANGENT0
)
{
   VS_OUTPUT Out;
   
   Out.normal = mul(worldView, normal);
   float4 projPos = mul(worldViewProj, Pos);
   
   Out.tangent = mul(worldView, tangent);
   Out.binormal = cross(Out.normal, Out.tangent);
   
   Out.pos = projPos;
   Out.texCoord0 = texCoord0;
   Out.depth = projPos.w;
   //Out.depth = projPos.z/projPos.w;
   return Out;
}

