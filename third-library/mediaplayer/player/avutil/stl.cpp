/*
 * Copyright (c) 2016 Raymond Zheng <raymondzheng1412@gmail.com>
 *
 * This file is part of Player.
 *
 * Player is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * Player is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with Player; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA
 */
#include <map>

using namespace std;

typedef map<int64_t, void *> Map;

extern "C" void* map_create();
extern "C" void map_put(void *data, int64_t key, void *value);
extern "C" void* map_get(void *data, int64_t key);
extern "C" int map_remove(void *data, int64_t key);
extern "C" int map_size(void *data);
extern "C" int map_max_size(void *data);
extern "C" void* map_index_get(void *data, int index);
extern "C" int64_t map_get_min_key(void *data);
extern "C" void map_clear(void *data);
extern "C" void map_destroy(void *data);
extern "C" void map_traversal_handle(void *data, void *parm, int (*enu)(void *parm, int64_t key, void *elem));

void* map_create() {
    Map *data = new Map();
    return data;
}

void map_put(void *data, int64_t key, void *value) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data)
        return;
    (*map_data)[key] = value;
}

void* map_get(void *data, int64_t key) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data)
        return NULL;

    Map::iterator it = map_data->find(key);
    if (it != map_data->end()) {
        return it->second;
    }
    return NULL;
}

int map_remove(void *data, int64_t key) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data)
        return -1;
    map_data->erase(key);
    return 0;
}

int ijk_map_size(void *data) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data)
        return 0;

    return map_data->size();
}

int ijk_map_max_size(void *data) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data)
        return 0;

    return map_data->max_size();
}

void* map_index_get(void *data, int index) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data || map_data->empty())
        return NULL;

    Map::iterator it;
    it = map_data->begin();

    for (int i = 0; i < index; i++) {
        it = it++;
        if (it == map_data->end()) {
            return NULL;
        }
    }

    return it->second;
}

void map_traversal_handle(void *data, void *parm, int (*enu)(void *parm, int64_t key, void *elem)) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data || map_data->empty())
        return;

    Map::iterator it;

    for (it = map_data->begin(); it != map_data->end(); it++) {
        enu(parm, it->first, it->second);
    }
}

int64_t map_get_min_key(void *data) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data || map_data->empty())
        return -1;

    Map::iterator it = map_data->begin();

    int64_t min_key = it->first;

    for (; it != map_data->end(); it++) {
        min_key = min_key < it->first ? min_key : it->first;
    }

    return min_key;
}

void map_clear(void *data) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data)
        return;

    map_data->clear();
}

void map_destroy(void *data) {
    Map *map_data = reinterpret_cast<Map *>(data);
    if (!map_data)
        return;

    map_data->clear();
    delete(map_data);
}
