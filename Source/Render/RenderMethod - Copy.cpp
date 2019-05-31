/*******************************************
	RenderMethod.cpp

	A set of render methods - different
	vertex/pixel shader combinations that
	can be selected for different parts of
	a mesh being rendered
********************************************/

#include "RenderMethod.h"
#include "MathDX.h"

namespace gen
{

//-----------------------------------------------------------------------------
// Render Method Specifications
//-----------------------------------------------------------------------------

// Prototypes for shader initialisation functions in array below
// The functions are defined using a function pointer type (PShaderFn in RenderMethod.h)
// These functions must all have the same style of prototype as shown above
void VS_XFormFn( int method, CMatrix4x4* worldMatrix, CCamera* camera );
void VS_VertLit1Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera );
void VS_NormalMap2Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera );
void PS_NullFn( int method, CMatrix4x4* worldMatrix, CCamera* camera );
void PS_PlainColourFn( int method, CMatrix4x4* worldMatrix, CCamera* camera );
void PS_PixelLit2Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera );
void PS_NormalMap2Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera );
void PS_ManualAlphaFn( int method, CMatrix4x4* worldMatrix, CCamera* camera );
void PS_AtmosphereFn( int method, CMatrix4x4* worldMatrix, CCamera* camera );


// Vertex element structures for array below
// Standard vertex elements: vertex coordinate + normal + texture coords(UV)
D3DVERTEXELEMENT9 PosNormUV[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END() // Terminate a vertex declaration with special element
};

