//--------------------------------------------------------------------------------------
//	File: Materials.fx
//
//	A range of shaders/techniques to illustrate material-based render methods
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Global Variables
//--------------------------------------------------------------------------------------

// Matrices for transforming model vertices from model space -> world space -> camera space -> 2D projected vertices
float4x4 WorldMatrix;
float4x4 ViewMatrix;
float4x4 ProjMatrix;
//Shadow  matrices
float4x4 ShadowViewMatrix;
float4x4 ShadowProjMatrix;
// Camera position (needed for specular lighting at least)
float3 CameraPos;
float3 ShadowCameraPos;
// Light data
float3 Light1Pos;
float3 Light2Pos;
float3 Light1Colour;
float3 Light2Colour;
float3 AmbientColour;

// Material colour data
float4 DiffuseColour;
float4 SpecularColour;
float  SpecularPower;
//Fog start and finish 
float FogStart;
float FogEnd;
//Fog enable
bool FogEnable;
// Texture maps
Texture2D DiffuseMap;
Texture2D DiffuseMap2; // Second diffuse map for special techniques (currently unused)
Texture2D DepthMap;
Texture2D NormalMap;
Texture2D ShadowMap; // shadow map
//Variables for godrays
//NOT USED CURRENTLY
float GodrayDensity;
float GodrayWeight;
float GodrayDecay;
float GodrayExposure;
float3 GodrayLightPosition;
Texture2D GodrayOcclusionMap;

//Godray variables
float GodrayStart;
float GodrayWidth;
int FlipHorizontal;
static const int NUM_SAMPLES = 100;
// Samplers to use with the above texture maps. Specifies texture filtering and addressing mode to use when accessing texture pixels
SamplerState TrilinearWrap
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};
SamplerState PointClamp
{
	Filter = MIN_MAG_MIP_POINT;
	AddressU = Clamp;
	AddressV = Clamp;
};
SamplerState GradientSample
{
	Filter = ANISOTROPIC;
	MaxAnisotropy = 8;
	AddressU = Wrap;
	AddressV = Wrap;
};
SamplerState GodraysSample
{
	Filter = MIN_MAG_MIP_LINEAR;
	AddressU = Border;
	AddressV = Border;
};
//Other
float ParallaxDepth; // A factor to strengthen/weaken the parallax effect. Cannot exaggerate it too much or will get distortion


//--------------------------------------------------------------------------------------
// Structures
//--------------------------------------------------------------------------------------

// Standard vertex data to be sent into the vertex shader
struct VS_INPUT
{
    float3 Pos     : POSITION;
    float3 Normal  : NORMAL;
	float2 UV      : TEXCOORD0;
};

// Input vertex data with additional tangents for normal mapping
struct VS_NORMALMAP_INPUT
{
    float3 Pos     : POSITION;
    float3 Normal  : NORMAL;
    float3 Tangent : TANGENT;
	float2 UV      : TEXCOORD0;
};
struct VS_FOG_BASIC_OUTPUT
{
	float4 ProjPos : SV_POSITION;
	float2 UV : TEXCOORD0;
	float fogFactor : FOG;
};


// Minimum vertex shader output 
struct VS_BASIC_OUTPUT
{
    float4 ProjPos       : SV_POSITION;
};


// Vertex shader output for texture only
struct VS_TEX_OUTPUT
{
    float4 ProjPos       : SV_POSITION;
    float2 UV            : TEXCOORD0;
	float fogFactor : FOG;
};
//For anisotropic filtering testing
struct VS_TEX_OUTPUT_ANISO
{
	float4 ProjPos       : SV_POSITION;
	float2 UV            : TEXCOORD0;
};

// Vertex shader output for pixel lighting with no texture
struct VS_LIGHTING_OUTPUT
{
    float4 ProjPos       : SV_POSITION;
	float3 WorldPos      : POSITION;
	float3 WorldNormal   : NORMAL;
	float fogFactor : FOG;
};

// Vertex shader output for pixel lighting with a texture
struct VS_LIGHTINGTEX_OUTPUT
{
    float4 ProjPos       : SV_POSITION;  // 2D "projected" position for vertex (required output for vertex shader)
	float3 WorldPos      : POSITION;
	float3 WorldNormal   : NORMAL;
    float2 UV            : TEXCOORD0;
	float fogFactor : FOG;
};

// Vertex shader output for normal mapping
struct VS_NORMALMAP_OUTPUT
{
	float4 ProjPos      : SV_POSITION;
	float3 WorldPos     : POSITION;
	float3 ModelNormal  : NORMAL;
	float3 ModelTangent : TANGENT;
	float2 UV           : TEXCOORD0;
};
struct VS_GODRAYS_OUTPUT
{
	float4 ProjPos: SV_POSITION;
	float2 UV : TEXCOORD0;
	float4 ScreenCoords: TEXCOORD1; // we will copy ProjPos into this
};

