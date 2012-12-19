#version 150

uniform sampler2D RT;
in vec2 uv;
out vec4 fragColour;

void main()
{
    vec4 Color;
    Color.a = 1.0;
    Color.rgb = vec3(0.5);
    Color -= texture( RT, uv - 0.001)*2.0;
    Color += texture( RT, uv + 0.001)*2.0;
    Color.rgb = vec3((Color.r+Color.g+Color.b)/3.0);
    fragColour = Color;
}