// Vertex elements for normal mapping: vertex coordinate + normal & tangent + texture coords(UV)
D3DVERTEXELEMENT9 PosNormTanUV[] =
{
	{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
	{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
	{ 0, 24, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT, 0 },
	{ 0, 36, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
	D3DDECL_END()
};


//****| INFO |************************************************************************************/
// The available render methods are in ERenderMethod in RenderMethod.h. This array defines the
// exact operation of each render method in turn. Each method has a vertex and pixel shader
// source file and a function to initialise them for rendering. Also provide the vertex element
// structure (from those given above) and a boolean indicating if the render method contains
// tangents. The DirectX shader and vertex declaration pointers are also provided (initially 0)
//************************************************************************************************/
SRenderMethod renderMethods[NumRenderMethods] =
{
//	|Vertex shader|       |V Shader init fn| |Pixel Shader|            |P shader init fn| |Num Tex| |Vert Elts|  |Tangents|  |for internal use|   |Method Name|
	"XFormOnly.vsh",      VS_XFormFn,        "PlainColour.psh",        PS_PlainColourFn,      0,    PosNormUV,     false,      0, 0, 0, 0, 0,  // PlainColour   
	"XFormTex.vsh",       VS_XFormFn,        "TextureOnly.psh",        PS_NullFn,             1,    PosNormUV,     false,      0, 0, 0, 0, 0,  // PlainTexture  
	"VertexLit1.vsh",     VS_VertLit1Fn,     "DiffuseColour.psh",      PS_NullFn,             0,    PosNormUV,     false,      0, 0, 0, 0, 0,  // VertexLit     
	"VertexLit1Tex.vsh",  VS_VertLit1Fn,     "DiffuseColourTex.psh",   PS_NullFn,             1,    PosNormUV,     false,      0, 0, 0, 0, 0,  // VertexLitTex  
	"PixelLit.vsh",       VS_XFormFn,        "PixelLit2.psh",          PS_PixelLit2Fn,        0,    PosNormUV,     false,      0, 0, 0, 0, 0,  // PixelLit      
	"PixelLitTex.vsh",    VS_XFormFn,        "PixelLit2Tex.psh",       PS_PixelLit2Fn,        1,    PosNormUV,     false,      0, 0, 0, 0, 0,  // PixelLitTex   
	"NormalMap.vsh",      VS_NormalMap2Fn,   "NormalMap.psh",          PS_NormalMap2Fn,       2,    PosNormTanUV,  true,       0, 0, 0, 0, 0,  // NormalMap       
	"DayNightPlanet.vsh", VS_VertLit1Fn,     "DayNightPlanet.psh",     PS_NullFn,             2,    PosNormUV,     false,      0, 0, 0, 0, 0,  // DayNightPlanet
	"Clouds.vsh",         VS_VertLit1Fn,     "Clouds.psh",             PS_ManualAlphaFn,      1,    PosNormUV,     false,      0, 0, 0, 0, 0,  // Clouds        
	"Atmosphere.vsh",     VS_VertLit1Fn,     "DiffuseColourAlpha.psh", PS_AtmosphereFn,       0,    PosNormUV,     false,      0, 0, 0, 0, 0,  // Atmosphere    
};


//-----------------------------------------------------------------------------
// Selection of render method
//-----------------------------------------------------------------------------

// Given a material name and the main texture used by that material, return the render method to
// use for that material. The available render methods are in ERenderMethod in RenderMethod.h
ERenderMethod RenderMethodFromMaterial
(
	const string&  materialName,
	const string&  textureName
)
{
	// Check if material has a texture at all
	if (textureName == "")
	{
		// Select from render methods that use no texture
		if (materialName.find("Atmosphere") == 0) // See if string begins with "Atmosphere"
		{
			return Atmosphere;
		}
		else
		{
			return PlainColour;
		}
	}
	else
	{
		// Select from render methods that use textures
		if (materialName.find("Plain") == 0) // See if string begins with "Plain"
		{
			return PlainTexture;
		}
		else if (materialName.find("VertexLit") == 0)
		{
			return VertexLitTex;
		}
		else if (materialName.find("NormalMap") == 0)
		{
			return NormalMap;
		}
		else if (materialName.find("Earth") == 0)
		{
			return DayNightPlanet;
		}
		else if (materialName.find("Clouds") == 0)
		{
			return Clouds;
		}
		else
		{
			return PixelLitTex;
		}
	}
}

// Return the number of textures used by a given render method
// If a render method uses multiple textures, secondary texture names will be based on the main
// texture name. E.g. if a normal mapping method uses 3 textures and the main texture is "wall.jpg"
// then the other textures must be named "wall1.jpg" and "wall2.jpg"
// The available render methods are in ERenderMethod in RenderMethod.h
unsigned int NumTexturesUsedByRenderMethod( ERenderMethod method )
{
	return renderMethods[method].numTextures;
}

// Return whether given render method uses tangents
bool RenderMethodUsesTangents( ERenderMethod method )
{
	return renderMethods[method].hasTangents;
}


//-----------------------------------------------------------------------------
// Method constants / globals / related functions
//-----------------------------------------------------------------------------

// Get reference to global DirectX render device from another source file
// Not good practice - these functions should be part of a class with this as a member
extern LPDIRECT3DDEVICE9 g_pd3dDevice;


// Folders used
static const string MediaFolder = "Media\\";
static const string ShaderFolder = "Source\\Render\\";


// Material diffuse/specular colour and specular power used for all methods, altered through function below
static D3DXCOLOR m_DiffuseColour( 1.0f, 1.0f, 1.0f, 1.0f );
static D3DXCOLOR m_SpecularColour( 1.0f, 1.0f, 1.0f, 1.0f );
static float m_SpecularPower = 0.0f;

// Ambient colour used for all methods, can be altered through function below
static SColourRGBA m_AmbientLight( 0.0f, 0.0f, 0.0f, 0.0f );

// Pointer to light list used for all methods, can be altered through function below
static CLight** m_Lights = 0;


// Set the material colour and specular power used in all methods
void SetMaterialColour( const D3DXCOLOR& diffuseColour, const D3DXCOLOR& specularColour, float specularPower )
{
	m_DiffuseColour = diffuseColour;
	m_SpecularColour = specularColour;
	m_SpecularPower = specularPower;
}

// Set the ambient light colour used for all methods
void SetAmbientLight( const SColourRGBA& colour )
{
	m_AmbientLight = colour;
}

// Set the light list to use for all methods
void SetLights( CLight** lights )
{
	m_Lights = lights;
}


//-----------------------------------------------------------------------------
// Method usage
//-----------------------------------------------------------------------------

// Use the given method for rendering, pass the world matrix and camera to be used for the shaders
void UseMethod( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	// Set shaders in DirectX
	g_pd3dDevice->SetVertexDeclaration( renderMethods[method].vertexDecl );
	g_pd3dDevice->SetVertexShader( renderMethods[method].vertexShader );
	g_pd3dDevice->SetPixelShader( renderMethods[method].pixelShader );

	// Initialise shader constants and other render settings
	renderMethods[method].vertexShaderFn( method, worldMatrix, camera );
	renderMethods[method].pixelShaderFn( method, worldMatrix, camera );
}


//-----------------------------------------------------------------------------
// Method initialisation
//-----------------------------------------------------------------------------

// Initialise general method data
bool InitialiseMethods()
{
	// Nothing to do (could load look-up textures here)

	return true;
}

// Initialises the given render method (vertex + pixel shader), returns true on success
bool LoadMethod( int method )
{
	// If the vertex shader for this method has not already been initialised
	if (!renderMethods[method].vertexShader)
	{
		// Load the vertex shader file specified above, storing resultant DirectX data
		if (!LoadVertexShader( renderMethods[method].vertexShaderFile,
                               &renderMethods[method].vertexShader,
		                       &renderMethods[method].vertexConsts ))
		{
			return false;
		}
	}
	if (!renderMethods[method].pixelShader)
	{
		// Load the vertex shader file specified above, storing resultant DirectX data
		if (!LoadPixelShader( renderMethods[method].pixelShaderFile,
		                      &renderMethods[method].pixelShader,
		                      &renderMethods[method].pixelConsts ))
		{
			return false;
		}
	}

	if (!renderMethods[method].vertexDecl)
	{
		if (FAILED(g_pd3dDevice->CreateVertexDeclaration( renderMethods[method].vertexElts,
		                                                  &renderMethods[method].vertexDecl )))
		{
			return false;
		}
	}

	return true;
}

// Releases the DirectX data associated with all render methods
void ReleaseMethods()
{
	for (int method = 0; method < NumRenderMethods; ++method)
	{
		if (renderMethods[method].vertexDecl)
		{
			renderMethods[method].vertexDecl->Release();
		}
		if (renderMethods[method].pixelConsts)
		{
			renderMethods[method].pixelConsts->Release();
		}
		if (renderMethods[method].pixelShader)
		{
			renderMethods[method].pixelShader->Release();
		}
		if (renderMethods[method].vertexConsts)
		{
			renderMethods[method].vertexConsts->Release();
		}
		if (renderMethods[method].vertexShader)
		{
			renderMethods[method].vertexShader->Release();
		}
	}
}


//-----------------------------------------------------------------------------
// Shader initialisation
//-----------------------------------------------------------------------------

// Load and compiler a HLSL vertex shader from a file. Provide the source code filename and pointers
// to the variables to hold the resultant shader and it associated constant table
bool LoadVertexShader( const string& fileName, LPDIRECT3DVERTEXSHADER9* vertexShader,
					   LPD3DXCONSTANTTABLE* constants )
{
	// Temporary variable to hold compiled pixel shader code
    LPD3DXBUFFER pShaderCode;

	// Compile external HLSL pixel shader into shader code to submit to the hardware
	string fullFileName = ShaderFolder + fileName;
	HRESULT hr = 
		D3DXCompileShaderFromFile( fullFileName.c_str(),// File containing pixel shader (HLSL)
			                       NULL, NULL,       // Advanced compilation options - not needed here
								   "main",           // Name of main function in the shader
								   "vs_3_0",         // Target vertex shader hardware - vs_1_1 is lowest level
												     // and will work on all video cards with a pixel shader
								   SHADER_FLAGS,     // Additional compilation flags (such as debug flags)
								   &pShaderCode,     // Ptr to variable to hold compiled shader code
								   NULL,             // Ptr to variable to hold error messages (not needed)
								   constants );      // Ptr to variable to hold constants for the shader
    if (FAILED(hr))
	{
		// Return if compilation failed
		return false;
	}

	// Create the pixel shader using the compiled shader code
    hr = g_pd3dDevice->CreateVertexShader( (DWORD*)pShaderCode->GetBufferPointer(), vertexShader );
    
	// Discard the shader code now the shader has been created 
	pShaderCode->Release();

	// If the creation failed then return (wait until after shader code has been discarded)
    if (FAILED(hr))
	{
		return false;
	}

	return true;
}


// Load and compiler a HLSL pixel shader from a file. Provide the source code filename and pointers
// to the variables to hold the resultant shader and it associated constant table
bool LoadPixelShader( const string& fileName, LPDIRECT3DPIXELSHADER9* pixelShader,
					  LPD3DXCONSTANTTABLE* constants )
{
	// Temporary variable to hold compiled pixel shader code
    LPD3DXBUFFER pShaderCode;

	// Compile external HLSL pixel shader into shader code to submit to the hardware
	string fullFileName = ShaderFolder + fileName;
	HRESULT hr = 
		D3DXCompileShaderFromFile( fullFileName.c_str(), // File containing pixel shader (HLSL)
			                       NULL, NULL,       // Advanced compilation options - not needed here
								   "main",           // Name of main function in the shader
								   "ps_3_0",         // Target pixel shader hardware - ps_1_1 is lowest level
												     // and will work on all video cards with a pixel shader
								   SHADER_FLAGS,     // Additional compilation flags (such as debug flags)
								   &pShaderCode,     // Ptr to variable to hold compiled shader code
								   NULL,             // Ptr to variable to hold error messages (not needed)
								   constants );      // Ptr to variable to hold constants for the shader
    if (FAILED(hr))
	{
		// Return if compilation failed
		return false;
	}

	// Create the pixel shader using the compiled shader code
    hr = g_pd3dDevice->CreatePixelShader( (DWORD*)pShaderCode->GetBufferPointer(), pixelShader );
    
	// Discard the shader code now the shader has been created 
	pShaderCode->Release();

	// If the creation failed then return (wait until after shader code has been discarded)
    if (FAILED(hr))
	{
		return false;
	}

	return true;
}



//-----------------------------------------------------------------------------
// Vertex shader initialisation functions
//-----------------------------------------------------------------------------

// Pass the world matrix and view/projection matrix to the vertex shader
void VS_XFormFn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	LPD3DXCONSTANTTABLE shaderConsts = renderMethods[method].vertexConsts;

	D3DXMATRIXA16 matViewProj = ToD3DXMATRIX( camera->GetViewProjMatrix() );
	shaderConsts->SetMatrix( g_pd3dDevice, "ViewProjMatrix", &matViewProj );

	D3DXMATRIX* matWorld = ToD3DXMATRIXPtr( worldMatrix );
	shaderConsts->SetMatrix( g_pd3dDevice, "WorldMatrix", matWorld );
}

// Pass data to vertex shaders that perform vertex lighting (1 point light)
// Passes full range of data - some shaders don't need all of it. This
// reduces the number of these functions at the expense of redundancy
void VS_VertLit1Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	LPD3DXCONSTANTTABLE shaderConsts = renderMethods[method].vertexConsts;

	D3DXMATRIXA16 matViewProj = ToD3DXMATRIX( camera->GetViewProjMatrix() );
	shaderConsts->SetMatrix( g_pd3dDevice, "ViewProjMatrix", &matViewProj );

	D3DXMATRIX* matWorld = ToD3DXMATRIXPtr( worldMatrix );
	shaderConsts->SetMatrix( g_pd3dDevice, "WorldMatrix", matWorld );

	D3DXVECTOR3 cameraPos = ToD3DXVECTOR( camera->Position() );
	shaderConsts->SetFloatArray( g_pd3dDevice, "CameraPosition", (FLOAT*)&cameraPos, 3 ); // If needed

	shaderConsts->SetFloatArray( g_pd3dDevice, "AmbientLight", (FLOAT*)&m_AmbientLight, 3 );
	shaderConsts->SetFloatArray( g_pd3dDevice, "LightPosition", (FLOAT*)&m_Lights[0]->GetPosition(), 3 );
	shaderConsts->SetFloatArray( g_pd3dDevice, "LightColour", (FLOAT*)&m_Lights[0]->GetColour(), 3 );
	shaderConsts->SetFloat( g_pd3dDevice, "LightBrightness", m_Lights[0]->GetBrightness() );

	shaderConsts->SetFloatArray( g_pd3dDevice, "MaterialColour", (FLOAT*)&m_DiffuseColour, 3 ); // If needed
	shaderConsts->SetFloatArray( g_pd3dDevice, "SpecularStrength", (FLOAT*)&m_SpecularColour, 3 ); // If needed
	shaderConsts->SetFloat( g_pd3dDevice, "SpecularPower", m_SpecularPower ); // If needed
}

// Pass data to vertex shaders that prepare for normal mapping (2 point lights)
// Uses optimised normal mapping from previous lab
void VS_NormalMap2Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	LPD3DXCONSTANTTABLE shaderConsts = renderMethods[method].vertexConsts;

	D3DXMATRIXA16 matViewProj = ToD3DXMATRIX( camera->GetViewProjMatrix() );
	shaderConsts->SetMatrix( g_pd3dDevice, "ViewProjMatrix", &matViewProj );

	D3DXMATRIX* matWorld = ToD3DXMATRIXPtr( worldMatrix );
	shaderConsts->SetMatrix( g_pd3dDevice, "WorldMatrix", matWorld );

	shaderConsts->SetFloatArray( g_pd3dDevice, "Light1Position", (FLOAT*)&m_Lights[0]->GetPosition(), 3 );
	shaderConsts->SetFloatArray( g_pd3dDevice, "Light2Position", (FLOAT*)&m_Lights[1]->GetPosition(), 3 );
}


