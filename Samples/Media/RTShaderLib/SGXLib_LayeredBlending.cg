/*
** layered blending & misc math
** Blending modes, RGB/HSL/Contrast/Desaturate
**
** The shaders below are base on the shaders created by:
** Romain Dura | Romz
** Blog: http://blog.mouaif.org
** Post: http://blog.mouaif.org/?p=94
*/


/*
** Desaturation
*/

float4 Desaturate(float3 color, float Desaturation)
{
	float3 grayXfer = float3(0.3, 0.59, 0.11);
	float grayf = dot(grayXfer, color);
	float3 gray = float3(grayf, grayf, grayf);
	return float4(lerp(color, gray, Desaturation), 1.0);
}


/*
** Hue, saturation, luminance
*/

float3 RGBToHSL(float3 color)
{
	float3 hsl; // init to 0 to avoid warnings ? (and reverse if + remove first part)
	
	float fmin = min(min(color.r, color.g), color.b);    //Min. value of RGB
	float fmax = max(max(color.r, color.g), color.b);    //Max. value of RGB
	float delta = fmax - fmin;             //Delta RGB value

	hsl.z = (fmax + fmin) / 2.0; // Luminance

	if (delta == 0.0)		//This is a gray, no chroma...
	{
		hsl.x = 0.0;	// Hue
		hsl.y = 0.0;	// Saturation
	}
	else                                    //Chromatic data...
	{
		if (hsl.z < 0.5)
			hsl.y = delta / (fmax + fmin); // Saturation
		else
			hsl.y = delta / (2.0 - fmax - fmin); // Saturation
		
		float deltaR = (((fmax - color.r) / 6.0) + (delta / 2.0)) / delta;
		float deltaG = (((fmax - color.g) / 6.0) + (delta / 2.0)) / delta;
		float deltaB = (((fmax - color.b) / 6.0) + (delta / 2.0)) / delta;

		if (color.r == fmax )
			hsl.x = deltaB - deltaG; // Hue
		else if (color.g == fmax)
			hsl.x = (1.0 / 3.0) + deltaR - deltaB; // Hue
		else if (color.b == fmax)
			hsl.x = (2.0 / 3.0) + deltaG - deltaR; // Hue

		if (hsl.x < 0.0)
			hsl.x += 1.0; // Hue
		else if (hsl.x > 1.0)
			hsl.x -= 1.0; // Hue
	}

	return hsl;
}

float HueToRGB(float f1, float f2, float hue)
{
	if (hue < 0.0)
		hue += 1.0;
	else if (hue > 1.0)
		hue -= 1.0;
	float res;
	if ((6.0 * hue) < 1.0)
		res = f1 + (f2 - f1) * 6.0 * hue;
	else if ((2.0 * hue) < 1.0)
		res = f2;
	else if ((3.0 * hue) < 2.0)
		res = f1 + (f2 - f1) * ((2.0 / 3.0) - hue) * 6.0;
	else
		res = f1;
	return res;
}

float3 HSLToRGB(float3 hsl)
{
	float3 rgb;
	
	if (hsl.y == 0.0)
		rgb = float3(hsl.z, hsl.z, hsl.z); // Luminance
	else
	{
		float f2;
		
		if (hsl.z < 0.5)
			f2 = hsl.z * (1.0 + hsl.y);
		else
			f2 = (hsl.z + hsl.y) - (hsl.y * hsl.z);
			
		float f1 = 2.0 * hsl.z - f2;
		
		rgb.r = HueToRGB(f1, f2, hsl.x + (1.0/3.0));
		rgb.g = HueToRGB(f1, f2, hsl.x);
		rgb.b= HueToRGB(f1, f2, hsl.x - (1.0/3.0));
	}
	
	return rgb;
}

/*
** Contrast, saturation, brightness
** Code of this function is from TGM's shader pack
** http://irrlicht.sourceforge.net/phpBB2/viewtopic.php?t=21057
*/

// For all settings: 1.0 = 100% 0.5=50% 1.5 = 150%
float3 ContrastSaturationBrightness(float3 color, float brt, float sat, float con)
{
	// Increase or decrease theese values to adjust r, g and b color channels seperately
	const float AvgLumR = 0.5;
	const float AvgLumG = 0.5;
	const float AvgLumB = 0.5;
	
	const float3 LumCoeff = float3(0.2125, 0.7154, 0.0721);
	
	float3 AvgLumin = float3(AvgLumR, AvgLumG, AvgLumB);
	float3 brtColor = color * brt;
	float intensityf = dot(brtColor, LumCoeff);
	float3 intensity = float3(intensityf, intensityf, intensityf);
	float3 satColor = lerp(intensity, brtColor, sat);
	float3 conColor = lerp(AvgLumin, satColor, con);
	return conColor;
}

