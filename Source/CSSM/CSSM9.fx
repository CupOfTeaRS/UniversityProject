//----------------------------------------------------------------------------
// Title:       Generic Shadow Mapping API based on CSSM - Example
//
// Author:      Ivica Kolic
// E-mail:      Ivica.Kolic@gmail.com
//
// Copyright (c) Ivica Kolic. All rights reserved.
//
// Description: API requires only two shader functions:
//              GetDepthAndProj     - used for creating shadow maps
//              GetDepthAndTexCoord - used for applying shadow maps
//----------------------------------------------------------------------------

//-----------------------------------------------------------------------------------------
// GENERIC SHADOW MAPPING API FUNCTIONS
// GENERIC SHADOW MAPPING API FUNCTIONS
// GENERIC SHADOW MAPPING API FUNCTIONS
//-----------------------------------------------------------------------------------------

// Shadow Mapping:
float4x4 _mProjMatrices[8];
float4x4 _mCtrlMatrices[8];
uniform uint _iProjNumber;

struct VS_OUTPUT_DEPTH
{
   float4 pos   : POSITION;
   float  depth : TEXCOORD0;
   float3 ctrl  : TEXCOORD1;
};

VS_OUTPUT_DEPTH GetDepthAndProj( float4 wPos, float idx )   // In DX10 idx is integet value
{
    VS_OUTPUT_DEPTH output;
    output.pos = mul( wPos, _mProjMatrices[idx] );
    output.ctrl.xy = mul( output.pos.xyw, (float3x3)_mCtrlMatrices[idx] ).xy;
    output.ctrl.z = output.depth = output.pos.z;
    output.pos.z = output.pos.w;
    return output;
}

float3 GetDepthAndTexCoord( float4 wPos, uniform uint projNumber ) // Uniform parameter is added.
{
    float3 ret=-100;
    [unroll(projNumber)]
    for( uint p=0; p<projNumber; p++ )
    {
        float4 pPos = mul( wPos, _mProjMatrices[p]);
        if( pPos.z >= 0 )
        {
            pPos.xyw /= pPos.w;
            float4 vCtrl = mul( pPos.xyww, _mCtrlMatrices[p] );
            if ( all(saturate(vCtrl)) )
            {
                ret.xyz = mad( pPos.xyz, float3(0.5f, -0.5f, 1.0f), float3(0.5f, 0.5f, 0.0f) );
                break;
            }
        }
    }
    return ret;
}
