// Copyright (C) 2014-2015 Eric Hebert (ayebear)
// This code is licensed under GPLv3, see LICENSE.txt for details.

#include "leveleditor.h"
#include "events.h"
#include "gameevents.h"
#include "gameworld.h"
#include "configfile.h"
#include "vectors.h"
#include "stateevent.h"

#define BIND_ACTION(callback) actions[#callback].setCallback(std::bind(&LevelEditor::callback, this))

LevelEditor::LevelEditor(GameWorld& world, StateEvent& stateEvent):
    world(world),
    stateEvent(stateEvent)
{
    view = world.camera.getView("window");
    loadConfig("data/config/level_editor.cfg");
    updateBorder();
    tileSize = world.tileMap.getTileSize();

    // Using a TileMap to display the current tile
    // TODO: Make something simpler to use tiles from tile sets
    currentTile.loadFromConfig("data/config/tilemap.cfg");
    currentTile.resize(1, 1);
    currentTile.useLayer(0);
    currentTile.setColor(sf::Color(255, 255, 255, 192));
    currentVisualId = 1;
    updateCurrentTile();
}

void LevelEditor::handleEvent(const sf::Event& event)
{
    actions.handleEvent(event);

    // Handle switching tiles with the mouse wheel
    if (event.type == sf::Event::MouseWheelMoved)
    {
        currentVisualId += event.mouseWheel.delta;
        if (currentVisualId < 0)
            currentVisualId = 0;
        int visualTileCount = visualTiles.size();
        if (currentVisualId >= visualTileCount)
            currentVisualId = visualTileCount - 1;
        updateCurrentTile();
    }
}

void LevelEditor::update(float dt)
{
    updateMousePos();

    // Handle painting tiles
    if (ng::Action::windowHasFocus)
    {
        bool leftPressed = sf::Mouse::isButtonPressed(sf::Mouse::Left);
        bool rightPressed = sf::Mouse::isButtonPressed(sf::Mouse::Right);
        if (leftPressed)
            paintTile(currentVisualId);
        else if (rightPressed)
            paintTile(0);
    }

    // Handle panning
    sf::Vector2f panDelta;
    if (actions["panLeft"].isActive())
        panDelta.x -= 1;
    if (actions["panRight"].isActive())
        panDelta.x += 1;
    if (actions["panUp"].isActive())
        panDelta.y -= 1;
    if (actions["panDown"].isActive())
        panDelta.y += 1;
    view.move(panDelta.x * panSpeed * dt, panDelta.y * panSpeed * dt);

    // Handle zooming
    if (actions["zoomIn"].isActive())
        view.zoom(1.0f - dt);
    if (actions["zoomOut"].isActive())
        view.zoom(1.0f + dt);

    // Update the view for the input system
    es::Events::send(ViewEvent{view});
}

void LevelEditor::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    target.setView(view);
    world.tileMap.drawLayer(target, currentLayer);
    target.draw(border);
    target.draw(currentTile);
}

void LevelEditor::loadConfig(const std::string& filename)
{
    cfg::File config(filename);
    if (!config)
    {
        std::cerr << "Error loading " << filename << ".\n";
        return;
    }

    // Load general settings
    panSpeed = config("panSpeed").toFloat();
    defaultZoom = config("defaultZoom").toFloat();
    view.zoom(defaultZoom);

    // Load controls and setup actions
    actions.loadSection(config.getSection("Controls"), "");
    actions["toggleLayer"].setCallback([&]{ currentLayer = !currentLayer; });
    actions["popState"].setCallback([&]{ stateEvent.command = StateEvent::Pop; });
    BIND_ACTION(save);
    BIND_ACTION(load);
    BIND_ACTION(test);
    BIND_ACTION(undo);
    BIND_ACTION(redo);
    BIND_ACTION(clear);
    BIND_ACTION(nextLevel);
    BIND_ACTION(prevLevel);

    // Load visual tile reverse lookup table
    for (auto& option: config.getSection("VisualTiles"))
    {
        int visualId = strlib::fromString<int>(option.first);
        auto values = strlib::split<int>(option.second, " ");

        // Setup tile
        auto& tile = visualTiles[visualId];
        tile.logicalId = 0;
        tile.visualId = visualId;
        tile.collidable = false;
        tile.blocksLaser = false;
        tile.state = false;

        // Extract information
        if (!values.empty())
        {
            tile.logicalId = values.front();
            if (values.size() >= 2)
                tile.state = (values[1] != 0);
            world.tileMapData.updateCollision(tile);
        }
    }
}

void LevelEditor::save()
{
    world.level.saveToFile("data/levels/test.cfg");
}

void LevelEditor::load()
{
    world.level.loadFromFile(world.levelLoader.getLevelFilename(currentLevel));
}

void LevelEditor::test()
{
    // Push GameState and send a test event
    stateEvent.command = StateEvent::Push;
    stateEvent.name = "Game";
    TestModeEvent event;
    world.level.saveToString(event.level);
    es::Events::send(event);
}

void LevelEditor::undo()
{
    // Use an "UndoStack" class of some kind
    // Have "actions" that can be stored and applied/unapplied with before/after values
}

void LevelEditor::redo()
{

}

void LevelEditor::clear()
{
    // Reset all of the tiles
    // Remove all of the objects
}

void LevelEditor::nextLevel()
{
    ++currentLevel;
    load();
}

void LevelEditor::prevLevel()
{
    --currentLevel;
    load();
}

void LevelEditor::updateMousePos()
{
    for (auto& event: es::Events::get<MousePosEvent>())
    {
        mousePos = event.mousePos;
        currentTile.setPosition(sf::Vector2f(int(mousePos.x / tileSize.x) * tileSize.x,
                                             int(mousePos.y / tileSize.y) * tileSize.y));
    }
}

void LevelEditor::paintTile(int visualId)
{
    // Get location of tile to place, and make sure it is in bounds
    sf::Vector2i location(mousePos.x / tileSize.x, mousePos.y / tileSize.y);
    if (world.tileMap.inBounds(location.x, location.y))
    {
        int tileId = world.tileMapData.getId(currentLayer, location.x, location.y);
        //std::cout << "Location: " << tileId << ", Type: " << visualId << "\n";

        // Set the tile and update the graphical tile map
        world.tileMapData(tileId) = visualTiles[visualId];
        world.tileMapChanger.updateVisualTile(tileId);
    }
}

void LevelEditor::updateBorder()
{
    border.setSize(vectors::cast<float>(world.tileMap.getPixelSize()));
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::Green);
    border.setOutlineThickness(32);
}

void LevelEditor::updateCurrentTile()
{
    currentTile.set(0, 0, currentVisualId);
}
