uniform mat4 worldViewProj;
attribute vec2 uv0;

void main()                    
{
	gl_Position = worldViewProj * gl_Vertex;

	gl_TexCoord[0]  = vec4( uv0, 0, 0 );
	
	const float size = 0.01;
	gl_TexCoord[1] = vec4( uv0 + vec2(0.0, 1.0)*size, 0, 0);
	gl_TexCoord[2] = vec4( uv0  + vec2(0.0, 2.0)*size, 0, 0);
	gl_TexCoord[3] = vec4( uv0  + vec2(0.0, -1.0)*size, 0, 0);
	gl_TexCoord[4] = vec4( uv0  + vec2(0.0, -2.0)*size, 0, 0);
}
