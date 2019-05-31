//----------------------------------------------------------------------------
// Title:       GENERIC SHADOW MAPPING API BASED ON CAMERA SPACE SHADOW MAPS
//
// Author:      Ivica Kolic
// E-mail:      Ivica.Kolic@gmail.com
//
// Copyright (c) Ivica Kolic. All rights reserved.
//
// Description: Complete shadow mapping solution.
//              Only ONE shadow map is required (no need for
//              accompanying depth/stencil buffer).
//              HIGHEST QUALITY (no wastage) shadow maps are created in a 
//              SINGLE PASS and solution works for EVERY TYPE of LIGHT/CAMERA.
//----------------------------------------------------------------------------

#ifndef __CSSM_H__
#define __CSSM_H__

#ifdef CSSM_API_EXPORTS
#define CSSM_API __declspec(dllexport)
#else
#define CSSM_API __declspec(dllimport)
#endif

//-----------------------------------------------------------------------
// Function:    GetCSSMShaderParams
// Description: Main shadow mapping function that takes light and camera
//              information and returns optimal shadow mapping parameters.
// Input:       light information, camera information, focus range.
// Output:      Function sets outProjMatrixArray, outCtrlMatrixArray
//              and outPlaneNumber parameters.
// FocusRange is the only variable that controles shadow map quality 
// (other than shadow map size). It defines distance for which half of 
// shadow map will be used.
// For FPS games, this parameter should be 15-20 meters.
// For racing simulations 50-100 meters. For flight simulations even more.
// In a mixed style game, focus range could be calculated as:
//           FocusRange = min( 20m, distance_from_the_ground )
//-----------------------------------------------------------------------

extern "C" CSSM_API void GetOptimalCSSMShaderParams(
     float*              outProjMatrixArray
    ,float*              outCtrlMatrixArray
    ,int*                outProjNumber
     // LIGHT INFO:
    ,const float*        inLightPos
    ,const float*        inLightDir
    ,float               inLightRange
    ,float               inLightAngle
    // CAMERA INFO:
    ,const float*        inViewMatrix
    ,const float*        inProjMatrix
    // CSSM INFO:
    ,float               inFocusRange
    ,bool                columnMajorMatrices=false // Used for OpenGL
);

//-----------------------------------------------------------------------
// Function:    IsShadowVisible
// Description: Function returns TRUE if shadow of the object is visible.
//              Function is not thread safe since it uses last parameters
//              passed to GetOptimalCSSMShaderParams function.
//              Purpose of this function is to additionaly speed up
//              shadow rendering by limitting work on relevant objects.
// Input:       3-dim vector of object position in world space, and
//              object radius.
// Output:      TRUE/FALSE shadow visibility information.
// NOTE:        This function is NOT IMPLEMENTED, but it will be soon.
//-----------------------------------------------------------------------

bool IsShadowVisible( const float* objectPos, float objectRadius ); // NOT IMPLEMENTED YET - DO NOT USE

//-----------------------------------------------------------------------------------------
// SHADER IMPLEMENTATION - only two functions. One for creating, and other for applying SM.
//-----------------------------------------------------------------------------------------
/*

cbuffer cbShadowMappingParams
{
    matrix _mProjMatrices[8];
    matrix _mCtrlMatrices[8];
    uniform int _iProjNumber;
};

// CREATING SHADOW MAP (sample with DX10 instancing):

struct VS_OUTPUT_DEPTH
{
   float  depth : TEXCOORD0;
   float4 pos   : SV_POSITION0;
   float3 ctrl  : SV_ClipDistance0;
};

// This function is used in vertex shader for creating shadow map.
// Input is world position and instance index.
// Output is projected position, depth and clipping.
VS_OUTPUT_DEPTH GetDepthAndProj( float4 wPos, uint idx )
{
    VS_OUTPUT_DEPTH output;
    output.pos = mul( wPos, _mProjMatrices[idx] );
    output.ctrl.xy = mul( output.pos.xyw, (float3x3)_mCtrlMatrices[idx] ).xy;
    output.ctrl.z = output.depth = output.pos.z;
    return output;
}

// APPLYING SHADOW MAP:

// This function should be used in pixel shader when shadow map is applyed.
// Input is world postion.
// Output is shadow map coordinates (x,y) and current depth (z).
// With this output, pixel shader should do something like this:
//     clip( _tShadowMap.Sample(SamBorder, ret.xy).r + DEPTH_BIAS - ret.z );

float3 GetDeptAndTexCoord( float4 wPos )
{
    float3 ret=-100;
    for( int p=_iProjNumber-1; p>=0; p-- )
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

// Only these two functions are part of the API (and cBuffer).






// SAMPLE SHADERS USING THESE FUNCTIONS:
// SAMPLE SHADERS USING THESE FUNCTIONS:
// SAMPLE SHADERS USING THESE FUNCTIONS:

BlendState BlendMin // Maintain minimal depth
{
    SrcBlend       = ONE;
    DestBlend      = ONE;
    BlendOp        = MIN;
};

VS_OUTPUT_DEPTH VertexDepth( VS_P_N_T v, uint idx:SV_InstanceID )
{
    return GetDepthAndProj( mul( v.pos, _mWorldMatrix ), idx);
    // Note: if you are using bit masking (for rendering vegetation),
    //       output shoud also contain texture coordinate, and pixel shader should
    //       clip-out transparent areas. Then appended output vertex structure should be used.
}

float4 PixelDepth( const VS_OUTPUT_DEPTH input ) : SV_TARGET
{
    return input.depth;
}

// Technique used for creating shadow map:
technique10 LightDepth
{
    pass p0
    {
        SetVertexShader(   CompileShader( vs_4_0, VertexDepth() ) );
        SetGeometryShader( NULL );
        SetPixelShader(    CompileShader( ps_4_0, PixelDepth() ) );
        SetDepthStencilState(DepthDisabled, 0);                                // No need for depth buffer
        SetBlendState(BlendMin, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF); // Maintain min value 
    }
}

//*/

#endif
