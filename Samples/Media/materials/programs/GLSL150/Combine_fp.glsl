#version 150

uniform sampler2D RT;
uniform sampler2D Sum;
uniform float blur;

in vec2 oUv0;
out vec4 fragColour;

void main()
{
   vec4 render = texture(RT, oUv0);
   vec4 sum = texture(Sum, oUv0);

   fragColour = mix(render, sum, blur);
}
