#version 100

precision mediump int;
precision mediump float;

varying vec2 texCoord[5];

attribute vec4 vertex;
attribute vec2 uv0;
uniform mat4 worldViewProj;

void main()                    
{
	gl_Position = worldViewProj * vertex;
	
	texCoord[0]  = uv0;
	
	const float size = 0.01;
	texCoord[1] = texCoord[0] + vec2(1.0, 0.0)*size;
	texCoord[2] = texCoord[0] + vec2(2.0, 0.0)*size;
	texCoord[3] = texCoord[0] + vec2(-1.0, 0.0)*size;
	texCoord[4] = texCoord[0] + vec2(-2.0, 0.0)*size;
}
