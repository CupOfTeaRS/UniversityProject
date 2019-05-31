/*******************************************
	GameData.h

	Main scene and game functions
********************************************/

#pragma once

namespace gen
{

///////////////////////////////
// Scene management

// Creates the scene geometry
bool SceneSetup();
void PreRenderScene();
// Release everything in the scene
void SceneShutdown();

///////////////////////////////
// Game loop functions

// Draw one frame of the scene
void RenderScene( float updateTime );
bool MainMenu(float updateTime);
// Render on-screen text each frame
void RenderSceneText( float updateTime );

// Update the scene between rendering
void UpdateScene( float updateTime );

} // namespace gen
