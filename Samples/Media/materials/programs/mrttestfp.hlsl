// MRT basic test
void mainfp_scene(
	float2 uv : TEXCOORD0,
	uniform sampler2D tex, 
	
	// 4 output textures
	// 2 RGBA
	// 2 float32_R
	out float4 outcolour0 : COLOR0,
	out float4 outcolour1 : COLOR1,
	out float4 outcolour2 : COLOR2,	
	out float4 outcolour3 : COLOR3
)
{
	float4 baseColour = tex2D(tex, uv);

	// first texture will be base
	outcolour0 = baseColour;

	// extract red for the second
	outcolour1 = baseColour * float4(1, 0, 0, 1);

	// greyscale positive for third
	outcolour2 = (baseColour.r + baseColour.g + baseColour.b) * 0.333;
	
	// greyscale inverted for third
	outcolour3 = 1.0 - outcolour2;
}

void mainfp_quad(
	float2 uv : TEXCOORD0,
	uniform sampler2D tex0 : register(s0),
	uniform sampler2D tex1 : register(s1),
	uniform sampler2D tex2 : register(s2),
	uniform sampler2D tex3 : register(s3),
	
	out float4 outcolour : COLOR0
)
{
	//outcolour = tex2D(tex0, uv);
	outcolour = tex2D(tex1, uv);
	//outcolour = tex2D(tex2, uv);
	//outcolour = tex2D(tex3, uv);
}




