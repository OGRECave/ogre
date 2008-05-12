sampler Blur1: register(s1);
sampler RT: register(s0);


float luminance(float3 c)
{
	return dot( c, float3(0.3, 0.59, 0.11) );
}

float4 main(float2 texCoord: TEXCOORD0) : COLOR {
	float4 sharp = tex2D(RT,   texCoord);
	float4 blur  = tex2D(Blur1, texCoord);



	return ( sharp + blur * 1.8 ) / 2;

//	float4 color	= lerp( sharp, blur, 0.4f );

//	return color;


/*
	return ( sharp + blur * 1.8 ) / 2 +
			 luminance(blur) * 
			 float4( 0.5, 0.5, 0.5, 0) +
			 luminance(sharp) *
			 float4( 0.3, 0.3, 0.3, 0);
*/
/*
	return ( sharp + blur * 0.9) / 2 +
			 luminance(blur) * float4(0.1, 0.15, 0.7, 0);
*/

/*
	return ( sharp + blur * 0.9) / 2 +
			 luminance(blur) * float4(0.1, 0.15, 0.7, 0);
*/

//	float4 retColor = luminance( sharp ) +
//							luminance( blur ) + blur / 2;
//	return retColor;
}









