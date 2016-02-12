cbuffer ViewProj
{
    matrix viewMat;
    matrix projMat;
};

struct VsOut
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

VsOut main(float4 pos : POSITION, float4 color : COLOR)
{
    VsOut vsOut;
    vsOut.pos = mul(pos, viewMat);
    vsOut.pos = mul(vsOut.pos, projMat);
    vsOut.color = color;
    return vsOut;
}