/*
** Float blending modes
** Adapted from here: http://www.nathanm.com/photoshop-blending-math/
** But I modified the HardMix (wrong condition), Overlay, SoftLight, ColorDodge, ColorBurn, VividLight, PinLight (inverted layers) ones to have correct results
*/

#define BlendLinearDodgef 			BlendAddf
#define BlendLinearBurnf 			BlendSubtractf
#define BlendAddf(base, blend) 		min(base + blend, 1.0)
#define BlendSubtractf(base, blend) 	max(base + blend - 1.0, 0.0)
#define BlendLightenf(base, blend) 		max(blend, base)
#define BlendDarkenf(base, blend) 		min(blend, base)
#define BlendLinearLightf(base, blend) 	(blend < 0.5 ? BlendLinearBurnf(base, (2.0 * blend)) : BlendLinearDodgef(base, (2.0 * (blend - 0.5))))
#define BlendScreenf(base, blend) 		(1.0 - ((1.0 - base) * (1.0 - blend)))
#define BlendOverlayf(base, blend) 	(base < 0.5 ? (2.0 * base * blend) : (1.0 - 2.0 * (1.0 - base) * (1.0 - blend)))
#define BlendSoftLightf(base, blend) 	((blend < 0.5) ? (2.0 * base * blend + base * base * (1.0 - 2.0 * blend)) : (sqrt(base) * (2.0 * blend - 1.0) + 2.0 * base * (1.0 - blend)))
#define BlendColorDodgef(base, blend) 	((blend == 1.0) ? blend : min(base / (1.0 - blend), 1.0))
#define BlendColorBurnf(base, blend) 	((blend == 0.0) ? blend : max((1.0 - ((1.0 - base) / blend)), 0.0))
#define BlendVividLightf(base, blend) 	((blend < 0.5) ? BlendColorBurnf(base, (2.0 * blend)) : BlendColorDodgef(base, (2.0 * (blend - 0.5))))
#define BlendPinLightf(base, blend) 	((blend < 0.5) ? BlendDarkenf(base, (2.0 * blend)) : BlendLightenf(base, (2.0 *(blend - 0.5))))
#define BlendHardMixf(base, blend) 	((BlendVividLightf(base, blend) < 0.5) ? 0.0 : 1.0)
#define BlendReflectf(base, blend) 		((blend == 1.0) ? blend : min(base * base / (1.0 - blend), 1.0))



/*
** Vector3 blending modes
*/

// Component wise blending
#define Blend1(base, blend, funcf) 		funcf(base, blend)
#define Blend3(base, blend, funcf) 		float3(funcf(base.r, blend.r), funcf(base.g, blend.g), funcf(base.b, blend.b))
#define Blend4(base, blend, funcf) 		float4(funcf(base.r, blend.r), funcf(base.g, blend.g), funcf(base.b, blend.b), funcf(base.a, blend.a))

#define BlendNormal(base, blend) 		(base)
#define BlendLighten				BlendLightenf
#define BlendDarken				BlendDarkenf
#define BlendMultiply(base, blend) 		(base * blend)
#define BlendAverage(base, blend) 		((base + blend) / 2.0)
#define BlendAdd(base, blend) 		min(base + blend, 1.0)
#define BlendSubtract(base, blend) 	max(base + blend - 1.0, 0.0)
#define BlendDifference(base, blend) 	abs(base - blend)
#define BlendNegation(base, blend) 	(1.0 - abs(1.0 - base - blend))
#define BlendExclusion(base, blend) 	(base + blend - 2.0 * base * blend)

