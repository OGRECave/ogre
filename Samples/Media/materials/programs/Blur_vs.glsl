void main()                    
{
	vec2 inPos = sign(gl_Vertex.xy);
	gl_Position = vec4(inPos.xy, 0.0, 1.0);

	vec2 uv = (vec2(inPos.x, -inPos.y) + 1.0)*0.5;
	gl_TexCoord[0]  = vec4( uv, 0, 0 );
	
	const float size = 0.01;
	gl_TexCoord[1] = vec4( uv + vec2(0.0, 1.0)*size, 0, 0);
	gl_TexCoord[2] = vec4( uv  + vec2(0.0, 2.0)*size, 0, 0);
	gl_TexCoord[3] = vec4( uv  + vec2(0.0, -1.0)*size, 0, 0);
	gl_TexCoord[4] = vec4( uv  + vec2(0.0, -2.0)*size, 0, 0);
}
