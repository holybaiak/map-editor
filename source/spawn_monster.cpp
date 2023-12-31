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

#include "tile.h"
#include "spawn_monster.h"

SpawnMonster::SpawnMonster(int size) :
	size(0),
	selected(false) {
	setSize(size);
}

SpawnMonster* SpawnMonster::deepCopy() const {
	SpawnMonster* copy = new SpawnMonster(size);
	copy->selected = selected;
	return copy;
}

void SpawnsMonster::addSpawnMonster(Tile* tile) {
	ASSERT(tile->spawnMonster);

	auto it = spawnsMonster.insert(tile->getPosition());
	ASSERT(it.second);
}

void SpawnsMonster::removeSpawnMonster(Tile* tile) {
	ASSERT(tile->spawnMonster);
	spawnsMonster.erase(tile->getPosition());
#if 0
	SpawnMonsterPositionList::iterator iter = begin();
	while(iter != end()) {
		if(*iter == tile->getPosition()) {
			spawnsMonster.erase(iter);
			return;
		}
		++iter;
	}
	ASSERT(false);
#endif
}

std::ostream &operator<<(std::ostream &os, const SpawnMonster &spawnMonster) {
	os << &spawnMonster << ":: -> " << spawnMonster.getSize() << std::endl;
	return os;
}