#define BlendScreen1(base, blend) 		Blend1(base, blend, BlendScreenf)
#define BlendOverlay1(base, blend) 		Blend1(base, blend, BlendOverlayf)
#define BlendSoftLight1(base, blend) 	Blend1(base, blend, BlendSoftLightf)
#define BlendHardLight1(base, blend) 	BlendOverlay1(blend, base)
#define BlendColorDodge1(base, blend) 	Blend1(base, blend, BlendColorDodgef)
#define BlendColorBurn1(base, blend) 	Blend1(base, blend, BlendColorBurnf)
// Linear Light is another contrast-increasing mode
// If the blend color is darker than midgray, Linear Light darkens the image by decreasing the brightness. If the blend color is lighter than midgray, the result is a brighter image due to increased brightness.
#define BlendLinearLight1(base, blend) 	Blend1(base, blend, BlendLinearLightf)
#define BlendVividLight1(base, blend) 	Blend1(base, blend, BlendVividLightf)
#define BlendPinLight1(base, blend) 		Blend1(base, blend, BlendPinLightf)
#define BlendHardMix1(base, blend) 		Blend1(base, blend, BlendHardMixf)
#define BlendReflect1(base, blend) 		Blend1(base, blend, BlendReflectf)
#define BlendGlow1(base, blend) 		BlendReflect1(blend, base)


#define BlendScreen3(base, blend) 		Blend3(base, blend, BlendScreenf)
#define BlendOverlay3(base, blend) 		Blend3(base, blend, BlendOverlayf)
#define BlendSoftLight3(base, blend) 	Blend3(base, blend, BlendSoftLightf)
#define BlendHardLight3(base, blend) 	BlendOverlay3(blend, base)
#define BlendColorDodge3(base, blend) 	Blend3(base, blend, BlendColorDodgef)
#define BlendColorBurn3(base, blend) 	Blend3(base, blend, BlendColorBurnf)
// Linear Light is another contrast-increasing mode
// If the blend color is darker than midgray, Linear Light darkens the image by decreasing the brightness. If the blend color is lighter than midgray, the result is a brighter image due to increased brightness.
#define BlendLinearLight3(base, blend) 	Blend3(base, blend, BlendLinearLightf)
#define BlendVividLight3(base, blend) 	Blend3(base, blend, BlendVividLightf)
#define BlendPinLight3(base, blend) 	Blend3(base, blend, BlendPinLightf)
#define BlendHardMix3(base, blend) 		Blend3(base, blend, BlendHardMixf)
#define BlendReflect3(base, blend) 		Blend3(base, blend, BlendReflectf)
#define BlendGlow3(base, blend) 		BlendReflect3(blend, base)


#define BlendScreen4(base, blend) 		Blend4(base, blend, BlendScreenf)
#define BlendOverlay4(base, blend) 		Blend4(base, blend, BlendOverlayf)
#define BlendSoftLight4(base, blend) 	Blend4(base, blend, BlendSoftLightf)
#define BlendHardLight4(base, blend) 	BlendOverlay4(blend, base)
#define BlendColorDodge4(base, blend) 	Blend4(base, blend, BlendColorDodgef)
#define BlendColorBurn4(base, blend) 	Blend4(base, blend, BlendColorBurnf)
// Linear Light is another contrast-increasing mode
// If the blend color is darker than midgray, Linear Light darkens the image by decreasing the brightness. If the blend color is lighter than midgray, the result is a brighter image due to increased brightness.
#define BlendLinearLight4(base, blend) 	Blend4(base, blend, BlendLinearLightf)
#define BlendVividLight4(base, blend) 	Blend4(base, blend, BlendVividLightf)
#define BlendPinLight4(base, blend) 		Blend4(base, blend, BlendPinLightf)
#define BlendHardMix4(base, blend) 		Blend4(base, blend, BlendHardMixf)
#define BlendReflect4(base, blend) 		Blend4(base, blend, BlendReflectf)
#define BlendGlow4(base, blend) 		BlendReflect4(blend, base)


#define BlendLinearDodge			BlendAdd
#define BlendLinearBurn			BlendSubtract

#define BlendPhoenix(base, blend) 		(min(base, blend) - max(base, blend) + 1.0)


#define BlendOpacity(base, blend, F, O) 	(F(base, blend) * O + blend * (1.0 - O))

// Hue Blend mode creates the result color by combining the luminance and saturation of the base color with the hue of the blend color.
float BlendHue1(float base, float blend)
{
	return base;
}
float3 BlendHue3(float3 base, float3 blend)
{
	float3 baseHSL = RGBToHSL(base);
	return HSLToRGB(float3(RGBToHSL(blend).r, baseHSL.g, baseHSL.b));
}
float4 BlendHue4(float4 base, float4 blend)
{
	float3 hue = BlendHue3(base.xyz, blend.xyz);
	return float4(hue.x, hue.y, hue.z, BlendHue1(base.w, blend.w));
}

