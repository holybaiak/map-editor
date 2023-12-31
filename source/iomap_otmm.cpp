//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#include "main.h"

#include "iomap_otmm.h"
#include "item.h"
#include "complexitem.h"
#include "iomap_otmm.h"
#include "filehandle.h"
#include "map.h"
#include "gui.h"

// ============================================================================
// Item

Item* Item::Create_OTMM(const IOMap &maphandle, BinaryNode* stream) {
	uint16_t _id;
	if (!stream->getU16(_id)) {
		return nullptr;
	}

	return Item::Create(_id);
}

bool Item::readItemAttribute_OTMM(const IOMap &maphandle, OTMM_ItemAttribute attr, BinaryNode* stream) {
	switch (attr) {
		case OTMM_ATTR_SUBTYPE: {
			uint16_t subtype;
			if (!stream->getU16(subtype)) {
				return false;
			}

			setSubtype(subtype & 0xf);
		} break;
		case OTMM_ATTR_ACTION_ID: {
			uint16_t aid;
			if (!stream->getU16(aid)) {
				return false;
			}
			setActionID(aid);
		} break;
		case OTMM_ATTR_UNIQUE_ID: {
			uint16_t uid;
			if (!stream->getU16(uid)) {
				return false;
			}

			setUniqueID(uid);
		} break;
		case OTMM_ATTR_TEXT: {
			std::string text;
			if (!stream->getString(text)) {
				return false;
			}

			setText(text);
		} break;
		case OTMM_ATTR_DESC: {
			std::string text;
			if (!stream->getString(text)) {
				return false;
			}

			setDescription(text);
		} break;

		// If otb structure changes....
		case OTMM_ATTR_DEPOT_ID: {
			return stream->skip(2);
		} break;
		case OTMM_ATTR_DOOR_ID: {
			return stream->skip(1);
		} break;
		case OTMM_ATTR_TELE_DEST: {
			return stream->skip(5);
		} break;
		default: {
			return false;
		} break;
	}

	return true;
}

bool Item::unserializeAttributes_OTMM(const IOMap &maphandle, BinaryNode* stream) {
	uint8_t attribute;
	while (stream->getU8(attribute)) {
		if (!readItemAttribute_OTMM(maphandle, OTMM_ItemAttribute(attribute), stream)) {
			return false;
		}
	}
	return true;
}

bool Item::unserializeItemNode_OTMM(const IOMap &maphandle, BinaryNode* node) {
	return unserializeAttributes_OTMM(maphandle, node);
}

void Item::serializeItemAttributes_OTMM(const IOMap &maphandle, NodeFileWriteHandle &stream) const {
	if (getSubtype() > 0) {
		stream.addU8(OTMM_ATTR_SUBTYPE);
		stream.addU16(getSubtype());
	}

	if (getActionID()) {
		stream.addU8(OTMM_ATTR_ACTION_ID);
		stream.addU16(getActionID());
	}

	if (getUniqueID()) {
		stream.addU8(OTMM_ATTR_UNIQUE_ID);
		stream.addU16(getUniqueID());
	}

	if (getText().length() > 0) {
		stream.addU8(OTMM_ATTR_TEXT);
		stream.addString(getText());
	}
}

void Item::serializeItemCompact_OTMM(const IOMap &maphandle, NodeFileWriteHandle &stream) const {
	stream.addU16(id);
}

bool Item::serializeItemNode_OTMM(const IOMap &maphandle, NodeFileWriteHandle &f) const {
	f.addNode(OTMM_ITEM);
	f.addU16(id);
	serializeItemAttributes_OTMM(maphandle, f);
	f.endNode();

	return true;
}

// ============================================================================
// Teleport

bool Teleport::readItemAttribute_OTMM(const IOMap &maphandle, OTMM_ItemAttribute attribute, BinaryNode* stream) {
	if (attribute == OTMM_ATTR_TELE_DEST) {
		uint16_t x = 0;
		uint16_t y = 0;
		uint8_t z = 0;
		if (!stream->getU16(x) || !stream->getU16(y) || !stream->getU8(z)) {
			return false;
		}

		destination = Position(x, y, z);
		return true;
	} else {
		return Item::readItemAttribute_OTMM(maphandle, attribute, stream);
	}
}

void Teleport::serializeItemAttributes_OTMM(const IOMap &maphandle, NodeFileWriteHandle &stream) const {
	Item::serializeItemAttributes_OTMM(maphandle, stream);

	stream.addByte(OTMM_ATTR_TELE_DEST);
	stream.addU16(destination.x);
	stream.addU16(destination.y);
	stream.addU8(destination.z);
}

