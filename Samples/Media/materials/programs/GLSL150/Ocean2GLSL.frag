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

#version 150

uniform sampler2D NormalMap;
uniform samplerCube EnvironmentMap;
uniform vec4 deepColor;
uniform vec4 shallowColor;
uniform vec4 reflectionColor;
uniform float reflectionAmount;
uniform float reflectionBlur;
uniform float waterAmount;
uniform float fresnelPower;
uniform float fresnelBias;
uniform float hdrMultiplier;

in mat3 rotMatrix; // first row of the 3x3 transform from tangent to cube space
in vec2 bumpCoord0;
in vec2 bumpCoord1;
in vec2 bumpCoord2;
in vec3 eyeVector;

out vec4 fragColour;

void main(void)
{
	// sum normal maps
	// sample from 3 different points so no texture repetition is noticeable
    vec4 t0 = texture(NormalMap, bumpCoord0) * 2.0 - 1.0;
    vec4 t1 = texture(NormalMap, bumpCoord1) * 2.0 - 1.0;
    vec4 t2 = texture(NormalMap, bumpCoord2) * 2.0 - 1.0;
    vec3 N = t0.xyz + t1.xyz + t2.xyz;

    N = normalize(rotMatrix * N);

	// reflection
    vec3 E = normalize(eyeVector);
    vec3 R = reflect(E, N);
    // Ogre conversion for cube map lookup
    R.z = -R.z;

    vec4 reflection = texture(EnvironmentMap, R, reflectionBlur);
    // cheap hdr effect
    reflection.rgb *= (reflection.r + reflection.g + reflection.b) * hdrMultiplier;

	// fresnel
    float facing = 1.0 - dot(-E, N);
    float fresnel = clamp(fresnelBias + pow(facing, fresnelPower), 0.0, 1.0);

    vec4 waterColor = mix(shallowColor, deepColor, facing) * waterAmount;

    reflection = mix(waterColor,  reflection * reflectionColor, fresnel) * reflectionAmount;
    fragColour = waterColor + reflection;
}
