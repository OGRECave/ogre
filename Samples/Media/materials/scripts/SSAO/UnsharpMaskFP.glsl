#version 120
varying vec2 oUv0;

uniform sampler2D blurred;
uniform sampler2D mrt0;

uniform float cLambda;

void main()
{
    float spacialImportance = texture2D(blurred, oUv0).w - texture2D(mrt0, oUv0).w;
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