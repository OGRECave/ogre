#version 100
precision mediump int;
precision mediump float;

uniform sampler2D RT;
uniform sampler2D Sum;
uniform float blur;

varying vec2 uv0;

void main()
{
   vec4 render = texture2D(RT, uv0);
   vec4 sum = texture2D(Sum, uv0);

   gl_FragColor = mix(render, sum, blur);
}