struct PS_PARTICLE_INPUT
{
    float4 ProjPos : SV_Position;
    float2 UV : TEXCOORD0;
    float Alpha : TEXCOORD1;
};

struct PS_PARTICLE_OUTPUT
{
    float4 Colour : SV_Target;
    float Depth : SV_Depth;
};

//--------------------------------------------------------------------------------------
// Vertex Shaders
//--------------------------------------------------------------------------------------

// Basic vertex shader to transform 3D model vertices to 2D only
//
VS_BASIC_OUTPUT VSTransformOnly( VS_INPUT vIn )
{
	VS_BASIC_OUTPUT vOut;
	
	// Transform the input model vertex position into world space, then view space, then 2D projection space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul( modelPos, WorldMatrix );
	float4 viewPos  = mul( worldPos, ViewMatrix );
	vOut.ProjPos    = mul( viewPos,  ProjMatrix );

	return vOut;
}
VS_FOG_BASIC_OUTPUT VSFogVertexShader(VS_INPUT vIn)
{
	VS_FOG_BASIC_OUTPUT vOut;
	

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;
	float4 cameraPosition;
	cameraPosition = mul(float4(vIn.Pos, 1.0f), WorldMatrix);
	cameraPosition = mul(cameraPosition, ViewMatrix);

	vOut.fogFactor = saturate((FogEnd - cameraPosition.z) / (FogEnd - FogStart));
	return vOut;

}

VS_TEX_OUTPUT VSBasicTransform(VS_INPUT vIn)
{
	VS_TEX_OUTPUT vOut;

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;

	return vOut;
}
VS_TEX_OUTPUT VSShadowTransform(VS_INPUT vIn)
{
	VS_TEX_OUTPUT vOut;

	// Use world matrix passed from C++ to transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;

	vOut.UV.y -= vOut.UV.y;
	return vOut;
}

// Basic vertex shader to transform 3D model vertices to 2D and pass UVs to the pixel shader
//
VS_TEX_OUTPUT VSTransformTex( VS_INPUT vIn )
{
	VS_TEX_OUTPUT vOut;
	
	// Transform the input model vertex position into world space, then view space, then 2D projection space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul( modelPos, WorldMatrix );
	float4 viewPos  = mul( worldPos, ViewMatrix );
	vOut.ProjPos    = mul( viewPos,  ProjMatrix );
	
	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;
	float4 cameraPosition;
	cameraPosition = mul(float4(vIn.Pos, 1.0f), WorldMatrix);
	cameraPosition = mul(cameraPosition, ViewMatrix);

	vOut.fogFactor = saturate((FogEnd - cameraPosition.z) / (FogEnd - FogStart));

	return vOut;
}

VS_TEX_OUTPUT VSTransformTexRandomTex(VS_INPUT vIn)
{
	VS_TEX_OUTPUT vOut;

	// Transform the input model vertex position into world space, then view space, then 2D projection space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);

	// Pass texture coordinates (UVs) on to the pixel shader
	vOut.UV = vIn.UV;
	
	if (GodrayDensity > 500)
	{
		float temp;
		temp = vOut.UV.y;
		vOut.UV.y = vOut.UV.x;
		vOut.UV.x = temp;
	}
	 
	

	return vOut;
}

// Standard vertex shader for pixel-lit untextured models
//
VS_LIGHTING_OUTPUT VSPixelLit( VS_INPUT vIn )
{
	VS_LIGHTING_OUTPUT vOut;

	// Add 4th element to position and normal (needed to multiply by 4x4 matrix. Recall lectures - set 1 for position, 0 for vector)
	float4 modelPos = float4(vIn.Pos, 1.0f);
	float4 modelNormal = float4(vIn.Normal, 0.0f);

	// Transform model vertex position and normal to world space
	float4 worldPos    = mul( modelPos,    WorldMatrix );
	float3 worldNormal = mul( modelNormal, WorldMatrix );

	// Pass world space position & normal to pixel shader for lighting calculations
   	vOut.WorldPos    = worldPos.xyz;
	vOut.WorldNormal = worldNormal;

	// Use camera matrices to further transform the vertex from world space into view space (camera's point of view) and finally into 2D "projection" space for rendering
	float4 viewPos  = mul( worldPos, ViewMatrix );
	vOut.ProjPos    = mul( viewPos,  ProjMatrix );

	float4 cameraPosition;
	cameraPosition = mul(float4(vIn.Pos, 1.0f), WorldMatrix);
	cameraPosition = mul(cameraPosition, ViewMatrix);

	vOut.fogFactor = saturate((FogEnd - cameraPosition.z) / (FogEnd - FogStart));
	return vOut;
}

