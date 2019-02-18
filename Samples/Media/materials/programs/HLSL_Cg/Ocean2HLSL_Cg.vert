/*********************************************************************NVMH3****
Copyright NVIDIA Corporation 2003
TO THE MAXIMUM EXTENT PERMITTED BY APPLICABLE LAW, THIS SOFTWARE IS PROVIDED
*AS IS* AND NVIDIA AND ITS SUPPLIERS DISCLAIM ALL WARRANTIES, EITHER EXPRESS
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY
AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL NVIDIA OR ITS SUPPLIERS
BE LIABLE FOR ANY SPECIAL, INCIDENTAL, INDIRECT, OR CONSEQUENTIAL DAMAGES
WHATSOEVER (INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS,
BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
ARISING OUT OF THE USE OF OR INABILITY TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.


Comments:
	Simple ocean shader with animated bump map and geometric waves
	Based partly on "Effective Water Simulation From Physical Models", GPU Gems

11 Aug 05: heavily modified by Jeff Doyle (nfz) for Ogre

******************************************************************************/

struct a2v {
	float4 Position : POSITION;   // in object space
	float2 TexCoord : TEXCOORD0;
};

struct v2f {
	float4 Position  : POSITION;  // in clip space
	float3 rotMatrix1 : TEXCOORD0; // first row of the 3x3 transform from tangent to obj space
	float3 rotMatrix2 : TEXCOORD1; // second row of the 3x3 transform from tangent to obj space
	float3 rotMatrix3 : TEXCOORD2; // third row of the 3x3 transform from tangent to obj space

	float2 bumpCoord0 : TEXCOORD3;
	float2 bumpCoord1 : TEXCOORD4;
	float2 bumpCoord2 : TEXCOORD5;

	float3 eyeVector  : TEXCOORD6;
};

// wave functions

struct Wave {
  float freq;  // 2*PI / wavelength
  float amp;   // amplitude
  float phase; // speed * 2*PI / wavelength
  float2 dir;
};

v2f main(a2v IN,
		uniform float4x4 WorldViewProj,
		uniform float3 eyePosition,
		uniform float BumpScale,
		uniform float2 textureScale,
		uniform float2 bumpSpeed,
		uniform float time,
		uniform float waveFreq,
		uniform float waveAmp
        )
{
	v2f OUT;

	#define NWAVES 2
	Wave wave[NWAVES] = {
		{ 1.0, 1.0, 0.5, float2(-1, 0) },
		{ 2.0, 0.5, 1.7, float2(-0.7, 0.7) }
	};

    wave[0].freq = waveFreq;
    wave[0].amp = waveAmp;

    wave[1].freq = waveFreq * 3.0;
    wave[1].amp = waveAmp * 0.33;

    float4 P = IN.Position;

	// sum waves
	float ddx = 0.0, ddy = 0.0;
	float deriv;
	float angle;

	// wave synthesis using two sine waves at different frequencies and phase shift
	for(int i = 0; i<NWAVES; ++i)
	{
		angle = dot(wave[i].dir, P.xz) * wave[i].freq + time * wave[i].phase;
		P.y += wave[i].amp * sin( angle );
		// calculate derivate of wave function
		deriv = wave[i].freq * wave[i].amp * cos(angle);
		ddx -= deriv * wave[i].dir.x;
		ddy -= deriv * wave[i].dir.y;
	}

	// compute the 3x3 transform from tangent space to object space
	// first rows are the tangent and binormal scaled by the bump scale

	OUT.rotMatrix1.xyz = BumpScale * normalize(float3(1, ddy, 0)); // Binormal
	OUT.rotMatrix2.xyz = BumpScale * normalize(float3(0, ddx, 1)); // Tangent
	OUT.rotMatrix3.xyz = normalize(float3(ddx, 1, ddy)); // Normal

	OUT.Position = mul(WorldViewProj, P);

	// calculate texture coordinates for normal map lookup
	OUT.bumpCoord0.xy = IN.TexCoord*textureScale + time * bumpSpeed;
	OUT.bumpCoord1.xy = IN.TexCoord*textureScale * 2.0 + time * bumpSpeed * 4.0;
	OUT.bumpCoord2.xy = IN.TexCoord*textureScale * 4.0 + time * bumpSpeed * 8.0;

	OUT.eyeVector = P.xyz - eyePosition; // eye position in vertex space
	return OUT;
}