// ============================================================================
// Door

bool Door::readItemAttribute_OTMM(const IOMap &maphandle, OTMM_ItemAttribute attribute, BinaryNode* stream) {
	if (attribute == OTMM_ATTR_DOOR_ID) {
		uint8_t id = 0;
		if (!stream->getU8(id)) {
			return false;
		}
		doorid = id;
		return true;
	} else {
		return Item::readItemAttribute_OTMM(maphandle, attribute, stream);
	}
}

void Door::serializeItemAttributes_OTMM(const IOMap &maphandle, NodeFileWriteHandle &stream) const {
	Item::serializeItemAttributes_OTMM(maphandle, stream);
	if (doorid) {
		stream.addByte(OTMM_ATTR_DOOR_ID);
		stream.addU8(doorid);
	}
}

// ============================================================================
// Depots

bool Depot::readItemAttribute_OTMM(const IOMap &maphandle, OTMM_ItemAttribute attribute, BinaryNode* stream) {
	if (attribute == OTMM_ATTR_DEPOT_ID) {
		uint16_t id = 0;
		if (!stream->getU16(id)) {
			return false;
		}
		depotid = id;
		return true;
	} else {
		return Item::readItemAttribute_OTMM(maphandle, attribute, stream);
	}
}

void Depot::serializeItemAttributes_OTMM(const IOMap &maphandle, NodeFileWriteHandle &stream) const {
	Item::serializeItemAttributes_OTMM(maphandle, stream);
	if (depotid) {
		stream.addByte(OTMM_ATTR_DEPOT_ID);
		stream.addU16(depotid);
	}
}

// ============================================================================
// Container

bool Container::unserializeItemNode_OTMM(const IOMap &maphandle, BinaryNode* node) {
	bool ret = Item::unserializeAttributes_OTMM(maphandle, node);

	if (ret) {
		BinaryNode* child = node->getChild();
		if (child) {
			do {
				uint8_t type;
				if (!child->getByte(type)) {
					return false;
				}
				// load container items
				if (type == OTMM_ITEM) {
					Item* item = Item::Create_OTMM(maphandle, child);
					if (!item) {
						return false;
					}
					if (!item->unserializeItemNode_OTMM(maphandle, child)) {
						delete item;
						return false;
					}
					contents.push_back(item);
				} else {
					// corrupted file data!
					return false;
				}
			} while (child->advance());
		}
		return true;
	}
	return false;
}

bool Container::serializeItemNode_OTMM(const IOMap &maphandle, NodeFileWriteHandle &f) const {
	f.addNode(OTMM_ITEM);

	f.addU16(id);
	serializeItemAttributes_OTMM(maphandle, f);

	for (ItemVector::const_iterator it = contents.begin(); it != contents.end(); ++it) {
		(*it)->serializeItemNode_OTMM(maphandle, f);
	}
	f.endNode();
	return true;
}

IOMapOTMM::IOMapOTMM() {
}

IOMapOTMM::~IOMapOTMM() {
}

ClientVersionID IOMapOTMM::getVersionInfo(const FileName &filename) {
	wxString wpath = filename.GetFullPath();
	DiskNodeFileReadHandle f((const char*)wpath.mb_str(wxConvUTF8));
	if (f.isOk() == false) {
		return CLIENT_VERSION_NONE;
	}

	BinaryNode* root = f.getRootNode();
	if (!root) {
		return CLIENT_VERSION_NONE;
	}
	root->skip(1); // Skip the type byte

	uint16_t u16;
	uint32_t u32;

	if (!root->getU32(u32) || u32 != 1) { // Version
		return CLIENT_VERSION_NONE;
	}

	root->getU16(u16);
	root->getU16(u16);
	root->getU32(u32);

	if (root->getU32(u32)) { // OTB minor version
		return ClientVersionID(u32);
	}
	return CLIENT_VERSION_NONE;
}

bool IOMapOTMM::loadMap(Map &map, const FileName &identifier, bool showdialog) {
	if (showdialog) {
		g_gui.CreateLoadBar("Loading OTMM map...");
	}
	DiskNodeFileReadHandle f(nstr(identifier.GetFullPath()));
	if (f.isOk() == false) {
		error("Couldn't open file for reading\nThe error reported was: " + wxstr(f.getErrorMessage()));
		return false;
	}

	bool ret = loadMap(map, f, identifier, showdialog);

	if (showdialog) {
		g_gui.DestroyLoadBar();
	}

	return ret;
}

