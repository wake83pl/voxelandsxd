/************************************************************************
* Minetest-c55
* Copyright (C) 2010 celeron55, Perttu Ahola <celeron55@gmail.com>
*
* light.cpp
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

#include "light.h"

//#if 1
u8 light_decode_table[LIGHT_MAX+1] = {
	1,
	1,
	1,
	1,
	1,
	1,
	1,
	2,
	3,
	7,
	15,
	31,
	63,
	127,
	255,
};
u8 light_ambient_decode_table[LIGHT_MAX+1] =
{
	8,
	11,
	14,
	18,
	22,
	29,
	37,
	47,
	60,
	76,
	97,
	123,
	157,
	200,
	255,
};
//#elif 1
//// This is good
//// a_n+1 = a_n * 0.786
//// Length of LIGHT_MAX+1 means LIGHT_MAX is the last value.
//// LIGHT_SUN is read as LIGHT_MAX from here.
//u8 light_decode_table[LIGHT_MAX+1] =
//{
//8,
//11,
//14,
//18,
//22,
//29,
//37,
//47,
//60,
//76,
//97,
//123,
//157,
//200,
//255,
//};
//#else
//// Use for debugging in dark
//u8 light_decode_table[LIGHT_MAX+1] =
//{
//58,
//64,
//72,
//80,
//88,
//98,
//109,
//121,
//135,
//150,
//167,
//185,
//206,
//229,
//255,
//};
//#endif

#ifndef SERVER

#include "mapblock.h"

void CMyLightManager::OnNodePreRender(scene::ISceneNode* node)
{
	CurrentSceneNode = node;

	//if (!node)
		return;

	// This light manager only considers solid objects, but you are free to manipulate
	// lights during any phase, depending on your requirements.
	//if (scene::ESNRP_SOLID != CurrentRenderPass)
		//return;

	// And in fact for this example, I only want to consider lighting for cube scene
	// nodes.  You will probably want to deal with lighting for (at least) mesh /
	// animated mesh scene nodes as well.
	//if (node->getType() != scene::ESNT_CUBE)
		//return;

	{
		// This is a naive implementation that prioritises every light in the scene
		// by its proximity to the node being rendered.  This produces some flickering
		// when lights orbit closer to a cube than its 'zone' lights.
		const v3f nodePosition = node->getAbsolutePosition();
		v3s16 pos = floatToInt(nodePosition-camera_offset,BS);
		enableBlockLights(getNodeBlockPos(pos));

		// Sort the light list by prioritising them based on their distance from the node
		// that's about to be rendered.
		//core::array<LightDistanceElement> sortingArray;
		//sortingArray.reallocate(SceneLightList->size());

		//for (u32 i=0; i<SceneLightList->size(); ++i) {
			//scene::ISceneNode* lightNode = (*SceneLightList)[i];
			//const f64 distance = lightNode->getAbsolutePosition().getDistanceFromSQ(nodePosition);
			//video::SLight & lightData = ((scene::ILightSceneNode*)lightNode)->getLightData();

			//if (video::ELT_DIRECTIONAL == lightData.Type)
				//continue;
			//sortingArray.push_back(LightDistanceElement(lightNode, distance));
		//}

		//sortingArray.sort();

		//// The list is now sorted by proximity to the node.
		//// Turn on the three nearest lights, and turn the others off.
		//for (u32 i=0; i<sortingArray.size(); ++i) {
			//sortingArray[i].node->setVisible(i < 3);
		//}
	}
}

// this should enable the lights in a block, then gather the lights in 3x3x3 blocks
// around it, find the ones that would touch the block, and enable them in order of
// closeness to the block - maximum of 8, but 0 is the sun, and 1 is possibly a wield
// light
void CMyLightManager::enableBlockLights(v3s16 blockpos)
{
	std::vector<LightData> blights;

	blights = block_lights[blockpos];
	for (std::vector<LightData>::iterator i=block_lights[blockpos+v3s16(0,1,0)].begin(); i!=block_lights[blockpos+v3s16(0,1,0)].end(); i++) {
		LightData l = *i;
		l.pos_f.Y += 16*BS;
		blights.push_back(l);
	}
	for (std::vector<LightData>::iterator i=block_lights[blockpos-v3s16(0,1,0)].begin(); i!=block_lights[blockpos-v3s16(0,1,0)].end(); i++) {
		LightData l = *i;
		l.pos_f.Y -= 16*BS;
		blights.push_back(l);
	}

	v3f pos = intToFloat(blockpos*MAP_BLOCKSIZE,BS);
	for (u32 i=0; i<7; i++) {
		if (i>=blights.size()) {
			lights[i]->setVisible(false);
			continue;
		}
		LightData l = blights[i];
		lights[i]->setVisible(true);
		lights[i]->setLightType(video::ELT_POINT);
		lights[i]->setPosition((l.pos_f+pos-camera_offset));
		lights[i]->setRadius(l.radius);
	}
}

#endif
