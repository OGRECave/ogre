#if __VERSION__ < 330

#version 120

uniform sampler2D tex;

void main()
{
	vec2 texcoord = vec2( gl_TexCoord[0] );
	gl_FragColor = texture2D( tex, texcoord, 0.0 );
}

#else

#version 330

uniform sampler2D tex;

in vec2 uv0;
out vec4 fragColour;

void main()
{
    fragColour = texture2D( tex, uv0 );
}

#endif