// Standard vertex shader for pixel-lit textured models
//
VS_LIGHTINGTEX_OUTPUT VSPixelLitTex( VS_INPUT vIn )
{
	VS_LIGHTINGTEX_OUTPUT vOut;

	// Add 4th element to position and normal (needed to multiply by 4x4 matrix. Recall lectures - set 1 for position, 0 for vector)
	float4 modelPos = float4(vIn.Pos, 1.0f);
	float4 modelNormal = float4(vIn.Normal, 0.0f);

	// Transform model vertex position and normal to world space
	float4 worldPos    = mul( modelPos,    WorldMatrix );
	float3 worldNormal = mul( modelNormal, WorldMatrix );

	// Pass world space position & normal to pixel shader for lighting calculations
   	vOut.WorldPos    = worldPos.xyz;
	vOut.WorldNormal = worldNormal;

	// Use camera matrices to further transform the vertex from world space into view space (camera's point of view) and finally into 2D "projection" space for rendering
	float4 viewPos  = mul( worldPos, ViewMatrix );
	vOut.ProjPos    = mul( viewPos,  ProjMatrix );

	// Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
	vOut.UV = vIn.UV;

	float4 cameraPosition;
	cameraPosition = mul(float4(vIn.Pos, 1.0f), WorldMatrix);
	cameraPosition = mul(cameraPosition, ViewMatrix);

	vOut.fogFactor = saturate((FogEnd - cameraPosition.z) / (FogEnd - FogStart));

	return vOut;
}


// Vertex shader for normal-mapped models
//
VS_NORMALMAP_OUTPUT VSNormalMap( VS_NORMALMAP_INPUT vIn )
{
	VS_NORMALMAP_OUTPUT vOut;

	//Transform the input model vertex position into world space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul( modelPos, WorldMatrix );
	vOut.WorldPos = worldPos.xyz;

	// Transform the vertex from world space into view space and finally into 2D "projection" space for rendering
	float4 viewPos = mul( worldPos, ViewMatrix );
	vOut.ProjPos   = mul( viewPos,  ProjMatrix );

	// Just send the model's normal and tangent untransformed (in model space). The pixel shader will do the matrix work on normals
	vOut.ModelNormal  = vIn.Normal;
	vOut.ModelTangent = vIn.Tangent;

	// Pass texture coordinates (UVs) on to the pixel shader, the vertex shader doesn't need them
	vOut.UV = vIn.UV;

	return vOut;
}

VS_GODRAYS_OUTPUT VSGodrays(VS_INPUT vIn)
{
	VS_GODRAYS_OUTPUT vOut;
	// Transform the input model vertex position into world space, then view space, then 2D projection space
	float4 modelPos = float4(vIn.Pos, 1.0f); // Promote to 1x4 so we can multiply by 4x4 matrix, put 1.0 in 4th element for a point (0.0 for a vector)
	float4 worldPos = mul(modelPos, WorldMatrix);
	float4 viewPos = mul(worldPos, ViewMatrix);
	vOut.ProjPos = mul(viewPos, ProjMatrix);
	vOut.ScreenCoords = vOut.ProjPos;
	const float2 uvCenter = float2(0.5, 0.5);
	float2 nuv = vIn.UV - uvCenter;
	vOut.UV = float2(max(0.0f, GodrayStart) * nuv.x, max(0.0f, GodrayStart) * nuv.y) + uvCenter;
	return vOut;
}

//--------------------------------------------------------------------------------------
// Pixel Shaders
//--------------------------------------------------------------------------------------

// A pixel shader that just outputs the diffuse material colour
//
float4 PSPlainColour( VS_BASIC_OUTPUT vOut ) : SV_Target
{
	return DiffuseColour;
}
//Shadow Map PS
float4 PSPixelDepth(VS_TEX_OUTPUT vOut) : SV_Target
{
    float4 diffuseTex = DiffuseMap.Sample(TrilinearWrap, vOut.UV);
    
        return (vOut.ProjPos.z / vOut.ProjPos.w) * 1000;
}
//Fog PS
float4 PSFogPixelShader(VS_FOG_BASIC_OUTPUT vOut) : SV_TARGET
{
	float4 diffuseMapColour = DiffuseMap.Sample(TrilinearWrap, vOut.UV);
	float4 fogColour = float4(0.5f, 0.5f, 0.5f, 1.0f);

	float4 finalColour = vOut.fogFactor * diffuseMapColour + (1.0 - vOut.fogFactor) * fogColour;

	return finalColour;
}
// A pixel shader that just outputs an diffuse map tinted by the diffuse material colour
//
float4 PSTexColour( VS_TEX_OUTPUT vOut ) : SV_Target
{
   
	float4 diffuseMapColour = DiffuseMap.Sample( TrilinearWrap, vOut.UV );
    
	 return float4( DiffuseColour.xyz * diffuseMapColour.xyz, diffuseMapColour.a ); // Only tint the RGB, get alpha from texture directly
}
float4 PSTexColour_FlipHorizontal(VS_TEX_OUTPUT vOut) : SV_Target
{
    vOut.UV *= float2(-1.0, 1.0);
    float4 diffuseMapColour = DiffuseMap.Sample(TrilinearWrap, vOut.UV);
    return float4(DiffuseColour.xyz * diffuseMapColour.xyz, diffuseMapColour.a); // Only tint the RGB, get alpha from texture directly
}
float4 PSGodRaysColour(VS_TEX_OUTPUT vOut) : SV_Target
{
	float4 diffuseMapColour = DiffuseMap.Sample(TrilinearWrap, vOut.UV);
	if (diffuseMapColour.a > 0)
	{
		if(GodrayDensity >= 500)
		diffuseMapColour.a += 0.00001 * GodrayDensity ;
		if(GodrayDensity < 500)
			diffuseMapColour.a += 0.00001 * GodrayDensity ;
	}
	if (diffuseMapColour.x > 0)
	{
		if (GodrayDecay < 90 * 0.025)
		{
			diffuseMapColour.xyz *= 0.6;
		}
		if (GodrayDecay < 110 * 0.025 && GodrayDecay >= 90 * 0.025)
		{
			diffuseMapColour.xyz *= 0.8;
		}
		if (GodrayDecay >= 110 * 0.025 && GodrayDecay < 125 * 0.025)
		{
			diffuseMapColour.xyz *= 0.9;
		}
		if (GodrayDecay > 125 * 0.025)
		{
			diffuseMapColour.xyz *= 0.7;
		}
	}
	
	
	return float4(DiffuseColour.xyz * diffuseMapColour.xyz   *   10    , diffuseMapColour.a - GodrayDensity * 0.0002); 
	// Only tint the RGB, get alpha from texture directly
}


