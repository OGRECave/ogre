#version 100

precision mediump int;
precision mediump float;

attribute vec4 vertex;
attribute vec4 normal;
attribute vec4 uv0;
attribute vec4 uv1; // pose1
attribute vec4 uv2; // pose2

varying vec2 oUv;
varying vec4 colour;

uniform mat4 worldViewProj;
uniform vec4 anim_t;

// hardware pose animation (no normals)
void main()
{
	// interpolate
    vec4 interp = vec4(vertex.xyz + anim_t.x*uv1.xyz + anim_t.y*uv2.xyz, 1.0);

	gl_Position = worldViewProj * interp;
	oUv = uv0.xy;
	colour = vec4(1.0,0.0,0.0,1.0);
}

