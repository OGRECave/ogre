// oceanGLSL.frag
// fragment program for Ocean water simulation
// 05 Aug 2005
// adapted for Ogre by nfz
// converted from HLSL to GLSL
// original shader source from Render Monkey 1.6 Reflections Refractions.rfx

// 06 Aug 2005: moved uvw calculation from fragment program into vertex program 

uniform float fadeBias;
uniform float fadeExp;
uniform vec4 waterColor;
uniform sampler3D Noise;
uniform samplerCube skyBox;

varying vec3 uvw;
varying vec3 normal;
varying vec3 vVec;

void main(void)
{
   vec3 noisy = texture3D(Noise, uvw).xyz;
   
   // convert to signed noise
   vec3 bump = 2.0 * noisy - 1.0;
   bump.xz *= 0.15;
   // Make sure the normal always points upwards
   // note that Ogres y axis is vertical (RM Z axis is vertical)
   bump.y = 0.8 * abs(bump.y) + 0.2;
   // Offset the surface normal with the bump
   bump = normalize(normal + bump);

   // Find the reflection vector
   vec3 normView = normalize(vVec);
   vec3 reflVec = reflect(normView, bump);
   // Ogre has z flipped for cubemaps
   reflVec.z = -reflVec.z;
   vec4 refl = textureCube(skyBox, reflVec);

   // set up for fresnel calc
   float lrp = 1.0 - dot(-normView, bump);
   
   // Interpolate between the water color and reflection for fresnel effect
   gl_FragColor = mix(waterColor, refl, clamp(fadeBias + pow(lrp, fadeExp), 0.0, 1.0) );
}
