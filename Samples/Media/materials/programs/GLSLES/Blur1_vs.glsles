#version 100

precision mediump int;
precision mediump float;

varying vec2 texCoord;
uniform mat4 worldViewProj;
attribute vec2 uv0;
attribute vec4 vertex;

void main()                    
{
	gl_Position = worldViewProj * vertex;
	
	texCoord = uv0;
	
/*	const float size = 0.01;
	texCoord[1] = texCoord[0] + vec2(0.0, 1.0)*size;
	texCoord[2] = texCoord[0] + vec2(0.0, 2.0)*size;
	texCoord[3] = texCoord[0] + vec2(0.0, -1.0)*size;
	texCoord[4] = texCoord[0] + vec2(0.0, -2.0)*size;
*/}