//-----------------------------------------------------------------------------
// Pixel shader initialisation functions
//-----------------------------------------------------------------------------

// Set the default settings for render states used in this app
void SetDefaultStates()
{
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE ); 
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );
}

// Function for pixel shaders that don't require any setup
void PS_NullFn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	// Nothing to do, just ensure we have default render states
	SetDefaultStates();
}

// Function for pixel shader that just needs material colour
void PS_PlainColourFn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	// Ensure we have default render states
	SetDefaultStates();

	LPD3DXCONSTANTTABLE shaderConsts = renderMethods[method].pixelConsts;

	shaderConsts->SetFloatArray( g_pd3dDevice, "MaterialColour", (FLOAT*)&m_DiffuseColour, 3 );
}

// Pass data to pixel shaders that perform pixel lighting (2 point lights)
void PS_PixelLit2Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	// Ensure we have default render states
	SetDefaultStates();

	LPD3DXCONSTANTTABLE shaderConsts = renderMethods[method].pixelConsts;

	D3DXVECTOR3 cameraPos = ToD3DXVECTOR( camera->Position() );
	shaderConsts->SetFloatArray( g_pd3dDevice, "CameraPosition", (FLOAT*)&cameraPos, 3 );

	shaderConsts->SetFloatArray( g_pd3dDevice, "AmbientLight", (FLOAT*)&m_AmbientLight, 3 );
	shaderConsts->SetFloatArray( g_pd3dDevice, "Light1Position", (FLOAT*)&m_Lights[0]->GetPosition(), 3 );
	shaderConsts->SetFloatArray( g_pd3dDevice, "Light1Colour", (FLOAT*)&m_Lights[0]->GetColour(), 3 );
	shaderConsts->SetFloat( g_pd3dDevice, "Light1Brightness", m_Lights[0]->GetBrightness() );
	shaderConsts->SetFloatArray( g_pd3dDevice, "Light2Position", (FLOAT*)&m_Lights[1]->GetPosition(), 3 );
	shaderConsts->SetFloatArray( g_pd3dDevice, "Light2Colour", (FLOAT*)&m_Lights[1]->GetColour(), 3 );
	shaderConsts->SetFloat( g_pd3dDevice, "Light2Brightness", m_Lights[1]->GetBrightness() );

	shaderConsts->SetFloatArray( g_pd3dDevice, "MaterialColour", (FLOAT*)&m_DiffuseColour, 3 ); // If needed
	shaderConsts->SetFloatArray( g_pd3dDevice, "SpecularStrength", (FLOAT*)&m_SpecularColour, 3 ); // If needed
	shaderConsts->SetFloat( g_pd3dDevice, "SpecularPower", m_SpecularPower );
}

