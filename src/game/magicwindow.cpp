// Copyright (C) 2014-2015 Eric Hebert (ayebear)
// This code is licensed under GPLv3, see LICENSE.txt for details.

#include "magicwindow.h"
#include <iostream>
#include "views.h"
#include "events.h"
#include "gameevents.h"

MagicWindow::MagicWindow():
    blockSize(DEFAULT_BLOCK_SIZE)
{
    changed = false;
    visible = false;
    border.setFillColor(sf::Color::Transparent);
    border.setOutlineColor(sf::Color::Blue);
    border.setOutlineThickness(4);
    preview.setFillColor(sf::Color::Transparent);
    preview.setOutlineColor(sf::Color(128, 128, 128, 128));
    preview.setOutlineThickness(4);
}

void MagicWindow::update()
{
    // Check SFML events
    for (auto& event: es::Events::get<sf::Event>())
    {
        if (event.type == sf::Event::MouseButtonReleased && event.mouseButton.button == sf::Mouse::Left)
            active = false;
        else if (event.type == sf::Event::MouseWheelMoved)
            handleResize(event.mouseWheel.delta);
        else if (event.type == sf::Event::KeyPressed)
            handleKeyPressed(event.key);
    }
    // Check real-time mouse input
    for (auto& event: es::Events::get<MousePosEvent>())
        setCenter(event.mousePos);
    // Check mouse-clicked events
    for (auto& event: es::Events::get<MouseClickedEvent>())
    {
        if (event.button == sf::Mouse::Left)
        {
            active = true;
            visible = true;
            setCenter(event.mousePos);
        }
        else if (event.button == sf::Mouse::Right)
        {
            active = false;
            visible = false;
        }
    }
}

void MagicWindow::setTileSize(const sf::Vector2u& newTileSize)
{
    tileSize = newTileSize;
    // Note: Other things may need to update
}

void MagicWindow::setCenter(const sf::Vector2f& center, bool force, bool updatePreview)
{
    // Only set the position if the player is holding the mouse button down
    // Or, if the code calling it forces it to set the position
    if (force || active)
    {
        sf::Vector2f newPosition;
        newPosition.x = center.x - (size.x / 2);
        newPosition.y = center.y - (size.y / 2);
        changed = (newPosition != position);
        if (changed)
        {
            // Update the window's position if it changed
            position = newPosition;
            this->center = center;
            setPosition(position);
            textureView.setCenter(center);
        }
    }

    // Update the preview box position, which hovers under the player's mouse pointer
    if (updatePreview)
        preview.setPosition(center);
}

void MagicWindow::setSize(const sf::Vector2f& newSize)
{
    changed = (size != newSize);
    if (changed)
    {
        // Update the size if it changed
        size = newSize;
        border.setSize(size);
        preview.setSize(size);
        preview.setOrigin(size.x / 2, size.y / 2);
        texture.create(size.x, size.y);
        textureView.setSize(size);

        // Update the center position
        setCenter(center, true, false);
    }
}

void MagicWindow::setSize(unsigned newBlockSize)
{
    blockSize = std::max(std::min(newBlockSize, MAX_BLOCK_SIZE), MIN_BLOCK_SIZE);
    setSize(sf::Vector2f(tileSize.x * blockSize, tileSize.y * blockSize));
}

bool MagicWindow::hasChanged() const
{
    return changed;
}

sf::RenderTexture& MagicWindow::getTexture()
{
    return texture;
}

bool MagicWindow::isWithin(const sf::Vector2u& pos) const
{
    // Checks if a point is within the visible window
    return (visible && sf::IntRect(position.x, position.y, size.x, size.y).contains(sf::Vector2i(pos.x, pos.y)));
}

bool MagicWindow::isWithin(const sf::FloatRect& aabb) const
{
    // Checks if a rectangle is within the visible window
    return (visible && sf::FloatRect(position.x, position.y, size.x, size.y).intersects(aabb));
}

void MagicWindow::show(bool state)
{
    visible = state;
}

bool MagicWindow::isVisible() const
{
    return visible;
}

void MagicWindow::setView(const sf::View& view, const sf::Vector2f& windowViewPos)
{
    // A regular full sized view is passed in (the size of the window)
    // Use the render texture position and size to calculate the new position
    // of the new view to use for the render texture.

    auto viewRect = getViewRect(view);
    auto textureViewRect = getViewRect(textureView);
    sf::Vector2f absolutePosition(position.x - windowViewPos.x, position.y - windowViewPos.y);
    textureViewRect.left = viewRect.left + absolutePosition.x;
    textureViewRect.top = viewRect.top + absolutePosition.y;
    textureViewRect.width = size.x;
    textureViewRect.height = size.y;
    textureView.reset(textureViewRect);
    texture.setView(textureView);
}

void MagicWindow::draw(sf::RenderTarget& target, sf::RenderStates states) const
{
    states.transform *= getTransform();
    if (visible)
    {
        // Draw the contents of the window if visible
        sf::Sprite sprite(texture.getTexture());
        target.draw(sprite, states);
        target.draw(border, states);
    }
    target.draw(preview);
}

void MagicWindow::handleResize(int delta)
{
    setSize(blockSize + delta);
}

void MagicWindow::handleKeyPressed(const sf::Event::KeyEvent& keyEvent)
{
    if (keyEvent.control)
    {
        if (keyEvent.code == sf::Keyboard::Add || keyEvent.code == sf::Keyboard::E)
            handleResize(1);
        else if (keyEvent.code == sf::Keyboard::Subtract || keyEvent.code == sf::Keyboard::Q)
            handleResize(-1);
    }
}
