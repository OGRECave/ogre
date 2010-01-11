/////////////////////////////////////////////////////////////////////////////////
//
// shadowcastervp.cg
//
// Hamilton Chong
// (c) 2006
//
// This is an example vertex shader for shadow caster objects.  
//
/////////////////////////////////////////////////////////////////////////////////


// Define inputs from application.
struct VertexIn
{
  float4 position : POSITION;       // vertex position in object space
  float4 normal   : NORMAL;         // vertex normal in object space
};

// Define outputs from vertex shader.
struct VertexOut
{
  float4 position   : POSITION;     // post projection position coordinates
  float4 pos  	    : TEXCOORD0;    // ditto. Not all hardware allows access values bound to POSITION in fp.
  float4 normal     : TEXCOORD1;    // normal in object space (to be interpolated)
  float4 modelPos   : TEXCOORD2;    // position in object space (to be interpolated) 
};

VertexOut main( VertexIn         In,                   // vertex to process
                uniform float4x4 uModelViewProjection  // model-view-projection matrix
              )
{
    VertexOut Out;   // output data
    
    // Transform vertex position into post projective (homogenous screen) space.
    Out.position = mul(uModelViewProjection, In.position);
    Out.pos      = mul(uModelViewProjection, In.position);

    // copy over data to interpolate using perspective correct interpolation
    Out.normal = float4(In.normal.x, In.normal.y, In.normal.z, 0.0);
    Out.modelPos = In.position;

    return Out;
}
/////////////////////////////////////////////////////////////////////////////////
//
// shadowcastervp.cg
//
// Hamilton Chong
// (c) 2006
//
// This is an example vertex shader for shadow caster objects.  
//
/////////////////////////////////////////////////////////////////////////////////


// Define inputs from application.
struct VertexIn
{
  float4 position : POSITION;       // vertex position in object space
  float4 normal   : NORMAL;         // vertex normal in object space
};

// Define outputs from vertex shader.
struct VertexOut
{
  float4 position   : POSITION;     // post projection position coordinates
  float4 pos  	    : TEXCOORD0;    // ditto. Not all hardware allows access values bound to POSITION in fp.
  float4 normal     : TEXCOORD1;    // normal in object space (to be interpolated)
  float4 modelPos   : TEXCOORD2;    // position in object space (to be interpolated) 
};

VertexOut main( VertexIn         In,                   // vertex to process
                uniform float4x4 uModelViewProjection  // model-view-projection matrix
              )
{
    VertexOut Out;   // output data
    
    // Transform vertex position into post projective (homogenous screen) space.
    Out.position = mul(uModelViewProjection, In.position);
    Out.pos      = mul(uModelViewProjection, In.position);

    // copy over data to interpolate using perspective correct interpolation
    Out.normal = float4(In.normal.x, In.normal.y, In.normal.z, 0.0);
    Out.modelPos = In.position;

    return Out;
}