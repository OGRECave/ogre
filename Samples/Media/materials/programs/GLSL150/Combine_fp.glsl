#version 150

uniform sampler2D RT;
uniform sampler2D Sum;
uniform float blur;

in vec2 uv;
out vec4 fragColour;

void main()
{
   vec4 render = texture(RT, uv);
   vec4 sum = texture(Sum, uv);

   fragColour = mix(render, sum, blur);
}