bool IOMapOTMM::loadMap(Map &map, NodeFileReadHandle &f, const FileName &identifier, bool showdialog) {
	BinaryNode* root = f.getRootNode();
	if (!root) {
		error("Could not read root node.");
		return false;
	}
	root->skip(1); // Skip the type byte

	uint8_t u8;
	uint16_t u16;
	uint32_t u32;

	if (!root->getU32(u32) || u32 != 1) { // Version
		error("Unsupported or damaged map version.");
		return false;
	}

	if (!root->getU16(u16)) {
		error("Could not read root header.");
		return false;
	}
	map.width = u16;
	if (!root->getU16(u16)) {
		error("Could not read root header.");
		return false;
	}
	map.height = u16;

	if (!root->getU32(u32) || u32 > (unsigned long)g_items.MajorVersion) { // OTB major version
		if (queryUser("Map error", "The loaded map appears to be a items.otb format that deviates from the items.otb loaded by the editor. Do you still want to attempt to load the map?")) {
			warning("Unsupported or damaged map version");
		} else {
			error("Outdated items.otb, could not load map.");
			return false;
		}
	}

	if (!root->getU32(u32) || u32 > (unsigned long)g_items.MinorVersion) { // OTB minor version
		warning("The editor needs an updated items.otb version.");
	}

	BinaryNode* mapHeaderNode = root->getChild();
	if (mapHeaderNode == nullptr || !mapHeaderNode->getByte(u8) || u8 != OTMM_MAP_DATA) {
		error("Could not get root child node. Cannot recover from fatal error!");
		return false;
	}

	int nodes_loaded = 0;

	BinaryNode* mapNode = mapHeaderNode->getChild();
	if (mapNode) {
		do {
			++nodes_loaded;
			if (showdialog && nodes_loaded % 15 == 0) {
				g_gui.SetLoadDone(int(100.0 * f.tell() / f.size()));
			}
			uint8_t node_type;
			if (!mapNode->getByte(node_type)) {
				warning("Invalid map node");
				continue;
			}
			switch (node_type) {
				case OTMM_EDITOR: {
				} break;
				case OTMM_DESCRIPTION: {
					std::string desc;
					mapNode->getString(desc);
					map.setMapDescription(desc);
				} break;
				case OTMM_TILE_DATA: {
					BinaryNode* tileNode = mapNode->getChild();
					if (tileNode) {
						do {
							Tile* tile = nullptr;
							uint8_t tile_type;
							if (!tileNode->getByte(tile_type)) {
								warning("Invalid tile type");
								continue;
							}
							if (tile_type != OTMM_TILE && tile_type != OTMM_HOUSETILE) {
								warning("Unknown type of tile node");
								continue;
							}

							uint16_t x_offset, y_offset;
							uint8_t z_offset;
							if (!tileNode->getU16(x_offset) || !tileNode->getU16(y_offset) || !tileNode->getU8(z_offset)) {
								warning("Could not read position of tile");
								continue;
							}
							const Position pos(x_offset, y_offset, z_offset);

							if (map.getTile(pos)) {
								warning("Duplicate tile at %d:%d:%d, discarding duplicate", pos.x, pos.y, pos.z);
								continue;
							}

							tile = map.allocator(pos);
							House* house = nullptr;
							if (tile_type == OTMM_HOUSETILE) {
								uint32_t house_id;
								if (!tileNode->getU32(house_id)) {
									warning("House tile without house data, discarding tile");
									continue;
								}
								if (house_id) {
									house = map.houses.getHouse(house_id);
									if (!house) {
										house = newd House(map);
										house->id = house_id;
										map.houses.addHouse(house);
									}
								} else {
									warning("Invalid house id from tile %d:%d:%d", pos.x, pos.y, pos.z);
								}
							}

							uint16_t ground_id;
							tileNode->getU16(ground_id);
							if (ground_id != 0) {
								tile->addItem(Item::Create(ground_id));
							}

							uint8_t attribute;
							while (tileNode->getU8(attribute)) {
								switch (attribute) {
									case OTMM_ATTR_TILE_FLAGS: {
										uint32_t flags = 0;
										if (!tileNode->getU32(flags)) {
											warning("Invalid tile flags of tile on %d:%d:%d", pos.x, pos.y, pos.z);
										}
										tile->setMapFlags(flags);
									} break;
									default: {
										warning("Unknown tile attribute at %d:%d:%d", pos.x, pos.y, pos.z);
									} break;
								}
							}

							BinaryNode* itemNode = tileNode->getChild();
							if (itemNode) {
								do {
									Item* item = nullptr;
									uint8_t item_type;
									if (!itemNode->getByte(item_type)) {
										warning("Unknown item type %d:%d:%d", pos.x, pos.y, pos.z);
										continue;
									}
									if (item_type == OTMM_ITEM) {
										item = Item::Create_OTMM(*this, itemNode);
										if (item) {
											if (item->unserializeItemNode_OTMM(*this, itemNode) == false) {
												warning("Couldn't unserialize item attributes at %d:%d:%d", pos.x, pos.y, pos.z);
											}
											tile->addItem(item);
										}
									} else {
										warning("Unknown type of tile child node");
									}
								} while (itemNode->advance());
							}

							tile->update();
							if (house) {
								house->addTile(tile);
							}
							map.setTile(pos, tile);
						} while (tileNode->advance());
					}
				} break;
				case OTMM_SPAWN_MONSTER_DATA: {
					BinaryNode* spawnMonsterNode = mapNode->getChild();
					if (spawnMonsterNode) {
						do {
							uint8_t spawn_monster_type;
							if (!spawnMonsterNode->getByte(spawn_monster_type)) {
								warning("Could not read monster spawn type.");
								continue;
							}
							if (spawn_monster_type != OTMM_SPAWN_MONSTER_AREA) {
								warning("Invalid monster spawn type.");
								continue;
							}

							// Read position
							uint16_t spawnMonster_x, spawnMonster_y;
							uint8_t spawnMonster_z;
							uint32_t radius;
							if (!spawnMonsterNode->getU16(spawnMonster_x) || !spawnMonsterNode->getU16(spawnMonster_y) || !spawnMonsterNode->getU8(spawnMonster_z)) {
								warning("Could not read monster spawn position.");
								continue;
							}
							const Position spawnPos(spawnMonster_x, spawnMonster_y, spawnMonster_z);

							// Read radius
							if (!spawnMonsterNode->getU32(radius)) {
								warning("Could not read monster spawn radius.");
								continue;
							}
							// Adjust radius
							radius = min(radius, uint32_t(g_settings.getInteger(Config::MAX_SPAWN_MONSTER_RADIUS)));

							// Create and assign monster spawn
							Tile* spawnMonsterTile = map.getTile(spawnPos);
							if (spawnMonsterTile && spawnMonsterTile->spawnMonster) {
								warning("Duplicate monster spawn on position %d:%d:%d\n", spawnMonsterTile->getX(), spawnMonsterTile->getY(), spawnMonsterTile->getZ());
								continue;
							}

							SpawnMonster* spawnMonster = newd SpawnMonster(radius);
							if (!spawnMonsterTile) {
								spawnMonsterTile = map.allocator(spawnPos);
								map.setTile(spawnPos, spawnMonsterTile);
							}
							spawnMonsterTile->spawnMonster = spawnMonster;
							map.addSpawnMonster(spawnMonsterTile);

							// Read any monsters associated with the spawnMonster
							BinaryNode* monsterNode = spawnMonsterNode->getChild();
							if (monsterNode) {
								do {
									uint8_t monster_type;
									if (!monsterNode->getByte(monster_type)) {
										warning("Could not read type of monster node.");
										continue;
									}
									std::string name;
									uint32_t spawntime = 0; // Only applicable for monsters

									if (monster_type == OTMM_MONSTER) {
										if (!monsterNode->getString(name)) {
											warning("Could not read name of monster.");
											return false;
										}
										if (!monsterNode->getU32(spawntime)) {
											warning("Could not read spawntime of monster.");
											return false;
										}
									} else {
										warning("Unknown monster node type (0x%.2x).", monster_type);
										return false;
									}

									// Read monster position
									uint16_t monster_x, monster_y;
									uint8_t monster_z;
									if (!monsterNode->getU16(monster_x) || !monsterNode->getU16(monster_y) || !monsterNode->getU8(monster_z)) {
										warning("Could not read monster position.");
										continue;
									}
									const Position monsterPos(monster_x, monster_y, monster_z);

									// Check radius
									if (uint32_t(abs(monsterPos.x - spawnPos.x)) > radius || uint32_t(abs(monsterPos.y - spawnPos.y)) > radius) {
										// Outside of the monster spawn...
									}

									// Create monster and put on map
									Tile* monster_tile;
									if (monsterPos == spawnPos) {
										monster_tile = spawnMonsterTile;
									} else {
										monster_tile = map.getTile(monsterPos);
									}
									if (!monster_tile) {
										warning("Discarding monster \"%s\" at %d:%d:%d due to invalid position", name.c_str(), monsterPos.x, monsterPos.y, monsterPos.z);
										break;
									}
									if (monster_tile->monster) {
										warning("Duplicate monster \"%s\" at %d:%d:%d, discarding", name.c_str(), monsterPos.x, monsterPos.y, monsterPos.z);
										break;
									}
									MonsterType* type = g_monsters[name];
									if (!type) {
										type = g_monsters.addMissingMonsterType(name);
									}
									Monster* monster = newd Monster(type);
									monster->setSpawnMonsterTime(spawntime);
									monster_tile->monster = monster;
									if (monster_tile->spawn_monster_count == 0) {
										// No monster spawn, create a newd one (this happends if the radius of the monster spawn has been decreased due to g_settings)
										ASSERT(monster_tile->spawnMonster == nullptr);
										SpawnMonster* spawnMonster = newd SpawnMonster(5);
										monster_tile->spawnMonster = spawnMonster;
										map.addSpawnMonster(monster_tile);
									}
								} while (monsterNode->advance());
							}
						} while (spawnMonsterNode->advance());
					}
				} break;
				case OTMM_SPAWN_NPC_DATA: {
					BinaryNode* spawnNpcNode = mapNode->getChild();
					if (spawnNpcNode) {
						do {
							uint8_t spawnNpcType;
							if (!spawnNpcNode->getByte(spawnNpcType)) {
								warning("Could not read spawnNpc type.");
								continue;
							}
							if (spawnNpcType != OTMM_SPAWN_NPC_AREA) {
								warning("Invalid spawnNpc type.");
								continue;
							}

							// Read position
							uint16_t spawnNpc_x, spawnNpc_y;
							uint8_t spawnNpc_z;
							uint32_t radius;
							if (!spawnNpcNode->getU16(spawnNpc_x) || !spawnNpcNode->getU16(spawnNpc_y) || !spawnNpcNode->getU8(spawnNpc_z)) {
								warning("Could not read spawnNpc position.");
								continue;
							}
							const Position spawnNpcPos(spawnNpc_x, spawnNpc_y, spawnNpc_z);

							// Read radius
							if (!spawnNpcNode->getU32(radius)) {
								warning("Could not read spawnNpc radius.");
								continue;
							}
							// Adjust radius
							radius = min(radius, uint32_t(g_settings.getInteger(Config::MAX_SPAWN_NPC_RADIUS)));

							// Create and assign spawnNpc
							Tile* spawnNpcTile = map.getTile(spawnNpcPos);
							if (spawnNpcTile && spawnNpcTile->spawnNpc) {
								warning("Duplicate spawnNpc on position %d:%d:%d\n", spawnNpcTile->getX(), spawnNpcTile->getY(), spawnNpcTile->getZ());
								continue;
							}

							SpawnNpc* spawnNpc = newd SpawnNpc(radius);
							if (!spawnNpcTile) {
								spawnNpcTile = map.allocator(spawnNpcPos);
								map.setTile(spawnNpcPos, spawnNpcTile);
							}
							spawnNpcTile->spawnNpc = spawnNpc;
							map.addSpawnNpc(spawnNpcTile);

							// Read any npc associated with the npc spawn
							BinaryNode* npcNode = spawnNpcNode->getChild();
							if (npcNode) {
								do {
									uint8_t npcType;
									if (!npcNode->getByte(npcType)) {
										warning("Could not read type of npc node.");
										continue;
									}
									std::string name;
									uint32_t spawntime = 0;

									if (npcType == OTMM_NPC) {
										if (!npcNode->getString(name)) {
											warning("Could not read name of NPC.");
											return false;
										}
									} else {
										warning("Unknown npc node type (0x%.2x).", npcType);
										return false;
									}

									// Read npc position
									uint16_t npc_x, npc_y;
									uint8_t npc_z;
									if (!npcNode->getU16(npc_x) || !npcNode->getU16(npc_y) || !npcNode->getU8(npc_z)) {
										warning("Could not read npc position.");
										continue;
									}
									const Position npcPos(npc_x, npc_y, npc_z);

									// Check radius
									if (uint32_t(abs(npcPos.x - spawnNpcPos.x)) > radius || uint32_t(abs(npcPos.y - spawnNpcPos.y)) > radius) {
										// Outside of the spawnNpc...
									}

									// Create npc and put on map
									Tile* npcTile;
									if (npcPos == spawnNpcPos) {
										npcTile = spawnNpcTile;
									} else {
										npcTile = map.getTile(npcPos);
									}
									if (!npcTile) {
										warning("Discarding npc \"%s\" at %d:%d:%d due to invalid position", name.c_str(), npcPos.x, npcPos.y, npcPos.z);
										break;
									}
									if (npcTile->npc) {
										warning("Duplicate npc \"%s\" at %d:%d:%d, discarding", name.c_str(), npcPos.x, npcPos.y, npcPos.z);
										break;
									}
									NpcType* type = g_npcs[name];
									if (!type) {
										type = g_npcs.addMissingNpcType(name);
									}
									Npc* npc = newd Npc(type);
									npc->setSpawnNpcTime(spawntime);
									npcTile->npc = npc;
									if (npcTile->spawn_npc_count == 0) {
										// No npc spawn, create a newd one (this happends if the radius of the npc spawn has been decreased due to g_settings)
										ASSERT(npcTile->spawnNpc == nullptr);
										SpawnNpc* spawnNpc = newd SpawnNpc(1);
										npcTile->spawnNpc = spawnNpc;
										map.addSpawnNpc(npcTile);
									}
								} while (npcNode->advance());
							}
						} while (spawnNpcNode->advance());
					}
				} break;
				case OTMM_TOWN_DATA: {
					BinaryNode* townNode = mapNode->getChild();
					if (townNode) {
						do {
							uint8_t town_type;
							if (!townNode->getByte(town_type)) {
								warning("Could not read town type");
								continue;
							}
							if (town_type != OTMM_TOWN) {
								warning("Unknown town type");
								continue;
							}
							uint32_t town_id;
							if (!townNode->getU32(town_id)) {
								warning("Invalid town id");
								continue;
							}

							Town* town = map.towns.getTown(town_id);
							if (town) {
								warning("Duplicate town id %d, discarding duplicate", town_id);
								continue;
							} else {
								town = newd Town(town_id);
								if (!map.towns.addTown(town)) {
									delete town;
									continue;
								}
							}
							std::string town_name;
							if (!townNode->getString(town_name)) {
								warning("Invalid town name");
								continue;
							}
							town->setName(town_name);
							Position pos;
							uint16_t x;
							uint16_t y;
							uint8_t z;
							if (!townNode->getU16(x) || !townNode->getU16(y) || !townNode->getU8(z)) {
								warning("Invalid town temple position");
								continue;
							}
							pos.x = x;
							pos.y = y;
							pos.z = z;
							town->setTemplePosition(pos);
						} while (townNode->advance());
					}
				} break;
				case OTMM_HOUSE_DATA: {
					BinaryNode* houseNode = mapNode->getChild();
					if (houseNode) {
						do {
							uint8_t house_type;
							if (!houseNode->getByte(house_type)) {
								warning("Could not read house type");
								continue;
							}
							if (house_type != OTMM_HOUSE) {
								warning("Unknown house type.");
								continue;
							}
							uint32_t house_id;
							if (!houseNode->getU32(house_id)) {
								warning("Could not read house id.");
								continue;
							}

							House* house = map.houses.getHouse(house_id);
							if (!house) {
								continue;
							}

							std::string house_name;
							if (!houseNode->getString(house_name)) {
								warning("Could not read house name.");
								continue;
							}

							uint32_t town_id;
							if (!houseNode->getU32(town_id)) {
								warning("Could not read house town id.");
								continue;
							}

							uint32_t rent;
							if (!houseNode->getU32(rent)) {
								warning("Could not read house rent.");
								continue;
							}

							uint32_t beds;
							if (!houseNode->getU32(beds)) {
								warning("Could not read house max beds.");
								continue;
							}

							house->name = house_name;
							house->townid = town_id;
							house->rent = rent;
							house->beds = beds;

							uint16_t x;
							uint16_t y;
							uint8_t z;
							if (!houseNode->getU16(x) || !houseNode->getU16(y) || !houseNode->getU8(z)) {
								warning("Invalid town temple position");
								continue;
							}
							house->setExit(Position(x, y, z));
						} while (houseNode->advance());
					}
				} break;
			}
		} while (mapNode->advance());
	}
	return true;
}

