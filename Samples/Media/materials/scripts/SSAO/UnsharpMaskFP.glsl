#version 120
varying vec2 uv;

uniform sampler2D blurred;
uniform sampler2D mrt0;

uniform float cLambda;

void main()
{
    float spacialImportance = texture2D(blurred, uv).w - texture2D(mrt0, uv).w;
    vec4 color = vec4(1,1,1,1);
    if (spacialImportance < 0) // darkening only
    {
        gl_FragColor = vec4(color.rgb + (cLambda * spacialImportance), 1);
    } 
	else 
    {
        gl_FragColor = color;
    }
}