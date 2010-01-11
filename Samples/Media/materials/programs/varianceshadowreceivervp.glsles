/////////////////////////////////////////////////////////////////////////////////
//
// shadowreceivervp.cg
//
// Hamilton Chong
// (c) 2006
//
// This is an example vertex shader for shadow receiver objects.  
//
/////////////////////////////////////////////////////////////////////////////////

// Define inputs from application.
struct VertexIn
{
  float4 position       : POSITION;     // vertex position in object space
  float4 normal         : NORMAL;       // vertex normal in object space
};

// Define outputs from vertex shader.
struct Vertex
{
  float4 position       : POSITION;     // vertex position in post projective space
  float4 shadowCoord    : TEXCOORD0;    // vertex position in shadow map coordinates
  float  diffuse        : TEXCOORD1;    // diffuse shading value
};

Vertex main(VertexIn         In,
            uniform float4x4 uModelViewProjection,   // model-view-projection matrix
            uniform float4   uLightPosition,         // light position in object space
            uniform float4x4 uModel,                 // model matrix
            uniform float4x4 uTextureViewProjection  // shadow map's view projection matrix
            )
{
    Vertex Out;

    // compute diffuse shading
    float3 lightDirection = normalize(uLightPosition.xyz - In.position.xyz);
    Out.diffuse = dot(In.normal.xyz, lightDirection);

    // compute shadow map lookup coordinates
    Out.shadowCoord = mul(uTextureViewProjection, mul(uModel, In.position));

    // compute vertex's homogenous screen-space coordinates
    Out.position = mul(uModelViewProjection, In.position);

    return Out;
}
/////////////////////////////////////////////////////////////////////////////////
//
// shadowreceivervp.cg
//
// Hamilton Chong
// (c) 2006
//
// This is an example vertex shader for shadow receiver objects.  
//
/////////////////////////////////////////////////////////////////////////////////

// Define inputs from application.
struct VertexIn
{
  float4 position       : POSITION;     // vertex position in object space
  float4 normal         : NORMAL;       // vertex normal in object space
};

// Define outputs from vertex shader.
struct Vertex
{
  float4 position       : POSITION;     // vertex position in post projective space
  float4 shadowCoord    : TEXCOORD0;    // vertex position in shadow map coordinates
  float  diffuse        : TEXCOORD1;    // diffuse shading value
};

Vertex main(VertexIn         In,
            uniform float4x4 uModelViewProjection,   // model-view-projection matrix
            uniform float4   uLightPosition,         // light position in object space
            uniform float4x4 uModel,                 // model matrix
            uniform float4x4 uTextureViewProjection  // shadow map's view projection matrix
            )
{
    Vertex Out;

    // compute diffuse shading
    float3 lightDirection = normalize(uLightPosition.xyz - In.position.xyz);
    Out.diffuse = dot(In.normal.xyz, lightDirection);

    // compute shadow map lookup coordinates
    Out.shadowCoord = mul(uTextureViewProjection, mul(uModel, In.position));

    // compute vertex's homogenous screen-space coordinates
    Out.position = mul(uModelViewProjection, In.position);

    return Out;
}