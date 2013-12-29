#version 150

in vec4 vertex;
in vec3 normal;
in vec4 uv0;
in vec4 uv1; // pos2
in vec4 uv2; // normal2

out vec4 colour;
// out vec2 oUv;

uniform mat4 worldViewProj;
uniform vec4 anim_t;
uniform vec4 objSpaceLightPos;
uniform vec4 ambient;

// hardware morph animation (with normals)
void main()
{
    // interpolate position
    vec4 posinterp = vec4(vertex.xyz + anim_t.x*(uv1.xyz - vertex.xyz), 1.0);

    // nlerp normal
    vec3 ninterp = normal + anim_t.x*(uv2.xyz - normal);
    ninterp = normalize(ninterp);

    gl_Position = worldViewProj * posinterp;
    // oUv = uv0.xy;

    vec3 lightDir = normalize(
        objSpaceLightPos.xyz - (posinterp.xyz * objSpaceLightPos.w));

    // Colour it red to make it easy to identify
    float lit = clamp(dot(lightDir, ninterp), 0.0, 1.0);
    colour = vec4((ambient.rgb + vec3(lit,lit,lit)) * vec3(1.0,0.0,0.0), 1.0);
}