// Per-pixel lighting pixel shader using diffuse/specular colours but no maps
//
float4 PSPixelLit( VS_LIGHTING_OUTPUT vOut ) : SV_Target 
{
	// Can't guarantee the normals are length 1 at this point, because the world matrix may contain scaling and because interpolation
	// from vertex shader to pixel shader will also rescale normal. So renormalise the normals we receive
	float3 worldNormal = normalize(vOut.WorldNormal); 


	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 cameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current pixel (in world space)
	
	//// LIGHT 1
	float3 light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 light1Dist = length(Light1Pos - vOut.WorldPos.xyz); 
	float3 diffuseLight1 = Light1Colour * max( dot(worldNormal.xyz, light1Dir), 0 ) / light1Dist;
	float3 halfway = normalize(light1Dir + cameraDir);
	float3 specularLight1 = diffuseLight1 * pow( max( dot(worldNormal.xyz, halfway), 0 ), SpecularPower );

	//// LIGHT 2
	float3 light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);
	float3 light2Dist = length(Light2Pos - vOut.WorldPos.xyz);
	float3 diffuseLight2 = Light2Colour * max( dot(worldNormal.xyz, light2Dir), 0 ) / light2Dist;
	halfway = normalize(light2Dir + cameraDir);
	float3 specularLight2 = diffuseLight2 * pow( max( dot(worldNormal.xyz, halfway), 0 ), SpecularPower );

	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 diffuseLight = AmbientColour + diffuseLight1 + diffuseLight2;
	float3 specularLight = specularLight1 + specularLight2;


	////////////////////
	// Combine colours 
	
	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = DiffuseColour * diffuseLight + SpecularColour * specularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	float4 fogColour = float4(0.5f, 0.5f, 0.5f, 0.1f);

	float4 finalColour = vOut.fogFactor * combinedColour + (1.0 - vOut.fogFactor) * fogColour;

	return combinedColour;
}


