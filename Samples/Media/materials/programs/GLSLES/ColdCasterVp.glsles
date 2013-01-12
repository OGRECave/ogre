#version 100

precision mediump int;
precision mediump float;

uniform mat4 worldViewProj;
uniform vec3 eyePosition;
attribute vec4 position;
attribute vec4 normal;
varying vec2 NDotV;

void main()
{
   vec4 eyeDir = vec4(eyePosition - position.xyz, 0.0);
   eyeDir = normalize(eyeDir);
   gl_Position = worldViewProj * position;
   NDotV = vec2(clamp( dot( normal, eyeDir ), 0.0, 1.0 ));
}
