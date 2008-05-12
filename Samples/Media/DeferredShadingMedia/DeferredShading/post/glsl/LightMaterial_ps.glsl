/******************************************************************************
Copyright (c) W.J. van der Laan

Permission is hereby granted, free of charge, to any person obtaining a copy of 
this software  and associated documentation files (the "Software"), to deal in 
the Software without restriction, including without limitation the rights to use, 
copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
Software, and to permit persons to whom the Software is furnished to do so, subject 
to the following conditions:

The above copyright notice and this permission notice shall be included in all copies 
or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, 
INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A 
PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION 
OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,OUT OF OR IN CONNECTION WITH THE 
SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
******************************************************************************/
/** Deferred shading framework
	// W.J. :wumpus: van der Laan 2005 //
	
	Post shader: Light geometry material
*/
uniform sampler2D tex0;
uniform sampler2D tex1;

varying vec2 texCoord;
varying vec3 projCoord;

// World view matrix to get object position in view space
uniform mat4 worldView;

// Attributes of light
uniform vec3 lightDiffuseColor;
uniform vec3 lightSpecularColor;
uniform vec3 lightFalloff;

void main()
{
	vec4 a0 = texture2D(tex0, texCoord); // Attribute 0: Diffuse color+shininess
    vec4 a1 = texture2D(tex1, texCoord); // Attribute 1: Normal+depth

    // Attributes
    vec3 colour = a0.rgb;
    float alpha = a0.a;		// Specularity
    float distance = a1.w;  // Distance from viewer (w)
    vec3 normal = a1.xyz;

	// Calculate position of texel in view space
    vec3 position = projCoord*distance;
	
	// Extract position in view space from worldView matrix
	//vec3 lightPos = vec3(worldView[0][3],worldView[1][3],worldView[2][3]);
	vec3 lightPos = vec3(worldView[3][0],worldView[3][1],worldView[3][2]);
	
    // Calculate light direction and distance
    vec3 lightVec = lightPos - position;
    float len_sq = dot(lightVec, lightVec);
    float len = sqrt(len_sq);
    vec3 lightDir = lightVec/len;
    
    /// Calculate attenuation    
    float attenuation = dot(lightFalloff, vec3(1, len, len_sq));
    
    /// Calculate diffuse colour
    vec3 light_diffuse = max(0.0,dot(lightDir, normal)) * lightDiffuseColor;
    
    /// Calculate specular component
    vec3 viewDir = -normalize(position);
    vec3 h = normalize(viewDir + lightDir);
    vec3 light_specular = pow(dot(normal, h),32.0) * lightSpecularColor;
    
    // Calcalate total lighting for this fragment
    vec3 total_light_contrib;
    total_light_contrib = light_diffuse;
	// Uncomment next line if specular desired
	//total_light_contrib += alpha * light_specular;
	
    gl_FragColor = vec4(total_light_contrib*colour/attenuation, 0);
    //gl_FragColor = vec4(1.0/attenuation, 0.0,0.0,0.0);
    //gl_FragColor = vec4(a1.xyz,  0.0);
}

