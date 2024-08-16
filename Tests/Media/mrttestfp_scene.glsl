// Need to enable the draw buffers extension
#extension GL_ARB_draw_buffers : enable

uniform sampler2D tex0;

void main()                    
{
	vec4 baseColour = texture2D(tex0, gl_TexCoord[0].xy);

	gl_FragData[0] = baseColour;

	gl_FragData[1] = baseColour * vec4(1, 0, 0, 1);

	float abs = (baseColour.r + baseColour.g + baseColour.b) * 0.333;
	gl_FragData[2] = vec4(abs, abs, abs, 1);
	
	float invabs = 1.0 - abs;
	gl_FragData[3] = vec4(invabs, invabs, invabs, 1);
}