bool IOMapOTMM::saveMap(Map &map, const FileName &identifier, bool showdialog) {
	DiskNodeFileWriteHandle f(std::string(identifier.GetFullPath().mb_str(wxConvUTF8)));

	if (f.isOk() == false) {
		error("Can not open file %s for writing", (const char*)identifier.GetFullPath().mb_str(wxConvUTF8));
		return false;
	}

	if (showdialog) {
		g_gui.CreateLoadBar("Saving OTMM map...");
	}
	bool ret = saveMap(map, f, identifier, showdialog);
	if (showdialog) {
		g_gui.DestroyLoadBar();
	}

	return ret;
}

bool IOMapOTMM::saveMap(Map &map, NodeFileWriteHandle &f, const FileName &identifier, bool showdialog) {
	/* STOP!
	 * Before you even think about modifying this, please reconsider.
	 * while adding stuff to the binary format may be "cool", you'll
	 * inevitably make it incompatible with any future releases of
	 * the map editor, meaning you cannot reuse your map. Before you
	 * try to modify this, PLEASE consider using an external file
	 * like monster.xml, npc.xml or house.xml, as that will be MUCH easier
	 * to port to newer versions of the editor than a custom binary
	 * format.
	 */

	f.addNode(0);
	{
		f.addU32(1); // Version
		f.addU16(map.getWidth());
		f.addU16(map.getHeight());
		f.addU32(g_items.MajorVersion);
		f.addU32(g_items.MinorVersion);

		f.addNode(OTMM_MAP_DATA);
		{
			f.addNode(OTMM_EDITOR);
			{
				f.addString("Saved with Remere's Map Editor " + __RME_VERSION__);
			}
			f.endNode(); // OTMM_DESCRIPTION
			f.addNode(OTMM_DESCRIPTION);
			{
				f.addString(map.getMapDescription());
			}
			f.endNode(); // OTMM_DESCRIPTION

			// Start writing tiles
			uint tiles_saved = 0;

			MapIterator map_iterator = map.begin();

			f.addNode(OTMM_TILE_DATA);
			{
				while (map_iterator != map.end()) {
					// Update progressbar
					++tiles_saved;
					if (showdialog && tiles_saved % 8192 == 0) {
						g_gui.SetLoadDone(int(tiles_saved / double(map.getTileCount()) * 100.0));
					}

					// Get tile
					const Tile* save_tile = *map_iterator;
					const Position &pos = save_tile->getPosition();

					// Is it an empty tile that we can skip? (Leftovers...)
					if (save_tile->size() == 0) {
						++map_iterator;
						continue;
					}

					f.addNode(save_tile->isHouseTile() ? OTMM_HOUSETILE : OTMM_TILE);
					{
						f.addU16(save_tile->getX());
						f.addU16(save_tile->getY());
						f.addU8(save_tile->getZ() & 0xf);

						if (save_tile->isHouseTile()) {
							f.addU32(save_tile->getHouseID());
						}
						if (save_tile->getMapFlags()) {
							f.addByte(OTMM_ATTR_TILE_FLAGS);
							f.addU32(save_tile->getMapFlags());
						}

						if (save_tile->ground) {
							const Item* ground = save_tile->ground;
							if (ground->isMetaItem()) {
								// Do nothing, we don't save metaitems...
								f.addU16(0);
							} else if (ground->hasBorderEquivalent()) {
								bool found = false;
								for (ItemVector::const_iterator it = save_tile->items.begin(); it != save_tile->items.end(); ++it) {
									if ((*it)->getGroundEquivalent() == ground->getID()) {
										// Do nothing
										// Found equivalent
										found = true;
										break;
									}
								}
								if (found == false) {
									f.addU16(ground->getID());
								} else {
									f.addU16(0);
								}
							} else if (ground->isComplex() || ground->hasSubtype()) {
								f.addU16(0);
								ground->serializeItemNode_OTMM(*this, f);
							} else {
								f.addU16(ground->getID());
							}
						} else {
							f.addU16(0);
						}

						for (ItemVector::const_iterator it = save_tile->items.begin(); it != save_tile->items.end(); ++it) {
							if (!(*it)->isMetaItem()) {
								(*it)->serializeItemNode_OTMM(*this, f);
							}
						}
					}
					f.endNode();

					++map_iterator;
				}
			}
			f.endNode(); // OTMM_TILE_DATA

			f.addNode(OTMM_SPAWN_MONSTER_DATA);
			{
				SpawnsNpc &spawnsMonster = map.spawnsMonster;
				MonsterList monster_list;
				for (SpawnNpcPositionList::const_iterator piter = spawnsMonster.begin(); piter != spawnsMonster.end(); ++piter) {
					const Tile* tile = map.getTile(*piter);
					ASSERT(tile);
					const SpawnMonster* spawnMonster = tile->spawnMonster;
					ASSERT(spawnMonster);

					f.addNode(OTMM_SPAWN_MONSTER_AREA);
					{
						f.addU16(tile->getX());
						f.addU16(tile->getY());
						f.addU8(tile->getZ() & 0xf);
						f.addU32(spawnMonster->getSize());
						for (int y = -tile->spawnMonster->getSize(); y <= tile->spawnMonster->getSize(); ++y) {
							for (int x = -tile->spawnMonster->getSize(); x <= tile->spawnMonster->getSize(); ++x) {
								Tile* monster_tile = map.getTile(*piter + Position(x, y, 0));
								if (monster_tile) {
									Monster* c = monster_tile->monster;
									if (c && c->isSaved() == false) {
										f.addNode(OTMM_MONSTER);
										{
											f.addString(c->getName());
											f.addU32(c->getSpawnMonsterTime());
											f.addU16(monster_tile->getX());
											f.addU16(monster_tile->getY());
											f.addU8(monster_tile->getZ() & 0xf);
										}
										f.endNode(); // OTMM_MONSTER

										// Flag as saved
										c->save();
										monster_list.push_back(c);
									}
								}
							}
						}
					}
					f.endNode(); // OTMM_SPAWN_MONSTER_AREA
				}
				for (MonsterList::iterator iter = monster_list.begin(); iter != monster_list.end(); ++iter) {
					(*iter)->reset();
				}
			}
			f.endNode(); // OTMM_SPAWN_MONSTER_DATA

			f.addNode(OTMM_SPAWN_NPC_DATA);
			{
				SpawnsNpc &spawnNpc = map.spawnsNpc;
				NpcList npcList;
				for (SpawnNpcPositionList::const_iterator piter = spawnNpc.begin(); piter != spawnNpc.end(); ++piter) {
					const Tile* tile = map.getTile(*piter);
					ASSERT(tile);
					const SpawnNpc* spawnNpc = tile->spawnNpc;
					ASSERT(spawnNpc);

					f.addNode(OTMM_SPAWN_NPC_AREA);
					{
						f.addU16(tile->getX());
						f.addU16(tile->getY());
						f.addU8(tile->getZ() & 0xf);
						f.addU32(spawnNpc->getSize());
						for (int y = -tile->spawnNpc->getSize(); y <= tile->spawnNpc->getSize(); ++y) {
							for (int x = -tile->spawnNpc->getSize(); x <= tile->spawnNpc->getSize(); ++x) {
								Tile* npcTile = map.getTile(*piter + Position(x, y, 0));
								if (npcTile) {
									Npc* npc = npcTile->npc;
									if (npc && npc->isSaved() == false) {
										f.addNode(OTMM_NPC);
										{
											f.addString(npc->getName());
											f.addU16(npcTile->getX());
											f.addU16(npcTile->getY());
											f.addU8(npcTile->getZ() & 0xf);
										}
										f.endNode(); // OTMM_NPC

										// Flag as saved
										npc->save();
										npcList.push_back(npc);
									}
								}
							}
						}
					}
					f.endNode(); // OTMM_SPAWN_NPC_AREA
				}
				for (NpcList::iterator iter = npc_list.begin(); iter != npc_list.end(); ++iter) {
					(*iter)->reset();
				}
			}
			f.endNode(); // OTMM_SPAWN_NPC_DATA

			f.addNode(OTMM_TOWN_DATA);
			{
				for (TownMap::const_iterator it = map.towns.begin(); it != map.towns.end(); ++it) {
					const Town* town = it->second;
					f.addNode(OTMM_TOWN);
					{
						f.addU32(town->getID());
						f.addString(town->getName());
						f.addU16(town->getTemplePosition().x);
						f.addU16(town->getTemplePosition().y);
						f.addU8(town->getTemplePosition().z & 0xf);
					}
					f.endNode(); // OTMM_TOWN
				}
			}
			f.endNode(); // OTMM_TOWN_DATA

			f.addNode(OTMM_HOUSE_DATA);
			{
				for (HouseMap::const_iterator it = map.houses.begin(); it != map.houses.end(); ++it) {
					const House* house = it->second;
					f.addNode(OTMM_HOUSE);
					{
						f.addU32(house->id);
						f.addString(house->name);
						f.addU16(house->townid);
						f.addU16(house->rent);
						f.addU16(house->beds);
						f.addU16(house->getExit().x);
						f.addU16(house->getExit().y);
						f.addU8(house->getExit().z & 0xf);
					}
					f.endNode(); // OTMM_HOUSE
				}
			}
			f.endNode(); // OTMM_HOUSE_DATA
		}
		f.endNode(); // OTMM_MAP_DATA
	}
	f.endNode(); // root node
	return true;
}
