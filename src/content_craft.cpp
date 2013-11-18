/*
Minetest-c55
Copyright (C) 2010-2011 celeron55, Perttu Ahola <celeron55@gmail.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License along
with this program; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#include "content_craft.h"
#include "inventory.h"
#include "content_mapnode.h"
#include "player.h"
#include "mapnode.h" // For content_t
#include "settings.h" // for g_settings

struct CraftDef {
	ItemSpec specs[9];
	InventoryItem *item;

	bool operator==(InventoryItem const * const *items)
	{
		u16 items_min_x = 100;
		u16 items_max_x = 100;
		u16 items_min_y = 100;
		u16 items_max_y = 100;
		for(u16 y=0; y<3; y++)
		for(u16 x=0; x<3; x++)
		{
			if(items[y*3 + x] == NULL)
				continue;
			if(items_min_x == 100 || x < items_min_x)
				items_min_x = x;
			if(items_min_y == 100 || y < items_min_y)
				items_min_y = y;
			if(items_max_x == 100 || x > items_max_x)
				items_max_x = x;
			if(items_max_y == 100 || y > items_max_y)
				items_max_y = y;
		}
		// No items at all, just return false
		if(items_min_x == 100)
			return false;

		u16 items_w = items_max_x - items_min_x + 1;
		u16 items_h = items_max_y - items_min_y + 1;

		u16 specs_min_x = 100;
		u16 specs_max_x = 100;
		u16 specs_min_y = 100;
		u16 specs_max_y = 100;
		for(u16 y=0; y<3; y++)
		for(u16 x=0; x<3; x++)
		{
			if(specs[y*3 + x].type == ITEM_NONE)
				continue;
			if(specs_min_x == 100 || x < specs_min_x)
				specs_min_x = x;
			if(specs_min_y == 100 || y < specs_min_y)
				specs_min_y = y;
			if(specs_max_x == 100 || x > specs_max_x)
				specs_max_x = x;
			if(specs_max_y == 100 || y > specs_max_y)
				specs_max_y = y;
		}
		// No specs at all, just return false
		if(specs_min_x == 100)
			return false;

		u16 specs_w = specs_max_x - specs_min_x + 1;
		u16 specs_h = specs_max_y - specs_min_y + 1;

		// Different sizes
		if(items_w != specs_w || items_h != specs_h)
			return false;

		for(u16 y=0; y<specs_h; y++)
		for(u16 x=0; x<specs_w; x++)
		{
			u16 items_x = items_min_x + x;
			u16 items_y = items_min_y + y;
			u16 specs_x = specs_min_x + x;
			u16 specs_y = specs_min_y + y;
			const InventoryItem *item = items[items_y * 3 + items_x];
			const ItemSpec &spec = specs[specs_y * 3 + specs_x];

			if(spec.checkItem(item) == false)
				return false;
		}

		return true;
	}
};

/*
	items: actually *items[9]
	return value: allocates a new item, or returns NULL.
*/
InventoryItem *craft_get_result(InventoryItem **items)
{
	static CraftDef defs[223];
	static int defs_init = 0;

	// only initialise (and hence allocate) these once
	if (!defs_init) {
		// Wood
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_TREE);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD, 4);
			defs_init++;
		}

		// Junglewood
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLETREE);
			defs[defs_init].item = new MaterialItem(CONTENT_JUNGLEWOOD, 4);
			defs_init++;
		}

		// Stick from wood
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new CraftItem("Stick", 4);
			defs_init++;
		}

		// Stick from jungle wood
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].item = new CraftItem("Stick", 4);
			defs_init++;
		}

		// Fence
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_FENCE, 2);
			defs_init++;
		}

		// Sign from wood
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_SIGN, 1);
			defs_init++;
		}

		// Sign from jungle wood
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_SIGN, 1);
			defs_init++;
		}

		// Torch
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_TORCH, 4);
			defs_init++;
		}

		// Wooden pick
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("WPick", 0);
			defs_init++;
		}

		// Stone pick
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("STPick", 0);
			defs_init++;
		}

		// Steel pick
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("SteelPick", 0);
			defs_init++;
		}

		// Mese pick
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("MesePick", 0);
			defs_init++;
		}

		// Wooden shovel
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("WShovel", 0);
			defs_init++;
		}

		// Stone shovel
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("STShovel", 0);
			defs_init++;
		}

		// Steel shovel
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("SteelShovel", 0);
			defs_init++;
		}

		// Wooden axe
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("WAxe", 0);
			defs_init++;
		}

		// Stone axe
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("STAxe", 0);
			defs_init++;
		}

		// Steel axe
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("SteelAxe", 0);
			defs_init++;
		}

		// Shears
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("Shears", 0);
			defs_init++;
		}

		// Wooden sword
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("WSword", 0);
			defs_init++;
		}

		// Stone sword
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("STSword", 0);
			defs_init++;
		}

		// Steel sword
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("SteelSword", 0);
			defs_init++;
		}

		// Rail
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].item = new MaterialItem(CONTENT_RAIL, 15);
			defs_init++;
		}

		// Chest
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_CHEST, 1);
			defs_init++;
		}

		// Locking Chest
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_LOCKABLE_CHEST, 1);
			defs_init++;
		}

		// Converted Locking Chest
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CHEST);
			defs[defs_init].item = new MaterialItem(CONTENT_LOCKABLE_CHEST, 1);
			defs_init++;
		}

		// Border Stone
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_MESE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].item = new MaterialItem(CONTENT_BORDERSTONE, 1);
			defs_init++;
		}

		// Furnace
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_FURNACE, 1);
			defs_init++;
		}

		// Cobble
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_COBBLE, 5);
			defs_init++;
		}

		// Steel block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL, 1);
			defs_init++;
		}

		// Uncrafted Steel block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL);
			defs[defs_init].item = new CraftItem("steel_ingot", 9);
			defs_init++;
		}

		// copper block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "copper_ingot");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "copper_ingot");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "copper_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "copper_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "copper_ingot");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "copper_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "copper_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "copper_ingot");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "copper_ingot");
			defs[defs_init].item = new MaterialItem(CONTENT_COPPER, 1);
			defs_init++;
		}

		// Uncrafted Copper block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COPPER);
			defs[defs_init].item = new CraftItem("copper_ingot", 9);
			defs_init++;
		}

		// gold block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "gold_ingot");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "gold_ingot");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "gold_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "gold_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "gold_ingot");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "gold_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "gold_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "gold_ingot");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "gold_ingot");
			defs[defs_init].item = new MaterialItem(CONTENT_GOLD, 1);
			defs_init++;
		}

		// Uncrafted gold block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GOLD);
			defs[defs_init].item = new CraftItem("gold_ingot", 9);
			defs_init++;
		}

		// silver block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "silver_ingot");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "silver_ingot");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "silver_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "silver_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "silver_ingot");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "silver_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "silver_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "silver_ingot");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "silver_ingot");
			defs[defs_init].item = new MaterialItem(CONTENT_SILVER, 1);
			defs_init++;
		}

		// Uncrafted silver block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_SILVER);
			defs[defs_init].item = new CraftItem("silver_ingot", 9);
			defs_init++;
		}

		// tin block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].item = new MaterialItem(CONTENT_TIN, 1);
			defs_init++;
		}

		// Uncrafted tin block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_TIN);
			defs[defs_init].item = new CraftItem("tin_ingot", 9);
			defs_init++;
		}

		// quartz block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].item = new MaterialItem(CONTENT_QUARTZ, 1);
			defs_init++;
		}

		// Uncrafted quartz block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_QUARTZ);
			defs[defs_init].item = new CraftItem("lump_of_quartz", 9);
			defs_init++;
		}

		// Sandstone
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			defs[defs_init].item = new MaterialItem(CONTENT_SANDSTONE, 1);
			defs_init++;
		}

		// Clay
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			defs[defs_init].item = new MaterialItem(CONTENT_CLAY, 1);
			defs_init++;
		}

		// Brick
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "clay_brick");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "clay_brick");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "clay_brick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "clay_brick");
			defs[defs_init].item = new MaterialItem(CONTENT_BRICK, 1);
			defs_init++;
		}

		// Cotton
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEGRASS);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEGRASS);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEGRASS);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEGRASS);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON, 1);
			defs_init++;
		}

		// Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_white");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLUE);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON, 1);
			defs_init++;
		}

		// Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_white");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_GREEN);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON, 1);
			defs_init++;
		}

		// Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_white");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_ORANGE);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON, 1);
			defs_init++;
		}

		// Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_white");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_PURPLE);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON, 1);
			defs_init++;
		}

		// Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_white");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_RED);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON, 1);
			defs_init++;
		}

		// Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_white");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_YELLOW);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON, 1);
			defs_init++;
		}

		// Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_white");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLACK);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON, 1);
			defs_init++;
		}

		// blue Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_blue");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON_BLUE, 1);
			defs_init++;
		}

		// green Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_green");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON_GREEN, 1);
			defs_init++;
		}

		// orange Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_orange");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON_ORANGE, 1);
			defs_init++;
		}

		// purple Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_purple");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON_PURPLE, 1);
			defs_init++;
		}

		// red Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_red");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON_RED, 1);
			defs_init++;
		}

		// yellow Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_yellow");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON_YELLOW, 1);
			defs_init++;
		}

		// black Cotton
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_black");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].item = new MaterialItem(CONTENT_COTTON_BLACK, 1);
			defs_init++;
		}

		// Paper
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_PAPYRUS);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_PAPYRUS);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_PAPYRUS);
			defs[defs_init].item = new CraftItem("paper", 1);
			defs_init++;
		}

		// Book
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "paper");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "paper");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "paper");
			defs[defs_init].item = new CraftItem("book", 1);
			defs_init++;
		}

		// Book shelf
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "book");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "book");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "book");
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_BOOKSHELF, 1);
			defs_init++;
		}

		// Ladder
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_LADDER, 1);
			defs_init++;
		}

		// Iron Apple
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "apple");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].item = new CraftItem("apple_iron", 1);
			defs_init++;
		}

		// roughstone slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_ROUGHSTONE_SLAB, 3);
			defs_init++;
		}

		// cobble slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].item = new MaterialItem(CONTENT_COBBLE_SLAB, 3);
			defs_init++;
		}

		// mossy cobble slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].item = new MaterialItem(CONTENT_MOSSYCOBBLE_SLAB, 3);
			defs_init++;
		}

		// stone slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].item = new MaterialItem(CONTENT_STONE_SLAB, 3);
			defs_init++;
		}

		// wood slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_SLAB, 3);
			defs_init++;
		}

		// jungle slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_JUNGLE_SLAB, 3);
			defs_init++;
		}

		// brick slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].item = new MaterialItem(CONTENT_BRICK_SLAB, 3);
			defs_init++;
		}

		// sandstone slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_SANDSTONE_SLAB, 3);
			defs_init++;
		}

		// glass slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_SLAB, 3);
			defs_init++;
		}

		// blue glass slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLUE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLUE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLUE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_BLUE_SLAB, 3);
			defs_init++;
		}

		// green glass slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_GREEN);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_GREEN);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_GREEN);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_GREEN_SLAB, 3);
			defs_init++;
		}

		// orange glass slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_ORANGE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_ORANGE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_ORANGE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_ORANGE_SLAB, 3);
			defs_init++;
		}

		// purple glass slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PURPLE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PURPLE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PURPLE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PURPLE_SLAB, 3);
			defs_init++;
		}

		// red glass slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_RED);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_RED);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_RED);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_RED_SLAB, 3);
			defs_init++;
		}

		// yellow glass slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_YELLOW);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_YELLOW);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_YELLOW);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_YELLOW_SLAB, 3);
			defs_init++;
		}

		// black glass slabs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLACK);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLACK);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLACK);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_BLACK_SLAB, 3);
			defs_init++;
		}

		// roughstone stairs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_ROUGHSTONE_STAIR, 6);
			defs_init++;
		}

		// roughstone stairs
		{
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_ROUGHSTONE_STAIR, 6);
			defs_init++;
		}

		// cobble stairs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].item = new MaterialItem(CONTENT_COBBLE_STAIR, 6);
			defs_init++;
		}

		// cobble stairs
		{
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].item = new MaterialItem(CONTENT_COBBLE_STAIR, 6);
			defs_init++;
		}

		// mossy cobble stairs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].item = new MaterialItem(CONTENT_MOSSYCOBBLE_STAIR, 6);
			defs_init++;
		}

		// mossy cobble stairs
		{
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].item = new MaterialItem(CONTENT_MOSSYCOBBLE_STAIR, 6);
			defs_init++;
		}

		// stone stairs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].item = new MaterialItem(CONTENT_STONE_STAIR, 6);
			defs_init++;
		}

		// stone stairs
		{
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].item = new MaterialItem(CONTENT_STONE_STAIR, 6);
			defs_init++;
		}

		// wood stairs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_STAIR, 6);
			defs_init++;
		}

		// wood stairs
		{
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_STAIR, 6);
			defs_init++;
		}

		// jungle stairs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_JUNGLE_STAIR, 6);
			defs_init++;
		}

		// jungle stairs
		{
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_JUNGLE_STAIR, 6);
			defs_init++;
		}

		// brick stairs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].item = new MaterialItem(CONTENT_BRICK_STAIR, 6);
			defs_init++;
		}

		// brick stairs
		{
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_BRICK);
			defs[defs_init].item = new MaterialItem(CONTENT_BRICK_STAIR, 6);
			defs_init++;
		}

		// sandstone stairs
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_SANDSTONE_STAIR, 6);
			defs_init++;
		}

		// sandstone stairs
		{
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_SANDSTONE_STAIR, 6);
			defs_init++;
		}

		// glass light
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_TORCH);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASSLIGHT, 4);
			defs_init++;
		}

		// stone brick
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_STONEBRICK, 4);
			defs_init++;
		}

		// stone brick
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].item = new MaterialItem(CONTENT_STONEBRICK, 4);
			defs_init++;
		}

		// stone block
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].item = new MaterialItem(CONTENT_STONEBLOCK, 4);
			defs_init++;
		}

		// stone brick
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].item = new MaterialItem(CONTENT_STONEBRICK, 4);
			defs_init++;
		}

		// stone brick
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].item = new MaterialItem(CONTENT_STONEBRICK, 4);
			defs_init++;
		}

		// roughstone brick
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_ROUGHSTONEBRICK, 4);
			defs_init++;
		}

		// roughstone brick
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_ROUGHSTONEBRICK, 4);
			defs_init++;
		}

		// roughstone block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_ROUGHSTONEBLOCK, 4);
			defs_init++;
		}

		// wood door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_DOOR_LT, 1);
			defs_init++;
		}

		// wood door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_DOOR_LT, 1);
			defs_init++;
		}

		// glass door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_DOOR_LT, 1);
			defs_init++;
		}

		// steel door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_DOOR_LT, 1);
			defs_init++;
		}

		// wood windowed door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_DOOR_LT);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_W_DOOR_LT, 1);
			defs_init++;
		}

		// steel windowed door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL_DOOR_LT);
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_W_DOOR_LT, 1);
			defs_init++;
		}

		// wood windowed door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_DOOR_RT);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_W_DOOR_RT, 1);
			defs_init++;
		}

		// steel windowed door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL_DOOR_RT);
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_W_DOOR_RT, 1);
			defs_init++;
		}

		// wood opposite door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_DOOR_LT);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_DOOR_RT, 1);
			defs_init++;
		}

		// glass opposite door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_DOOR_LT);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_DOOR_RT, 1);
			defs_init++;
		}

		// steel opposite door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL_DOOR_LT);
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_DOOR_RT, 1);
			defs_init++;
		}

		// wood !opposite door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_DOOR_RT);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_DOOR_LT, 1);
			defs_init++;
		}

		// glass !opposite door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_DOOR_RT);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_DOOR_LT, 1);
			defs_init++;
		}

		// steel !opposite door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL_DOOR_RT);
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_DOOR_LT, 1);
			defs_init++;
		}

		// wood opposite windowed door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_W_DOOR_LT);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_W_DOOR_RT, 1);
			defs_init++;
		}

		// steel opposite windowed door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL_W_DOOR_LT);
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_W_DOOR_RT, 1);
			defs_init++;
		}

		// wood !opposite windowed door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_W_DOOR_RT);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_W_DOOR_LT, 1);
			defs_init++;
		}

		// steel !opposite windowed door
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL_W_DOOR_RT);
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_W_DOOR_LT, 1);
			defs_init++;
		}

		// wood hatch
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_HATCH, 1);
			defs_init++;
		}

		// wood hatch
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_HATCH, 1);
			defs_init++;
		}

		// steel hatch
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_HATCH, 1);
			defs_init++;
		}

		// wood windowed hatch
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_HATCH);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_W_HATCH, 1);
			defs_init++;
		}

		// steel windowed hatch
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL_HATCH);
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_W_HATCH, 1);
			defs_init++;
		}

		// wood gate
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_GATE, 1);
			defs_init++;
		}

		// wood gate
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_GATE, 1);
			defs_init++;
		}

		// steel gate
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL);
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_STEEL);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].item = new MaterialItem(CONTENT_STEEL_GATE, 1);
			defs_init++;
		}

		// flower pot
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "lump_of_clay");
			defs[defs_init].item = new MaterialItem(CONTENT_FLOWER_POT_RAW, 1);
			defs_init++;
		}

		// yellow dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_FLOWER_DAFFODIL);
			defs[defs_init].item = new CraftItem("dye_yellow", 2);
			defs_init++;
		}

		// red dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_FLOWER_ROSE);
			defs[defs_init].item = new CraftItem("dye_red", 2);
			defs_init++;
		}

		// blue dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_FLOWER_TULIP);
			defs[defs_init].item = new CraftItem("dye_blue", 2);
			defs_init++;
		}

		// green dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_blue");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "dye_yellow");
			defs[defs_init].item = new CraftItem("dye_green", 2);
			defs_init++;
		}

		// green dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_yellow");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "dye_blue");
			defs[defs_init].item = new CraftItem("dye_green", 2);
			defs_init++;
		}

		// orange dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_yellow");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "dye_red");
			defs[defs_init].item = new CraftItem("dye_orange", 2);
			defs_init++;
		}

		// orange dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_red");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "dye_yellow");
			defs[defs_init].item = new CraftItem("dye_orange", 2);
			defs_init++;
		}

		// purple dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_red");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "dye_blue");
			defs[defs_init].item = new CraftItem("dye_purple", 2);
			defs_init++;
		}

		// purple dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_blue");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "dye_red");
			defs[defs_init].item = new CraftItem("dye_purple", 2);
			defs_init++;
		}

		// white dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_blue");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "dye_yellow");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "dye_red");
			defs[defs_init].item = new CraftItem("dye_white", 2);
			defs_init++;
		}

		// black dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_white");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].item = new CraftItem("dye_black", 2);
			defs_init++;
		}

		// black dye
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "dye_white");
			defs[defs_init].item = new CraftItem("dye_black", 2);
			defs_init++;
		}

		// Wooden bucket
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new ToolItem("WBucket", 0);
			defs_init++;
		}

		// Steel bucket
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].item = new ToolItem("SteelBucket", 0);
			defs_init++;
		}

		// Tin bucket
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "tin_ingot");
			defs[defs_init].item = new ToolItem("TinBucket", 0);
			defs_init++;
		}

		// Roughstone wall
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_ROUGHSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_ROUGHSTONE_WALL, 6);
			defs_init++;
		}

		// Cobble wall
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_COBBLE);
			defs[defs_init].item = new MaterialItem(CONTENT_COBBLE_WALL, 6);
			defs_init++;
		}

		// MossyCobble wall
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_MOSSYCOBBLE);
			defs[defs_init].item = new MaterialItem(CONTENT_MOSSYCOBBLE_WALL, 6);
			defs_init++;
		}

		// Stone wall
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_STONE);
			defs[defs_init].item = new MaterialItem(CONTENT_STONE_WALL, 6);
			defs_init++;
		}

		// Sandstone wall
		{
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_SANDSTONE_WALL, 6);
			defs_init++;
		}

		// hay bale
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_DEADGRASS);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_DEADGRASS);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_DEADGRASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_DEADGRASS);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_DEADGRASS);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_DEADGRASS);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_DEADGRASS);
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_DEADGRASS);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_DEADGRASS);
			defs[defs_init].item = new MaterialItem(CONTENT_HAY, 1);
			defs_init++;
		}

		// sandstone block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_SANDSTONE_BLOCK, 4);
			defs_init++;
		}

		// sandstone brick
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_SANDSTONE_BRICK, 4);
			defs_init++;
		}

		// sandstone brick
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_SANDSTONE);
			defs[defs_init].item = new MaterialItem(CONTENT_SANDSTONE_BRICK, 4);
			defs_init++;
		}

		// terracotta block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].item = new MaterialItem(CONTENT_TERRACOTTA_BLOCK, 4);
			defs_init++;
		}

		// terracotta brick
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].item = new MaterialItem(CONTENT_TERRACOTTA_BRICK, 4);
			defs_init++;
		}

		// terracotta brick
		{
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].item = new MaterialItem(CONTENT_TERRACOTTA_BRICK, 4);
			defs_init++;
		}

		// terracotta tile
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].specs[8] = ItemSpec(ITEM_MATERIAL, CONTENT_TERRACOTTA);
			defs[defs_init].item = new MaterialItem(CONTENT_TERRACOTTA_TILE, 5);
			defs_init++;
		}

		// blue clay
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_blue");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CLAY);
			defs[defs_init].item = new MaterialItem(CONTENT_CLAY_BLUE, 1);
			defs_init++;
		}

		// green clay
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_green");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CLAY);
			defs[defs_init].item = new MaterialItem(CONTENT_CLAY_GREEN, 1);
			defs_init++;
		}

		// orange clay
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_orange");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CLAY);
			defs[defs_init].item = new MaterialItem(CONTENT_CLAY_ORANGE, 1);
			defs_init++;
		}

		// purple clay
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_purple");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CLAY);
			defs[defs_init].item = new MaterialItem(CONTENT_CLAY_PURPLE, 1);
			defs_init++;
		}

		// red clay
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_red");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CLAY);
			defs[defs_init].item = new MaterialItem(CONTENT_CLAY_RED, 1);
			defs_init++;
		}

		// yellow clay
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_yellow");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CLAY);
			defs[defs_init].item = new MaterialItem(CONTENT_CLAY_YELLOW, 1);
			defs_init++;
		}

		// black clay
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_black");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CLAY);
			defs[defs_init].item = new MaterialItem(CONTENT_CLAY_BLACK, 1);
			defs_init++;
		}

		// blue glass
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_blue");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_BLUE, 1);
			defs_init++;
		}

		// green glass
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_green");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_GREEN, 1);
			defs_init++;
		}

		// orange glass
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_orange");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_ORANGE, 1);
			defs_init++;
		}

		// purple glass
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_purple");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PURPLE, 1);
			defs_init++;
		}

		// red glass
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_red");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_RED, 1);
			defs_init++;
		}

		// yellow glass
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_yellow");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_YELLOW, 1);
			defs_init++;
		}

		// black glass
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_black");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_BLACK, 1);
			defs_init++;
		}

		// Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET, 3);
			defs_init++;
		}

		// blue Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLUE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLUE);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_BLUE, 3);
			defs_init++;
		}

		// blue Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_blue");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CARPET);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_BLUE, 1);
			defs_init++;
		}

		// green Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_GREEN);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_GREEN);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_GREEN, 3);
			defs_init++;
		}

		// green Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_green");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CARPET);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_GREEN, 1);
			defs_init++;
		}

		// orange Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_ORANGE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_ORANGE);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_ORANGE, 3);
			defs_init++;
		}

		// orange Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_orange");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CARPET);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_ORANGE, 1);
			defs_init++;
		}

		// purple Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_PURPLE);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_PURPLE);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_PURPLE, 3);
			defs_init++;
		}

		// purple Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_purple");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CARPET);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_PURPLE, 1);
			defs_init++;
		}

		// red Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_RED);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_RED);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_RED, 3);
			defs_init++;
		}

		// red Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_red");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CARPET);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_RED, 1);
			defs_init++;
		}

		// yellow Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_YELLOW);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_YELLOW);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_YELLOW, 3);
			defs_init++;
		}

		// yellow Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_yellow");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CARPET);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_YELLOW, 1);
			defs_init++;
		}

		// black Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLACK);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLACK);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_BLACK, 3);
			defs_init++;
		}

		// black Carpet
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "dye_black");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_CARPET);
			defs[defs_init].item = new MaterialItem(CONTENT_CARPET_BLACK, 1);
			defs_init++;
		}

		// Coal block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "lump_of_coal");
			defs[defs_init].item = new MaterialItem(CONTENT_COAL, 1);
			defs_init++;
		}

		// Charoal block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].item = new MaterialItem(CONTENT_CHARCOAL, 1);
			defs_init++;
		}

		// fire starter
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_flint");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "steel_ingot");
			defs[defs_init].item = new ToolItem("FireStarter", 0);
			defs_init++;
		}

		// quartz dust
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_quartz");
			defs[defs_init].item = new CraftItem("quartz_dust", 1);
			defs_init++;
		}

		// gun powder
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "lump_of_charcoal");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "saltpeter");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "lump_of_flint");
			defs[defs_init].item = new CraftItem("gunpowder", 2);
			defs_init++;
		}

		// tnt
		if (g_settings->getBool("enable_tnt")) {
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "gunpowder");
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "gunpowder");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "gunpowder");
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "gunpowder");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_SAND);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "gunpowder");
			defs[defs_init].item = new MaterialItem(CONTENT_TNT, 1);
			defs_init++;
		}

		// glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE, 3);
			defs_init++;
		}

		// blue glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLUE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLUE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLUE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_BLUE, 3);
			defs_init++;
		}

		// blue glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "blue_dye");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PANE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_BLUE, 1);
			defs_init++;
		}

		// green glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_GREEN);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_GREEN);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_GREEN);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_GREEN, 3);
			defs_init++;
		}

		// green glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "green_dye");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PANE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_GREEN, 1);
			defs_init++;
		}

		// orange glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_ORANGE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_ORANGE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_ORANGE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_ORANGE, 3);
			defs_init++;
		}

		// orange glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "orange_dye");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PANE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_ORANGE, 1);
			defs_init++;
		}

		// purple glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PURPLE);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PURPLE);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PURPLE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_PURPLE, 3);
			defs_init++;
		}

		// purple glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "purple_dye");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PANE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_PURPLE, 1);
			defs_init++;
		}

		// red glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_RED);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_RED);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_RED);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_RED, 3);
			defs_init++;
		}

		// red glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "red_dye");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PANE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_RED, 1);
			defs_init++;
		}

		// yellow glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_YELLOW);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_YELLOW);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_YELLOW);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_YELLOW, 3);
			defs_init++;
		}

		// yellow glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "yellow_dye");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PANE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_YELLOW, 1);
			defs_init++;
		}

		// black glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLACK);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLACK);
			defs[defs_init].specs[6] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_BLACK);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_BLACK, 3);
			defs_init++;
		}

		// black glass pane
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "black_dye");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_GLASS_PANE);
			defs[defs_init].item = new MaterialItem(CONTENT_GLASS_PANE_BLACK, 1);
			defs_init++;
		}

		// Snow block
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "snow_ball");
			defs[defs_init].specs[1] = ItemSpec(ITEM_CRAFT, "snow_ball");
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "snow_ball");
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "snow_ball");
			defs[defs_init].specs[4] = ItemSpec(ITEM_CRAFT, "snow_ball");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "snow_ball");
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "snow_ball");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "snow_ball");
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "snow_ball");
			defs[defs_init].item = new MaterialItem(CONTENT_SNOW_BLOCK, 1);
			defs_init++;
		}

		// finished wood
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_FINISHED, 4);
			defs_init++;
		}

		// finished wood
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_JUNGLEWOOD);
			defs[defs_init].item = new MaterialItem(CONTENT_WOOD_FINISHED, 4);
			defs_init++;
		}

		// chair
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_FINISHED);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_FINISHED);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_CHAIR, 2);
			defs_init++;
		}

		// table
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_FINISHED);
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_FINISHED);
			defs[defs_init].specs[2] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD_FINISHED);
			defs[defs_init].specs[3] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[5] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_TABLE, 1);
			defs_init++;
		}

		// bed
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_BED_HEAD, 1);
			defs_init++;
		}

		// blue bed
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLUE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLUE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_BED_BLUE_HEAD, 1);
			defs_init++;
		}

		// green bed
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_GREEN);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_GREEN);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_BED_GREEN_HEAD, 1);
			defs_init++;
		}

		// orange bed
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_ORANGE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_ORANGE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_BED_ORANGE_HEAD, 1);
			defs_init++;
		}

		// purple bed
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_PURPLE);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_PURPLE);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_BED_PURPLE_HEAD, 1);
			defs_init++;
		}

		// red bed
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_RED);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_RED);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_BED_RED_HEAD, 1);
			defs_init++;
		}

		// yellow bed
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_YELLOW);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_YELLOW);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_BED_YELLOW_HEAD, 1);
			defs_init++;
		}

		// black bed
		{
			defs[defs_init].specs[0] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[1] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON);
			defs[defs_init].specs[2] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[3] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[4] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLACK);
			defs[defs_init].specs[5] = ItemSpec(ITEM_MATERIAL, CONTENT_WOOD);
			defs[defs_init].specs[6] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].specs[7] = ItemSpec(ITEM_MATERIAL, CONTENT_COTTON_BLACK);
			defs[defs_init].specs[8] = ItemSpec(ITEM_CRAFT, "Stick");
			defs[defs_init].item = new MaterialItem(CONTENT_BED_BLACK_HEAD, 1);
			defs_init++;
		}
	}

	for (int i=0; i<defs_init; i++) {
		if (defs[i] == items)
			return defs[i].item->clone();
	}

	return NULL;
}