// Per-pixel lighting pixel shader using diffuse/specular material colours and combined diffuse/specular map
//
float4 PSPixelLitTex( VS_LIGHTINGTEX_OUTPUT vOut ) : SV_Target
{
	const float DepthAdjust = 0.0005f;
	// Can't guarantee the normals are length 1 at this point, because the world matrix may contain scaling and because interpolation
	// from vertex shader to pixel shader will also rescale normal. So renormalise the normals we receive
	float3 worldNormal = normalize(vOut.WorldNormal); 


	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 cameraDir = normalize(ShadowCameraPos - vOut.WorldPos.xyz); // Position of camera - position of current pixel (in world space)
	
	//// LIGHT 1
	float3 light1Dir =0;   // Direction for each light is different
	float3 light1Dist = 0; 
	float3 diffuseLight1 = 0;
	float3 halfway =0;
	float3 specularLight1 = 0;

	//// LIGHT 2
	float3 light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);
	float3 light2Dist = length(Light2Pos - vOut.WorldPos.xyz);
	float3 diffuseLight2 = Light2Colour * max( dot(worldNormal.xyz, light2Dir), 0 ) / light2Dist;
	halfway = normalize(light2Dir + cameraDir);
	float3 specularLight2 = diffuseLight2 * pow( max( dot(worldNormal.xyz, halfway), 0 ), SpecularPower );

	

	//Shadow Colour

	float4 shadowViewPos = mul(float4(vOut.WorldPos, 1.0f), ShadowViewMatrix);
	float4 shadowProjPos = mul(shadowViewPos, ShadowProjMatrix);

	float3 shadowDir = normalize(ShadowCameraPos - vOut.WorldPos.xyz);
	// Convert 2D pixel position as viewed from light into texture coordinates for shadow map - an advanced topic related to the projection step
	// Detail: 2D position x & y get perspective divide, then converted from range -1->1 to UV range 0->1. Also flip V axis
	
		float2 shadowUV = 0.5f * shadowProjPos.xy / shadowProjPos.w + float2(0.5f, 0.5f);
		

		// Get depth of this pixel if it were visible from the light (another advanced projection step)
		float depthFromLight = shadowProjPos.z / shadowProjPos.w;

		light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);
		light1Dist = length(Light1Pos - vOut.WorldPos.xyz);
		halfway = normalize(shadowDir + vOut.WorldPos.xyz);

		if (depthFromLight < ShadowMap.Sample(PointClamp, shadowUV).r  )
		{
			
			diffuseLight1 = Light1Colour * max(dot(worldNormal.xyz, light1Dir), 0) / light1Dist;
			specularLight1 = diffuseLight1 * pow(max(dot(worldNormal.xyz, light1Dir), 0), SpecularPower);
		}
		 
	
	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 diffuseLight = AmbientColour + diffuseLight1 + diffuseLight2;
	float3 specularLight = specularLight1 + specularLight2;

	////////////////////
	// Material colour

	// Combine diffuse material colour with diffuse map
	float4 diffuseTex = DiffuseMap.Sample( TrilinearWrap, vOut.UV );
	float4 diffuseMaterial = DiffuseColour * diffuseTex;
	
	// Combine specular material colour with specular map held in diffuse map alpha
	float3 specularMaterial = SpecularColour * diffuseTex.a;
	
	////////////////////
	// Combine colours 
	
	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = diffuseMaterial * diffuseLight  + specularMaterial * specularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	float4 fogColour = float4(0.5f, 0.5f, 0.5f, 0.1f);

	float4 finalColour = vOut.fogFactor * combinedColour + (1.0 - vOut.fogFactor) * fogColour;

	
	
	if (FogEnable)
	{
		return finalColour;
	}

	
	return combinedColour;
}

float4 PSAtmosLitTex(VS_LIGHTINGTEX_OUTPUT vOut) : SV_Target
{
	float3 worldNormal = normalize(vOut.WorldNormal);


	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 cameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current pixel (in world space)

																 //// LIGHT 1
	float3 light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 light1Dist = length(Light1Pos - vOut.WorldPos.xyz);
	float3 diffuseLight1 = Light1Colour * max(dot(worldNormal.xyz, light1Dir), 0) / light1Dist;
	float3 halfway = normalize(light1Dir + cameraDir);
	float3 specularLight1 = diffuseLight1 * pow(max(dot(worldNormal.xyz, halfway), 0), SpecularPower);

	//// LIGHT 2
	float3 light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);
	float3 light2Dist = length(Light2Pos - vOut.WorldPos.xyz);
	float3 diffuseLight2 = Light2Colour * max(dot(worldNormal.xyz, light2Dir), 0) / light2Dist;
	halfway = normalize(light2Dir + cameraDir);
	float3 specularLight2 = diffuseLight2 * pow(max(dot(worldNormal.xyz, halfway), 0), SpecularPower);

	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 diffuseLight = AmbientColour + diffuseLight1 + diffuseLight2;
	float3 specularLight = specularLight1 + specularLight2;

	////////////////////
	// Material colour
	float DiffuseLevel = dot(worldNormal, light1Dir) + 0.4f;
	// Combine diffuse material colour with diffuse map
	float4 diffuseTex = DiffuseMap.Sample(TrilinearWrap, vOut.UV);
	float4 diffuseMaterial = DiffuseColour * diffuseTex;

	// Combine specular material colour with specular map held in diffuse map alpha
	float3 specularMaterial = SpecularColour * diffuseTex.a;

	////////////////////
	// Combine colours 
	float facing = dot(worldNormal, cameraDir);
	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = float3(0.2f, 0.6f, 0.5f);
	combinedColour.rgb *= diffuseLight + specularMaterial ;
	combinedColour.a = 0.5 ; // No alpha processing in this shader, so just set it to 1
	combinedColour.a *= saturate(facing);
	return combinedColour;
}

