/*
----------------------------------------------------------
Island Tessellation sample from NVIDIA's DirectX 11 SDK:
http://developer.nvidia.com/nvidia-graphics-sdk-11-direct3d
----------------------------------------------------------
*/

cbuffer QuadObject
{
    static const float2 QuadVertices[4] =
    {
        {-1.0, -1.0},
        { 1.0, -1.0},
        {-1.0,  1.0},
        { 1.0,  1.0}
    };

    static const float2 QuadTexCoordinates[4] =
    {
        {0.0, 1.0},
        {1.0, 1.0},
        {0.0, 0.0},
        {1.0, 0.0}
    };
}

SamplerState SamplerPointClamp
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState SamplerLinearClamp
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Clamp;
    AddressV = Clamp;
};

SamplerState SamplerLinearWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

SamplerState SamplerAnisotropicWrap
{
    Filter = ANISOTROPIC;
    AddressU = Wrap;
    AddressV = Wrap;
	MaxAnisotropy = 16;
};

SamplerState SamplerCube
{
    Filter = MIN_MAG_MIP_POINT;
    AddressU = Clamp;
    AddressV = Clamp;
	AddressW = Clamp;
};

SamplerState SamplerLinearMirror
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Mirror;
    AddressV = Mirror;
};

SamplerState SamplerLinearBorderBlack
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Border;
    AddressV = Border;
    AddressW = Border;
    BorderColor = float4(0, 0, 0, 0);
};

SamplerComparisonState SamplerBackBufferDepth
{
    Filter = COMPARISON_MIN_MAG_LINEAR_MIP_POINT;
    AddressU = Border;
    AddressV = Border;
    BorderColor = float4(1, 1, 1, 1);
    ComparisonFunc = LESS_EQUAL;
};

SamplerComparisonState SamplerDepthAnisotropic
{
    Filter = COMPARISON_ANISOTROPIC;
    AddressU = Border;
    AddressV = Border;
    ComparisonFunc = LESS;
    BorderColor = float4(1, 1, 1, 1);
    MaxAnisotropy = 16;
};

float4 ColorPS(uniform float4 color) : SV_Target
{
    return color;
}