#version 150

void main()																					
{																							
	//Transform the vertex (ModelViewProj matrix)											
	gl_Position = ftransform();																
}
