#if __VERSION__ < 330

#version 120

uniform sampler2D tex;

void main()
{
	vec2 texcoord = vec2( gl_TexCoord[0] );
	gl_FragColor = texture2DLod( tex, texcoord, 0.0 );
}

#else

#version 330

uniform sampler2D tex;

in vec2 uv0;
out vec4 fragColour;

void main()
{
    fragColour = textureLod( tex, uv0, 0.0 );
}

#endif
