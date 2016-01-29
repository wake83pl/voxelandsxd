/************************************************************************
* Minetest-c55
* Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* light.h
* voxelands - 3d voxel world sandbox game
* Copyright (C) Lisa 'darkrose' Milne 2014 <lisa@ltmnet.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful, but
* WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
* See the GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>
*
* License updated from GPLv2 or later to GPLv3 or later by Lisa Milne
* for Voxelands.
************************************************************************/

#ifndef LIGHT_HEADER
#define LIGHT_HEADER

#include "common_irrlicht.h"
#include "debug.h"

/*
	Day/night cache:
	Meshes are cached for different day-to-night transition values
*/

/*#define DAYNIGHT_CACHE_COUNT 3
// First one is day, last one is night.
extern u32 daynight_cache_ratios[DAYNIGHT_CACHE_COUNT];*/

/*
	Lower level lighting stuff
*/

// This directly sets the range of light.
// Actually this is not the real maximum, and this is not the
// brightest. The brightest is LIGHT_SUN.
#define LIGHT_MAX 14
// Light is stored as 4 bits, thus 15 is the maximum.
// This brightness is reserved for sunlight
#define LIGHT_SUN 15

// these are used in mob spawning
#define LIGHT_SPAWN_BRIGHT 10
#define LIGHT_SPAWN_DARK 4

inline u8 diminish_light(u8 light)
{
	if(light == 0)
		return 0;
	if(light >= LIGHT_MAX)
		return LIGHT_MAX - 1;

	return light - 1;
}

inline u8 diminish_light(u8 light, u8 distance)
{
	if(distance >= light)
		return 0;
	return  light - distance;
}

inline u8 undiminish_light(u8 light)
{
	// We don't know if light should undiminish from this particular 0.
	// Thus, keep it at 0.
	if(light == 0)
		return 0;
	if(light == LIGHT_MAX)
		return light;

	return light + 1;
}

extern u8 light_decode_table[LIGHT_MAX+1];
extern u8 light_ambient_decode_table[LIGHT_MAX+1];

inline u8 decode_light(u8 light)
{
	if(light == LIGHT_SUN)
		return light_decode_table[LIGHT_MAX];

	if(light > LIGHT_MAX)
		light = LIGHT_MAX;

	return light_decode_table[light];
}

inline u8 decode_ambient_light(u8 light)
{
	if(light == LIGHT_SUN)
		return light_ambient_decode_table[LIGHT_MAX];

	if(light > LIGHT_MAX)
		light = LIGHT_MAX;

	return light_ambient_decode_table[light];
}

// 0 <= daylight_factor <= 1000
// 0 <= lightday, lightnight <= LIGHT_SUN
// 0 <= return value <= LIGHT_SUN
inline u8 blend_light(u32 daylight_factor, u8 lightday, u8 lightnight)
{
	u32 c = 1000;
	u32 l = ((daylight_factor * lightday + (c-daylight_factor) * lightnight))/c;
	if(l > LIGHT_SUN)
		l = LIGHT_SUN;
	return l;
}

#ifndef SERVER

#include <ILightManager.h>
#include <ILightSceneNode.h>
#include <vector>
#include <map>
#include "utility.h"

struct LightData {
	v3s16 pos;
	v3f pos_f;
	float radius;

	LightData():
		radius(0)
	{}

	LightData(v3s16 apos, float aradius):
		pos(apos),
		radius(aradius)
	{
		pos_f = intToFloat(pos,BS);
	}
};

class CMyLightManager : public scene::ILightManager
{
	// These data represent the state information that this light manager
	// is interested in.
	scene::ISceneManager * SceneManager;
	core::array<scene::ISceneNode*> * SceneLightList;
	scene::E_SCENE_NODE_RENDER_PASS CurrentRenderPass;
	scene::ISceneNode * CurrentSceneNode;

public:
	CMyLightManager(scene::ISceneManager* sceneManager):
		SceneManager(sceneManager),
		SceneLightList(0),
		CurrentRenderPass(scene::ESNRP_NONE),
		CurrentSceneNode(0)
	{
		for (int i=0; i<7; i++) {
			lights[i] = sceneManager->addLightSceneNode();
			lights[i]->setLightType(video::ELT_POINT);
		}
	}

	// This is called before the first scene node is rendered.
	virtual void OnPreRender(core::array<scene::ISceneNode*> & lightList)
	{
		// Store the light list. I am free to alter this list until the end of OnPostRender().
		SceneLightList = &lightList;
		if (SceneLightList->size() < 8) {
			for (int i=0; i<7; i++) {
				lights[i] = SceneManager->addLightSceneNode();
				lights[i]->setLightType(video::ELT_POINT);
				lights[i]->setVisible(false);
			}
		}
	}

	// Called after the last scene node is rendered.
	virtual void OnPostRender() {}

	virtual void OnRenderPassPreRender(scene::E_SCENE_NODE_RENDER_PASS renderPass)
	{
		// I don't have to do anything here except remember which render pass I am in.
		CurrentRenderPass = renderPass;
	}

	virtual void OnRenderPassPostRender(scene::E_SCENE_NODE_RENDER_PASS renderPass)
	{
		// I only want solid nodes to be lit, so after the solid pass, turn all lights off.
		//if (scene::ESNRP_SOLID == renderPass) {
			//for (u32 i = 0; i < SceneLightList->size(); ++i)
				//(*SceneLightList)[i]->setVisible(false);
		//}
	}

	// This is called before the specified scene node is rendered
	virtual void OnNodePreRender(scene::ISceneNode* node);

	// Called after the specified scene node is rendered
	virtual void OnNodePostRender(scene::ISceneNode* node)
	{
		// I don't need to do any light management after individual node rendering.
	}

	// this should enable the lights in a block, then gather the lights in 3x3x3 blocks
	// around it, find the ones that would touch the block, and enable them in order of
	// closeness to the block - maximum of 8, but 0 is the sun, and 1 is possibly a wield
	// light
	void enableBlockLights(v3s16 blockpos);

	void setBlockLights(v3s16 blockpos, std::vector<LightData> lights)
	{
		block_lights[blockpos] = lights;
	}

	void clearBlockLights()
	{
		block_lights.clear();
	}

	void updateCameraOffset(v3s16 offset)
	{
		camera_offset = intToFloat(offset,BS);
	}

private:

	std::map<v3s16,std::vector<LightData> > block_lights;
	v3f camera_offset;
	scene::ILightSceneNode *lights[7];

	// A utility class to aid in sorting scene nodes into a distance order
	class LightDistanceElement
	{
	public:
		LightDistanceElement() {};

		LightDistanceElement(scene::ISceneNode* n, f64 d)
			: node(n), distance(d) { }

		scene::ISceneNode* node;
		f64 distance;

		// Lower distance elements are sorted to the start of the array
		bool operator < (const LightDistanceElement& other) const
		{
			return (distance < other.distance);
		}
	};
};

#endif

#endif