// Normal mapping pixel shader combining wih diffuse/specular material colours
//
float4 PSNormalMap( VS_NORMALMAP_OUTPUT vOut ) : SV_Target
{
	///////////////////////
	// Normal Map Extraction

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
	float3 modelNormal = normalize( vOut.ModelNormal );
	float3 modelTangent = normalize( vOut.ModelTangent );

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space
	float3 modelBiTangent = cross( modelNormal, modelTangent );
	float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);
	
	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
	float3 textureNormal = 2.0f * NormalMap.Sample( TrilinearWrap, vOut.UV ) - 1.0f; // Scale from 0->1 to -1->1

	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
	float3 worldNormal = normalize( mul( mul( textureNormal, invTangentMatrix ), WorldMatrix ) );


	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 cameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current vertex (or pixel) (in world space)
	
	//// LIGHT 1
	float3 light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 light1Dist = length(Light1Pos - vOut.WorldPos.xyz); 
	float3 diffuseLight1 = Light1Colour * max( dot(worldNormal.xyz, light1Dir), 0 ) / light1Dist;
	float3 halfway = normalize(light1Dir + cameraDir);
	float3 specularLight1 = diffuseLight1 * pow( max( dot(worldNormal.xyz, halfway), 0 ), SpecularPower );

	//// LIGHT 2
	float3 light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);
	float3 light2Dist = length(Light2Pos - vOut.WorldPos.xyz);
	float3 diffuseLight2 = Light2Colour * max( dot(worldNormal.xyz, light2Dir), 0 ) / light2Dist;
	halfway = normalize(light2Dir + cameraDir);
	float3 specularLight2 = diffuseLight2 * pow( max( dot(worldNormal.xyz, halfway), 0 ), SpecularPower );

	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 diffuseLight = AmbientColour + diffuseLight1 + diffuseLight2;
	float3 specularLight = specularLight1 + specularLight2;


	////////////////////
	// Material colour

	// Combine diffuse material colour with diffuse map
	float4 diffuseTex = DiffuseMap.Sample( TrilinearWrap, vOut.UV );
	float4 diffuseMaterial = DiffuseColour * diffuseTex;
	
	// Combine specular material colour with specular map held in diffuse map alpha
	float3 specularMaterial = SpecularColour * diffuseTex.a;

	
	////////////////////
	// Combine colours 
	
	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = diffuseMaterial * diffuseLight + specularMaterial * specularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}


