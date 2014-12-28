// Copyright (C) 2014 Eric Hebert (ayebear)
// This code is licensed under GPLv3, see LICENSE.txt for details.

#ifndef TILEMAPDATA_H
#define TILEMAPDATA_H

#include "matrix.h"
#include <map>
#include <set>

// Holds all of the information for a tile
struct Tile
{
    int logicalId;
    int visualId;
    bool collidable;
    bool state;
};

/*
A logical tile map in memory (not graphical)
Note that this is specific to the game.
*/
class TileMapData
{
    public:

        TileMapData();

        // Access tiles
        Tile& operator()(int layer, unsigned x, unsigned y);
        Tile& operator()(unsigned x, unsigned y);
        Tile& operator()(int id);

        // Important methods
        void resize(unsigned width, unsigned height, bool preserve = true);
        void useLayer(int layer);
        int getId(unsigned x, unsigned y) const;
        int getId(int layer, unsigned x, unsigned y) const;
        void deriveTiles();
        void updateVisualId(int id);
        void updateCollision(int id);

        // Tiles with objects on top
        void addTile(int id);
        bool findTile(int id) const;
        void clearTiles();
        void printTiles() const;

        // Tile IDs
        using TileList = std::vector<int>;
        TileList& operator[](int logicalId);
        void clearTileIds();

        // These are needed to extract information from the ID
        int getLayer(int id) const;
        unsigned getX(int id) const;
        unsigned getY(int id) const;

    private:

        // Populates the maps from a config file
        void loadTileInfo();


        // Level specific ----------------------------------------------------

        // Tile map
        Matrix<Tile> tiles[2];
        int currentLayer;

        // Map of logical IDs to lists of tile IDs
        std::map<int, TileList> tileIds;

        // All of the tile IDs with objects currently on top of them
        std::set<int> tilesWithObjectsOnTop;


        // Game specific -----------------------------------------------------

        // This is used to check if a tile type is collidable
        std::map<int, bool> logicalToCollision;

        // This is used for tiles with states to change visual IDs
        std::map<int, TileList> logicalToVisual;

        // This is used to check if collision should change when state changes
        std::set<int> stateChangesCollision;
};

#endif
