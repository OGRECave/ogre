#version 120

uniform vec3 lightDiffuse;
uniform vec3 lightSpecular;
uniform vec4 scaleBias;

uniform sampler2D normalHeightMap;
uniform sampler2D diffuseMap;

varying vec3 oEyeDir;
varying vec3 oLightDir;
varying vec3 oHalfAngle;
varying vec4 oUv0;

// Expand a range-compressed vector
vec3 expand(vec3 v)
{
    return (v - 0.5) * 2.0;
}

void main()
{
    // Get the height using the tex coords
    float height = texture2D(normalHeightMap, oUv0.xy).a;

    // Calculate displacement
    float displacement = (height * scaleBias.x) + scaleBias.y;

    vec3 uv2 = vec3(oUv0.xy, 1.0);

    // calculate the new tex coord to use for normal and diffuse
    vec2 newTexCoord = ((oEyeDir * displacement) + uv2).xy;

    // get the new normal and diffuse values
    vec3 normal = expand(texture2D(normalHeightMap, newTexCoord).xyz);
    vec3 diffuse = texture2D(diffuseMap, newTexCoord).xyz;
    vec3 specular = pow(clamp(dot(normal, oHalfAngle), 0.0, 1.0), 32.0) * lightSpecular;

    vec3 col = diffuse * (clamp(dot(normal, oLightDir), 0.0, 1.0) * lightDiffuse) + specular;
    gl_FragColor = vec4(col, 1.0);
}