// Parallax mapping pixel shader combining wih diffuse/specular material colours
//
float4 PSParallaxMap( VS_NORMALMAP_OUTPUT vOut ) : SV_Target
{
	///////////////////////
	// Normal Map Extraction

	// Will use the model normal/tangent to calculate matrix for tangent space. The normals for each pixel are *interpolated* from the
	// vertex normals/tangents. This means they will not be length 1, so they need to be renormalised (same as per-pixel lighting issue)
	float3 modelNormal = normalize( vOut.ModelNormal );
	float3 modelTangent = normalize( vOut.ModelTangent );

	// Calculate bi-tangent to complete the three axes of tangent space - then create the *inverse* tangent matrix to convert *from*
	// tangent space into model space
	float3 modelBiTangent = cross( modelNormal, modelTangent );
	float3x3 invTangentMatrix = float3x3(modelTangent, modelBiTangent, modelNormal);

	/// Parallax Mapping Extra ///

	// Get normalised vector to camera for parallax mapping and specular equation (this vector was calculated later in previous shaders)
	float3 CameraDir = normalize(CameraPos - vOut.WorldPos.xyz);

	// Transform camera vector from world into model space. Need *inverse* world matrix for this.
	// Only need 3x3 matrix to transform vectors, to invert a 3x3 matrix we transpose it (flip it about its diagonal)
	float3x3 invWorldMatrix = transpose( WorldMatrix );
	float3 cameraModelDir = normalize( mul( CameraDir, invWorldMatrix ) ); // Normalise in case world matrix is scaled

	// Then transform model-space camera vector into tangent space (texture coordinate space) to give the direction to offset texture
	// coordinate, only interested in x and y components. Calculated inverse tangent matrix above, so invert it back for this step
	float3x3 tangentMatrix = transpose( invTangentMatrix ); 
	float2 textureOffsetDir = mul( cameraModelDir, tangentMatrix );
	
	// Get the depth info from the normal map's alpha channel at the given texture coordinate
	// Rescale from 0->1 range to -x->+x range, x determined by ParallaxDepth setting
	float texDepth = ParallaxDepth * (NormalMap.Sample( TrilinearWrap, vOut.UV ).a - 0.5f);
	
	// Use the depth of the texture to offset the given texture coordinate - this corrected texture coordinate will be used from here on
	float2 offsetTexCoord = vOut.UV + texDepth * textureOffsetDir;

	///////////////////////////////

	// Get the texture normal from the normal map. The r,g,b pixel values actually store x,y,z components of a normal. However, r,g,b
	// values are stored in the range 0->1, whereas the x, y & z components should be in the range -1->1. So some scaling is needed
	float3 textureNormal = 2.0f * NormalMap.Sample( TrilinearWrap, offsetTexCoord ) - 1.0f; // Scale from 0->1 to -1->1

	// Now convert the texture normal into model space using the inverse tangent matrix, and then convert into world space using the world
	// matrix. Normalise, because of the effects of texture filtering and in case the world matrix contains scaling
	float3 worldNormal = normalize( mul( mul( textureNormal, invTangentMatrix ), WorldMatrix ) );


	///////////////////////
	// Calculate lighting

	// Calculate direction of camera
	float3 cameraDir = normalize(CameraPos - vOut.WorldPos.xyz); // Position of camera - position of current vertex (or pixel) (in world space)
	
	//// LIGHT 1
	float3 light1Dir = normalize(Light1Pos - vOut.WorldPos.xyz);   // Direction for each light is different
	float3 light1Dist = length(Light1Pos - vOut.WorldPos.xyz); 
	float3 diffuseLight1 = Light1Colour * max( dot(worldNormal.xyz, light1Dir), 0 ) / light1Dist;
	float3 halfway = normalize(light1Dir + cameraDir);
	float3 specularLight1 = diffuseLight1 * pow( max( dot(worldNormal.xyz, halfway), 0 ), SpecularPower );

	//// LIGHT 2
	float3 light2Dir = normalize(Light2Pos - vOut.WorldPos.xyz);
	float3 light2Dist = length(Light2Pos - vOut.WorldPos.xyz);
	float3 diffuseLight2 = Light2Colour * max( dot(worldNormal.xyz, light2Dir), 0 ) / light2Dist;
	halfway = normalize(light2Dir + cameraDir);
	float3 specularLight2 = diffuseLight2 * pow( max( dot(worldNormal.xyz, halfway), 0 ), SpecularPower );

	// Sum the effect of the two lights - add the ambient at this stage rather than for each light (or we will get twice the ambient level)
	float3 diffuseLight = AmbientColour + diffuseLight1 + diffuseLight2;
	float3 specularLight = specularLight1 + specularLight2;


	////////////////////
	// Material colour

	// Combine diffuse material colour with diffuse map
	float4 diffuseTex = DiffuseMap.Sample( TrilinearWrap, offsetTexCoord );
	float4 diffuseMaterial = DiffuseColour * diffuseTex;
	
	// Combine specular material colour with specular map held in diffuse map alpha
	float3 specularMaterial = SpecularColour * diffuseTex.a;

	
	////////////////////
	// Combine colours 
	
	// Combine maps and lighting for final pixel colour
	float4 combinedColour;
	combinedColour.rgb = diffuseMaterial * diffuseLight + specularMaterial * specularLight;
	combinedColour.a = 1.0f; // No alpha processing in this shader, so just set it to 1

	return combinedColour;
}
float4 PSGodrays(VS_GODRAYS_OUTPUT vOut) : SV_Target
{
	//blurring
	
	// location on screen relative to image center
	float2 sUV = float2(vOut.ScreenCoords.x / vOut.ScreenCoords.w, vOut.ScreenCoords.y / vOut.ScreenCoords.w);
	// we just want this as a 2D direction, so normalize it
	sUV = normalize(sUV);
	// duv will be our screen-radial 2D UV vector -- that is, our step size in UV based
	//    on the local partial derivatives of UV in screen-x and screen-y.
	//    Note that ddx()/ddy() return "float2" here
	float2 duv = (sUV.x * ddx(vOut.UV)) - (sUV.y * ddy(vOut.UV));
	//scaling
	int scale = 100;
	duv *= scale;
	// now we can use this step to accumulate our color samples
	float4 blurred = 0;
	float blurScaling = 0;
	const int numSamples = 64;
    
		for (int i = 0; i < numSamples; i++)
		{
			blurScaling = (1 / (float)(numSamples - 1));
			blurred += DiffuseMap.Sample(GodraysSample, vOut.UV + blurScaling + duv);
	    }
	blurred /= numSamples;
	//tweak the colour
	blurred.rgb = pow(blurred.rgb, 20.0f);
	blurred.rgb *= scale;
	blurred.rgb = saturate(blurred.rgb);
	//now compose original pic on top of the blur
	float4 originalTex = DiffuseMap.Sample(GodraysSample, vOut.UV);
	float3 newC = originalTex.rgb + (1.0 - originalTex.a) * blurred.rgb;
	float newA = max(originalTex.a, blurred.a);
	return float4 (newC.rgb, newA);

}

PS_PARTICLE_OUTPUT PS_SoftParticle(PS_PARTICLE_INPUT pIn)
{
    PS_PARTICLE_OUTPUT pOut;

	// Extract diffuse material colour for this pixel from a texture
    float4 texColour = DiffuseMap.Sample(TrilinearWrap, pIn.UV);

	// Combine texture alpha with particle alpha
    pOut.Colour = texColour;
    pOut.Colour.a *= pIn.Alpha;
    pOut.Depth = pIn.ProjPos.z;
    return pOut;
}
//--------------------------------------------------------------------------------------
// States
//--------------------------------------------------------------------------------------

// States are needed to switch between additive blending for the lights and no blending for other models

RasterizerState CullNone  // Cull none of the polygons, i.e. show both sides
{
	CullMode = None;
};
RasterizerState CullBack  // Cull back side of polygon - normal behaviour, only show front of polygons
{
	CullMode = Back;
};
RasterizerState CullFront  // Cull back side of polygon - normal behaviour, only show front of polygons
{
	CullMode = Front;
};

