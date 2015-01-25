// Copyright (C) 2014-2015 Eric Hebert (ayebear)
// This code is licensed under GPLv3, see LICENSE.txt for details.

#include "aboutstate.h"
#include "gameobjects.h"
#include "spriteloader.h"
#include "colorcode.h"

AboutState::AboutState(GameObjects& objects):
    objects(objects)
{
    setup();
}

void AboutState::handleEvents()
{
    sf::Event event;
    while (objects.window.pollEvent(event))
    {
        if (event.type == sf::Event::Closed)
            stateEvent.command = StateEvent::Exit;
        else if (event.type == sf::Event::KeyPressed || event.type == sf::Event::MouseButtonReleased)
            stateEvent.command = StateEvent::Pop;
    }
}

void AboutState::update()
{
}

void AboutState::draw()
{
    objects.window.clear();
    objects.window.draw(bgSprite);
    for (auto& text: textList)
        objects.window.draw(text);
    objects.window.display();
}

void AboutState::setup()
{
    // Load files and settings
    SpriteLoader::load(bgSprite, "data/images/menu_bg.png", true);
    cfg::File config("data/config/about.cfg");
    font.loadFromFile(config("font"));
    int fontSize = config("fontSize").toInt();
    int padding = config("padding").toInt();
    ColorCode fontColor(config("fontColor"));
    auto viewSize = objects.window.getView().getSize();
    auto& descriptionArray = config("description");
    float totalHeight = (descriptionArray.size() * (fontSize + padding)) - padding;
    float startTop = (viewSize.y - totalHeight) / 2.0f;

    // Setup the sf::Text objects
    int count = 0;
    for (auto& text: descriptionArray)
    {
        textList.emplace_back(text.toString(), font, fontSize);
        auto& line = textList.back();
        line.setColor(fontColor.toColor());

        // Calculate position
        float left = (viewSize.x - line.getGlobalBounds().width) / 2.0f;
        float top = startTop + (count * (fontSize + padding));
        line.setPosition(left, top);

        ++count;
    }
}