// Pass data to pixel shaders that perform normal mapping (2 point lights)
// Uses optimised normal mapping from previous lab
void PS_NormalMap2Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	// Ensure we have default render states
	SetDefaultStates();

	LPD3DXCONSTANTTABLE shaderConsts = renderMethods[method].pixelConsts;

	D3DXMATRIX* matWorld = ToD3DXMATRIXPtr( worldMatrix );
	shaderConsts->SetMatrix( g_pd3dDevice, "WorldMatrix", matWorld );

	D3DXVECTOR3 cameraPos = ToD3DXVECTOR( camera->Position() );
	shaderConsts->SetFloatArray( g_pd3dDevice, "CameraPosition", (FLOAT*)&cameraPos, 3 );

	shaderConsts->SetFloatArray( g_pd3dDevice, "AmbientLight", (FLOAT*)&m_AmbientLight, 3 );
	shaderConsts->SetFloatArray( g_pd3dDevice, "Light1Colour", (FLOAT*)&m_Lights[0]->GetColour(), 3 );
	shaderConsts->SetFloat( g_pd3dDevice, "Light1Brightness", m_Lights[0]->GetBrightness() );
	shaderConsts->SetFloatArray( g_pd3dDevice, "Light2Colour", (FLOAT*)&m_Lights[1]->GetColour(), 3 );
	shaderConsts->SetFloat( g_pd3dDevice, "Light2Brightness", m_Lights[1]->GetBrightness() );

	shaderConsts->SetFloatArray( g_pd3dDevice, "MaterialColour", (FLOAT*)&m_DiffuseColour, 3 ); // If needed
	shaderConsts->SetFloatArray( g_pd3dDevice, "SpecularStrength", (FLOAT*)&m_SpecularColour, 3 ); // If needed
	shaderConsts->SetFloat( g_pd3dDevice, "SpecularPower", m_SpecularPower );
}