// Saturation Blend mode creates the result color by combining the luminance and hue of the base color with the saturation of the blend color.
float BlendSaturation1(float base, float blend)
{
	return base;
}
float3 BlendSaturation3(float3 base, float3 blend)
{
	float3 baseHSL = RGBToHSL(base);
	return HSLToRGB(float3(baseHSL.r, RGBToHSL(blend).g, baseHSL.b));
}
float4 BlendSaturation4(float4 base, float4 blend)
{
	float3 hue = BlendSaturation3(base.xyz, blend.xyz);
	return float4(hue.x, hue.y, hue.z, BlendSaturation1(base.w, blend.w));
}

// Color Mode keeps the brightness of the base color and applies both the hue and saturation of the blend color.
float BlendColor1(float base, float blend)
{
	return base;
}
float3 BlendColor3(float3 base, float3 blend)
{
	float3 blendHSL = RGBToHSL(blend);
	return HSLToRGB(float3(blendHSL.r, blendHSL.g, RGBToHSL(base).b));
}
float4 BlendColor4(float4 base, float4 blend)
{
	float3 hue = BlendColor3(base.xyz, blend.xyz);
	return float4(hue.x, hue.y, hue.z, BlendColor1(base.w, blend.w));
}



// Luminosity Blend mode creates the result color by combining the hue and saturation of the base color with the luminance of the blend color.
float BlendLuminosity1(float base, float blend)
{
	return base;
}
float3 BlendLuminosity3(float3 base, float3 blend)
{
	float3 baseHSL = RGBToHSL(base);
	return HSLToRGB(float3(baseHSL.r, baseHSL.g, RGBToHSL(blend).b));
}
float4 BlendLuminosity4(float4 base, float4 blend)
{
	float3 hue = BlendLuminosity3(base.xyz, blend.xyz);
	return float4(hue.x, hue.y, hue.z, BlendLuminosity1(base.w, blend.w));
}

//------------------------------------
// Interface for RTShader
//------------------------------------

void SGX_blend_normal(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendNormal(basePixel, blendPixel);
}
void SGX_blend_normal(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendNormal(basePixel, blendPixel);
}
void SGX_blend_normal(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendNormal(basePixel, blendPixel);
}


void SGX_blend_lighten(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendLighten(basePixel, blendPixel);
}
void SGX_blend_lighten(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendLighten(basePixel, blendPixel);
}
void SGX_blend_lighten(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendLighten(basePixel, blendPixel);
}

void SGX_blend_darken(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendDarken(basePixel, blendPixel);
}
void SGX_blend_darken(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendDarken(basePixel, blendPixel);
}
void SGX_blend_darken(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendDarken(basePixel, blendPixel);
}


void SGX_blend_multiply(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendMultiply(basePixel, blendPixel);
}
void SGX_blend_multiply(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendMultiply(basePixel, blendPixel);
}
void SGX_blend_multiply(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendMultiply(basePixel, blendPixel);
}


void SGX_blend_average(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendAverage(basePixel, blendPixel);
}
void SGX_blend_average(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendAverage(basePixel, blendPixel);
}
void SGX_blend_average(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendAverage(basePixel, blendPixel);
}


void SGX_blend_add(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendAdd(basePixel, blendPixel);
}
void SGX_blend_add(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendAdd(basePixel, blendPixel);
}
void SGX_blend_add(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendAdd(basePixel, blendPixel);
}


void SGX_blend_subtract(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendSubtract(basePixel, blendPixel);
}
void SGX_blend_subtract(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendSubtract(basePixel, blendPixel);
}
void SGX_blend_subtract(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendSubtract(basePixel, blendPixel);
}


void SGX_blend_difference(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendDifference(basePixel, blendPixel);
}
void SGX_blend_difference(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendDifference(basePixel, blendPixel);
}
void SGX_blend_difference(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendDifference(basePixel, blendPixel);
}


void SGX_blend_negation(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendNegation(basePixel, blendPixel);
}
void SGX_blend_negation(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendNegation(basePixel, blendPixel);
}
void SGX_blend_negation(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendNegation(basePixel, blendPixel);
}


void SGX_blend_exclusion(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendExclusion(basePixel, blendPixel);
}
void SGX_blend_exclusion(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendExclusion(basePixel, blendPixel);
}
void SGX_blend_exclusion(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendExclusion(basePixel, blendPixel);
}


