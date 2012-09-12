#version 120
varying vec2 uv;

uniform sampler2D blurred;
uniform sampler2D mrt0;
uniform sampler2D mrt1;

uniform float cLambda;

void main()
{
    float spacialImportance = texture2D(blurred, uv).w - texture2D(mrt1, uv).w;
    vec4 color = texture2D(mrt0, uv);
    if (spacialImportance < 0) // darkening only
    {
        gl_FragColor = vec4(color.rgb + (cLambda * spacialImportance), 1);
    } 
	else 
    {
        gl_FragColor = color;
    }
}