void craft_set_creative_inventory(Player *player)
{
	player->resetInventory();

	// Give some good tools
	{
		InventoryItem *item = new ToolItem("MesePick", 0);
		void* r = player->inventory.addItem("main", item);
		assert(r == NULL);
	}
	{
		InventoryItem *item = new ToolItem("SteelPick", 0);
		void* r = player->inventory.addItem("main", item);
		assert(r == NULL);
	}
	{
		InventoryItem *item = new ToolItem("SteelAxe", 0);
		void* r = player->inventory.addItem("main", item);
		assert(r == NULL);
	}
	{
		InventoryItem *item = new ToolItem("SteelShovel", 0);
		void* r = player->inventory.addItem("main", item);
		assert(r == NULL);
	}

	/*
		Give materials
	*/

	// CONTENT_IGNORE-terminated list
	content_t material_items[] = {
		CONTENT_TORCH,
		CONTENT_ROUGHSTONE,
		CONTENT_MUD,
		CONTENT_STONE,
		CONTENT_SAND,
		CONTENT_SANDSTONE,
		CONTENT_CLAY,
		CONTENT_BRICK,
		CONTENT_TREE,
		CONTENT_LEAVES,
		CONTENT_CACTUS,
		CONTENT_PAPYRUS,
		CONTENT_BOOKSHELF,
		CONTENT_GLASS,
		CONTENT_FENCE,
		CONTENT_RAIL,
		CONTENT_MESE,
		CONTENT_WATERSOURCE,
		CONTENT_COTTON,
		CONTENT_CHEST,
		CONTENT_FURNACE,
		CONTENT_SIGN,
		CONTENT_LAVASOURCE,
		CONTENT_WOOD,
		CONTENT_LADDER,
		CONTENT_IGNORE
	};

	content_t *mip = material_items;
	for(u16 i=0; i<PLAYER_INVENTORY_SIZE; i++)
	{
		if(*mip == CONTENT_IGNORE)
			break;

		InventoryItem *item = new MaterialItem(*mip, 1);
		player->inventory.addItem("main", item);

		mip++;
	}
}

void craft_give_initial_stuff(Player *player)
{
	{
		InventoryItem *item = new ToolItem("SteelPick", 0);
		void* r = player->inventory.addItem("main", item);
		assert(r == NULL);
	}
	{
		InventoryItem *item = new MaterialItem(CONTENT_TORCH, 99);
		void* r = player->inventory.addItem("main", item);
		assert(r == NULL);
	}
	{
		InventoryItem *item = new ToolItem("SteelAxe", 0);
		void* r = player->inventory.addItem("main", item);
		assert(r == NULL);
	}
	{
		InventoryItem *item = new ToolItem("SteelShovel", 0);
		void* r = player->inventory.addItem("main", item);
		assert(r == NULL);
	}
	{
		InventoryItem *item = new MaterialItem(CONTENT_ROUGHSTONE, 99);
		void* r = player->inventory.addItem("main", item);
		assert(r == NULL);
	}
}