DepthStencilState DepthWritesOff // Don't write to the depth buffer - polygons rendered will not obscure other polygons
{
	DepthWriteMask = ZERO;
};
DepthStencilState DepthWritesOn  // Write to the depth buffer - normal behaviour 
{
	DepthWriteMask = ALL;
};
DepthStencilState DepthEnable
{
	DepthEnable = TRUE;
};

BlendState NoBlending // Switch off blending - pixels will be opaque
{
    BlendEnable[0] = FALSE;
};

BlendState AlphaBlending
{
    BlendEnable[0] = TRUE;
    SrcBlend = SRC_ALPHA;
    DestBlend = INV_SRC_ALPHA;
    BlendOp = ADD;
};


//--------------------------------------------------------------------------------------
// Techniques
//--------------------------------------------------------------------------------------

// Techniques are used to render models in our scene. They select a combination of vertex, geometry and pixel shader from those provided above. Can also set states.
// Different material render methods select different techniques

technique10 Atmosphere
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VSPixelLitTex()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PSAtmosLitTex()));

		// Switch off blending states
		SetBlendState(AlphaBlending, float4(0.0f, 0.2f, 0.8f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOff, 0);
	}
}
//shadow map render technique
technique10 DepthOnly
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VSShadowTransform()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PSPixelDepth()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}
//Fog render technique
technique10 Fog
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VSFogVertexShader()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PSFogPixelShader()));

		// Switch off blending states
		SetBlendState(NoBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullBack);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}
// Diffuse material colour only
technique10 PlainColour
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VSTransformOnly() ) );
        SetGeometryShader( NULL );                                   
        SetPixelShader( CompileShader( ps_4_0, PSPlainColour() ) );

		// Switch off blending states
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState( CullBack ); 
		SetDepthStencilState( DepthWritesOn, 0 );
     }
}


// Texture tinted with diffuse material colour
technique10 TexColour
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VSTransformTex() ) );
        SetGeometryShader( NULL );                                   
        SetPixelShader( CompileShader( ps_4_0, PSTexColour() ) );

		// Switch off blending states
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState( CullBack ); 
		SetDepthStencilState( DepthWritesOn, 0 );
     }
}

technique10 AlphaBlend
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VSTransformTex()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PSTexColour()));

		// Switch off blending states
		SetBlendState(AlphaBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(DepthWritesOn, 0);
	}
}

technique10 AlphaBlendFlipHorizontal
{
    pass P0
    {
        SetVertexShader(CompileShader(vs_4_0, VSTransformTex()));
        SetGeometryShader(NULL);
        SetPixelShader(CompileShader(ps_4_0, PSTexColour_FlipHorizontal()));

		// Switch off blending states
        SetBlendState(AlphaBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
        SetRasterizerState(CullNone);
        SetDepthStencilState(DepthWritesOff, 0);
    }
}
//God Rays
technique10 GodRays
{
	pass P0
	{
		SetVertexShader(CompileShader(vs_4_0, VSGodrays()));
		SetGeometryShader(NULL);
		SetPixelShader(CompileShader(ps_4_0, PSGodrays()));

		// Switch off blending states
		SetBlendState(AlphaBlending, float4(0.0f, 0.0f, 0.0f, 0.0f), 0xFFFFFFFF);
		SetRasterizerState(CullNone);
		SetDepthStencilState(DepthEnable, 0);
	}
	
}
// Pixel lighting with diffuse texture
technique10 PixelLit
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VSPixelLit() ) );
        SetGeometryShader( NULL );                                   
        SetPixelShader( CompileShader( ps_4_0, PSPixelLit() ) );

		// Switch off blending states
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState( CullBack ); 
		SetDepthStencilState( DepthWritesOn, 0 );
	}
}

// Pixel lighting with diffuse texture
technique10 PixelLitTex
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VSPixelLitTex() ) );
        SetGeometryShader( NULL );                                   
        SetPixelShader( CompileShader( ps_4_0, PSPixelLitTex() ) );

		// Switch off blending states
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState( CullBack ); 
		SetDepthStencilState( DepthWritesOn, 0 );
	}
	
}

// Normal Mapping
technique10 NormalMapping
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VSNormalMap() ) );
        SetGeometryShader( NULL );                                   
        SetPixelShader( CompileShader( ps_4_0, PSNormalMap() ) );

		// Switch off blending states
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState( CullBack ); 
		SetDepthStencilState( DepthWritesOn, 0 );
	}
}

// Parallax Mapping
technique10 ParallaxMapping
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0, VSNormalMap() ) );
        SetGeometryShader( NULL );                                   
        SetPixelShader( CompileShader( ps_4_0, PSParallaxMap() ) );

		// Switch off blending states
		SetBlendState( NoBlending, float4( 0.0f, 0.0f, 0.0f, 0.0f ), 0xFFFFFFFF );
		SetRasterizerState( CullBack ); 
		SetDepthStencilState( DepthWritesOn, 0 );
	}
}


