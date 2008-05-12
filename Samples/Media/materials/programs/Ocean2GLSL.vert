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

11 Aug 05: converted from HLSL to GLSL by Jeff Doyle (nfz) to work in Ogre

******************************************************************************/

uniform vec3 eyePosition;
uniform float BumpScale;
uniform vec2 textureScale;
uniform vec2 bumpSpeed;
uniform float time;
uniform float waveFreq;
uniform float waveAmp;

varying mat3 rotMatrix; //  transform from tangent to obj space
varying vec2 bumpCoord0;
varying vec2 bumpCoord1;
varying vec2 bumpCoord2;
varying vec3 eyeVector;

// wave functions
struct Wave {
  float freq;  // 2*PI / wavelength
  float amp;   // amplitude
  float phase; // speed * 2*PI / wavelength
  vec2 dir;
};


void main(void)
{

	#define NWAVES 2

	Wave wave[NWAVES];

	wave[0] = Wave( waveFreq, waveAmp, 0.5, vec2(-1, 0) );
	wave[1] = Wave( 3.0 * waveFreq, 0.33 * waveAmp, 1.7, vec2(-0.7, 0.7) );


    vec4 P = gl_Vertex;

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

	// compute the 3x3 tranform from tangent space to object space
	// compute tangent basis
    vec3 T = normalize(vec3(1.0, ddy, 0.0)) * BumpScale;
    vec3 B = normalize(vec3(0.0, ddx, 1.0)) * BumpScale;
    vec3 N = normalize(vec3(ddx, 1.0, ddy));

	rotMatrix = mat3(T, B, N);

	gl_Position = gl_ModelViewProjectionMatrix * P;

	// calculate texture coordinates for normal map lookup
	bumpCoord0.xy = gl_MultiTexCoord0.xy * textureScale + time * bumpSpeed;
	bumpCoord1.xy = gl_MultiTexCoord0.xy * textureScale * 2.0 + time * bumpSpeed * 4.0;
	bumpCoord2.xy = gl_MultiTexCoord0.xy * textureScale * 4.0 + time * bumpSpeed * 8.0;


	eyeVector = P.xyz - eyePosition; // eye position in vertex space
}