// Function for pixel shader that draws alpha blended objects and switches off z-buffer for manual sorting
void PS_ManualAlphaFn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	// Start with default render states
	SetDefaultStates();

	// Enable alpha blending
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );

	// Alpha blending for Earth clouds causes sorting issues - switch off the z-buffer and sort manually
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
}

// Special function for atmosphere pixel shader, similar to above, but renders back faces
void PS_AtmosphereFn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	// Start with default render states
	SetDefaultStates();

	// Enable reverse culling - i.e. render back faces of model - atmosphere is drawn behind the planet
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CW );

	// Enable alpha blending
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
	
	// Recall that alpha blending causes sorting issues - in this case we switch off the z-buffer and sort manually
	g_pd3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_FALSE );
}

// Pass data to pixel shaders that perform pixel lighting (2 point lights) on 2-sided alpha blended polygons
void PS_AlphaPixLit2Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	// Nothing to do, just ensure we have default render states
	SetDefaultStates();

	// Disable culling - i.e. 2-sided
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	// Enable alpha blending
	g_pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
	g_pd3dDevice->SetRenderState( D3DRS_SRCBLEND, D3DBLEND_SRCALPHA );
	g_pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
}


// Pass data to pixel shaders that perform pixel lighting (2 point lights) on 2-sided alpha blended polygons
void PS_CutoutPixLit2Fn( int method, CMatrix4x4* worldMatrix, CCamera* camera )
{
	// Nothing to do, just ensure we have default render states
	SetDefaultStates();

	// Disable culling - i.e. 2-sided
	g_pd3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	// Enable alpha testing
	g_pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, TRUE ); 
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAREF, 106 );
	g_pd3dDevice->SetRenderState( D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL );
}

} // namespace gen