// Copyright (C) 2014-2015 Eric Hebert (ayebear)
// This code is licensed under GPLv3, see LICENSE.txt for details.

#ifndef LEVEL_H
#define LEVEL_H

#include <string>
#include <SFML/System/Vector2.hpp>
#include "configfile.h"
#include "OCS/Objects.hpp"

class TileMapData;
class TileMap;
class TileMapChanger;
class MagicWindow;

/*
This class loads data from level files:
    Tile map layers
    Game objects

Note that it does not store this data, only populates other objects with the data.

The current level file format looks like this:
    height = 12
    width = 32
    version = 1

    [Real]
    visual = {
    ...
    }
    logical = {
    ...
    }

    [Alternate]
    visual = {
    ...
    }
    logical = {
    ...
    }

    [SwitchObjects]
    123 = {
        "nameOfObjectToSwitch"
    }

    [Objects]
    name:Type = {
        "Component 321 100"
    }
*/
class Level
{
    public:
        Level(TileMapData& tileMapData, TileMap& tileMap, TileMapChanger& tileMapChanger, ocs::ObjectManager& entities, MagicWindow& magicWindow);

        // Loads a level file
        bool loadFromFile(const std::string& filename);
        void loadFromString(const std::string& data);

        // Saves a level file
        bool saveToFile(const std::string& filename) const;
        void saveToString(std::string& data) const;

        // Returns an object ID from an object's name
        ocs::ID getObjectIdFromName(const std::string& name) const;

    private:
        // Loading levels
        void load(cfg::File& config);
        void loadLogicalLayer(cfg::File& config, int layer);
        void loadVisualLayer(cfg::File& config, int layer);
        void loadTileMap(cfg::File& config);
        void loadObjects(cfg::File& config);

        // Saving levels
        void save(cfg::File& config) const;
        void saveTileMap(cfg::File& config) const;
        void saveObjects(cfg::File& config) const;

        static const cfg::File::ConfigMap defaultOptions;

        TileMapData& tileMapData; // Logical tile map
        TileMap& tileMap; // Visual tile map
        TileMapChanger& tileMapChanger;
        ocs::ObjectManager& entities;
        MagicWindow& magicWindow;

        std::map<std::string, ocs::ID> objectNamesToIds;
};

#endif
