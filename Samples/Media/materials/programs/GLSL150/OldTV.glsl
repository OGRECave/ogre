#version 150

in vec4 pos;
in vec2 oUv0;
out vec4 fragColour;

uniform sampler2D Image;
uniform sampler3D Rand;
uniform sampler3D Noise;
uniform float distortionFreq;
uniform float distortionScale;
uniform float distortionRoll;
uniform float interference;
uniform float frameLimit;
uniform float frameShape;
uniform float frameSharpness;
uniform float time_0_X;
uniform float sin_time_0_X;

void main()
{
   // Define a frame shape
   float f = (1 - pos.x * pos.x) * (1 - pos.y * pos.y);
   float frame = clamp(frameSharpness * (pow(f, frameShape) - frameLimit), 0.0, 1.0);

   // Interference ... just a texture filled with rand()
   float rand = texture(Rand, vec3(1.5 * pos.x, 1.5 * pos.y, time_0_X)).x - 0.2;

   // Some signed noise for the distortion effect
   float noisy = texture(Noise, vec3(0, 0.5 * pos.y, 0.1 * time_0_X)).x - 0.5;

   // Repeat a 1 - x^2 (0 < x < 1) curve and roll it with sinus.
   float dst = fract(pos.y * distortionFreq + distortionRoll * sin_time_0_X);
   dst *= (1 - dst);
   // Make sure distortion is highest in the center of the image
   dst /= 1 + distortionScale * abs(pos.y);

   // ... and finally distort
   vec2 inUv = oUv0;
   inUv.x += distortionScale * noisy * dst;
   vec4 image = texture(Image, inUv);

   // Combine frame, distorted image and interference
   fragColour = frame * (interference * rand + image);
}