void SGX_blend_screen(float4 basePixel, float4 blendPixel, out float4 oColor)
{	
	oColor = Blend4(basePixel, blendPixel, BlendScreenf);
}
void SGX_blend_screen(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendScreenf);
}
void SGX_blend_screen(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendScreenf);
}

void SGX_blend_overlay(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendOverlayf);
}
void SGX_blend_overlay(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendOverlayf);
}
void SGX_blend_overlay(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendOverlayf);
}

void SGX_blend_softLight(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendSoftLightf);
}
void SGX_blend_softLight(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendSoftLightf);
}
void SGX_blend_softLight(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendSoftLightf);
}


void SGX_blend_hardLight(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendOverlayf);
}
void SGX_blend_hardLight(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendOverlayf);
}
void SGX_blend_hardLight(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendOverlayf);
}


void SGX_blend_colorDodge(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendColorDodgef);
}
void SGX_blend_colorDodge(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendColorDodgef);
}
void SGX_blend_colorDodge(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendColorDodgef);
}


void SGX_blend_colorBurn(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendColorBurnf);
}
void SGX_blend_colorBurn(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendColorBurnf);
}
void SGX_blend_colorBurn(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendColorBurnf);
}


void SGX_blend_linearDodge(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendLinearDodge(basePixel, blendPixel);
}
void SGX_blend_linearDodge(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendLinearDodge(basePixel, blendPixel);
}
void SGX_blend_linearDodge(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendLinearDodge(basePixel, blendPixel);
}


void SGX_blend_linearBurn(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendLinearBurn(basePixel, blendPixel);
}
void SGX_blend_linearBurn(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendLinearBurn(basePixel, blendPixel);
}
void SGX_blend_linearBurn(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendLinearBurn(basePixel, blendPixel);
}


void SGX_blend_linearLight(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendLinearLightf);
}
void SGX_blend_linearLight(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendLinearLightf);
}
void SGX_blend_linearLight(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendLinearLightf);
}


void SGX_blend_vividLight(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendVividLightf);
}
void SGX_blend_vividLight(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendVividLightf);
}
void SGX_blend_vividLight(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendVividLightf);
}


void SGX_blend_pinLight(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendPinLightf);
}
void SGX_blend_pinLight(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendPinLightf);
}
void SGX_blend_pinLight(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendPinLightf);
}


void SGX_blend_hardMix(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendHardMixf);
}
void SGX_blend_hardMix(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendHardMixf);
}
void SGX_blend_hardMix(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendHardMixf);
}


void SGX_blend_reflect(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendReflectf);
}
void SGX_blend_reflect(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendReflectf);
}
void SGX_blend_reflect(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendReflectf);
}

void SGX_blend_glow(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = Blend4(basePixel, blendPixel, BlendReflectf);
}
void SGX_blend_glow(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = Blend3(basePixel, blendPixel, BlendReflectf);
}
void SGX_blend_glow(float basePixel, float blendPixel, out float oColor)
{
	oColor = Blend1(basePixel, blendPixel, BlendReflectf);
}


void SGX_blend_phoenix(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendPhoenix(basePixel, blendPixel);
}
void SGX_blend_phoenix(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendPhoenix(basePixel, blendPixel);
}
void SGX_blend_phoenix(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendPhoenix(basePixel, blendPixel);
}


void SGX_blend_saturation(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendSaturation4(basePixel, blendPixel);
}
void SGX_blend_saturation(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendSaturation3(basePixel, blendPixel);
}
void SGX_blend_saturation(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendSaturation1(basePixel, blendPixel);
}


void SGX_blend_color(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendColor4(basePixel, blendPixel);
}
void SGX_blend_color(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendColor3(basePixel, blendPixel);
}
void SGX_blend_color(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendColor1(basePixel, blendPixel);
}


void SGX_blend_luminosity(float4 basePixel, float4 blendPixel, out float4 oColor)
{
	oColor = BlendLuminosity4(basePixel, blendPixel);
}
void SGX_blend_luminosity(float3 basePixel, float3 blendPixel, out float3 oColor)
{
	oColor = BlendLuminosity3(basePixel, blendPixel);
}
void SGX_blend_luminosity(float basePixel, float blendPixel, out float oColor)
{
	oColor = BlendLuminosity1(basePixel, blendPixel);
}



