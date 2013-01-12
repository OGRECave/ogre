#version 100

precision mediump int;
precision mediump float;

attribute vec4 position;
uniform mat4 worldViewProj;

void main()																					
{																							
	//Transform the vertex (ModelViewProj matrix)											
	gl_Position = position * worldViewProj;
}
