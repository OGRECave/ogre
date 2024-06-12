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
    
    Post shader: Light geometry material
*/

#define LIGHT_POINT         1
#define LIGHT_SPOT          2
#define LIGHT_DIRECTIONAL   3

//////////////////////////////////////////////////////////////////////////////
// Helper function section
//////////////////////////////////////////////////////////////////////////////
OGRE_NATIVE_GLSL_VERSION_DIRECTIVE

void checkShadow(
    sampler2D shadowMap,
    vec3 viewPos,
    mat4 invView,
    mat4 shadowViewProj,
    float shadowFarClip,
#if LIGHT_TYPE == LIGHT_DIRECTIONAL
    vec3 shadowCamPos
#else
    float distanceFromLight
#endif
    )
{
    vec3 worldPos = (invView * vec4(viewPos, 1)).xyz;
#if LIGHT_TYPE == LIGHT_DIRECTIONAL
    float distanceFromLight = length(shadowCamPos-worldPos);
#endif
    vec4 shadowProjPos = shadowViewProj * vec4(worldPos,1);
    shadowProjPos /= shadowProjPos.w;
    vec2 shadowSampleTexCoord = shadowProjPos.xy;
    float shadowDepth = texture(shadowMap, shadowSampleTexCoord).r;
    float shadowDistance = shadowDepth * shadowFarClip;
    if((shadowDistance - distanceFromLight + 0.1) < 0.0)
        discard;
}

//////////////////////////////////////////////////////////////////////////////
// Main shader section
//////////////////////////////////////////////////////////////////////////////    

#if LIGHT_TYPE == LIGHT_DIRECTIONAL
in vec2 oUv0;
in vec3 oRay;
#else
in vec4 oPos;
#endif
    
uniform sampler2D Tex0;
uniform sampler2D Tex1;

#if LIGHT_TYPE != LIGHT_POINT
uniform vec3 lightDir;
#endif

#if LIGHT_TYPE == LIGHT_SPOT
uniform vec4 spotParams;
#endif

#if LIGHT_TYPE != LIGHT_DIRECTIONAL
uniform float vpWidth;
uniform float vpHeight;
uniform vec3 farCorner;
uniform float flip;
#endif

#ifdef IS_SHADOW_CASTER
uniform mat4 invView;
uniform mat4 shadowViewProjMat;
uniform sampler2D ShadowTex;
uniform vec3 shadowCamPos;
uniform float shadowFarClip;
#endif

uniform float farClipDistance;
// Attributes of light
uniform vec4 lightDiffuseColor;
uniform vec4 lightSpecularColor;
uniform vec4 lightFalloff;
uniform vec3 lightPos;

out vec4 fragColour;

void main()
{
    // None directional lights have some calculations to do in the beginning of the pixel shader
#if LIGHT_TYPE != LIGHT_DIRECTIONAL
    vec4 normProjPos = oPos / oPos.w;
    // -1 is because generally +Y is down for textures but up for the screen
    vec2 oUv0 = vec2(normProjPos.x, normProjPos.y * -1.0 * flip) * 0.5 + 0.5;
    vec3 oRay = vec3(normProjPos.x, normProjPos.y * flip, 1.0) * farCorner;
#endif
    
    vec4 a0 = texture(Tex0, oUv0); // Attribute 0: Diffuse color+shininess
    vec4 a1 = texture(Tex1, oUv0); // Attribute 1: Normal+depth

    // Attributes
    vec3 colour = a0.rgb;
    float specularity = a0.a;
    float distance = a1.w;  // Distance from viewer (w)
    vec3 normal = a1.xyz;

    // Calculate position of texel in view space
    vec3 viewPos = normalize(oRay)*distance*farClipDistance;

    // Calculate light direction and distance
#if LIGHT_TYPE == LIGHT_DIRECTIONAL
    vec3 objToLightDir = -lightDir.xyz;
#else
    vec3 objToLightVec = lightPos - viewPos;
    float len_sq = dot(objToLightVec, objToLightVec);
    float len = sqrt(len_sq);
    vec3 objToLightDir = objToLightVec/len;
#endif

#ifdef IS_SHADOW_CASTER
    #if LIGHT_TYPE == LIGHT_DIRECTIONAL
        checkShadow(ShadowTex, viewPos, invView, shadowViewProjMat, shadowFarClip, shadowCamPos);
    #else
        checkShadow(ShadowTex, viewPos, invView, shadowViewProjMat, shadowFarClip, len);
    #endif
#endif
    
    // Calculate diffuse colour
    vec3 total_light_contrib;
    total_light_contrib = max(0.0,dot(objToLightDir, normal)) * lightDiffuseColor.rgb;

#if IS_SPECULAR
    // Calculate specular component
    vec3 viewDir = -normalize(viewPos);
    vec3 h = normalize(viewDir + objToLightDir);
    vec3 light_specular = pow(dot(normal, h),32.0) * lightSpecularColor.rgb;

    total_light_contrib += specularity * light_specular;
#endif

#if IS_ATTENUATED
    if(lightFalloff.x - len < 0.0)
        discard;
    // Calculate attenuation
    float attenuation = dot(lightFalloff.yzw, vec3(1.0, len, len_sq));
    total_light_contrib /= attenuation;
#endif

#if LIGHT_TYPE == LIGHT_SPOT
    float spotlightAngle = clamp(dot(lightDir.xyz, -objToLightDir), 0.0, 1.0);
    float spotFalloff = clamp((spotlightAngle - spotParams.x) / (spotParams.y - spotParams.x), 0.0, 1.0);
    total_light_contrib *= (1.0-spotFalloff);
#endif

    fragColour = vec4(total_light_contrib*colour, 0.0);
}
