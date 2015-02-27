// Copyright (C) 2014-2015 Eric Hebert (ayebear)
// This code is licensed under GPLv3, see LICENSE.txt for details.

#include "entityprototypeloader.h"

EntityPrototypeLoader::EntityPrototypeLoader(ocs::ObjectManager& objManager, const std::string& configFilename):
    objManager(objManager),
    config(configFilename)
{
}

bool EntityPrototypeLoader::load()
{
    // Load components from config file
    bool status = config.getStatus();
    if (status)
    {
        extractParentInfo();
        loadAllEntities();
    }
    return status;
}

bool EntityPrototypeLoader::load(ocs::ObjectManager& objManager, const std::string& configFilename)
{
    // Create an instance of this class inside of this function, because it needs to hold state
    EntityPrototypeLoader loader(objManager, configFilename);
    return loader.load();
}

void EntityPrototypeLoader::extractParentInfo()
{
    for (auto& section: config)
    {
        // Get inheritance information from section name
        std::string entityName;
        std::vector<std::string> parentEntities;
        splitNames(section.first, entityName, parentEntities);

        // Store parent list and copy original component data
        entToComp[entityName].parentNames.swap(parentEntities);
        entToComp[entityName].componentData.swap(section.second);
    }
}

void EntityPrototypeLoader::loadAllEntities()
{
    for (auto& parentInfo: entToComp)
    {
        std::cout << "Loading entity '" << parentInfo.first << "'...\n";
        visitedParents.clear();
        loadEntity(parentInfo.first, parentInfo.first);
    }
}

void EntityPrototypeLoader::splitNames(const std::string& sectionName, std::string& entityName, std::vector<std::string>& parentEntities)
{
    std::vector<std::string> outerSplit;
    strlib::split(sectionName, ":", outerSplit, false);

    // Set the entity name (whether it has parents or not)
    if (!outerSplit.empty())
        entityName = outerSplit.front();

    // Split and trim parent names (if there are any parents)
    if (outerSplit.size() == 2)
    {
        strlib::split(outerSplit.back(), ",", parentEntities, false);
        for (auto& parentName: parentEntities)
            strlib::trimWhitespace(parentName);
    }
}

void EntityPrototypeLoader::loadEntity(const std::string& entityName, const std::string& parentName)
{
    // Don't process a parent that doesn't exist
    if (entToComp.find(parentName) == entToComp.end())
    {
        std::cout << "  Error: '" << parentName << "' doesn't exist.\n";
        return;
    }

    // Mark this entity name as visited
    visitedParents.insert(parentName);
    //std::cout << "  Visited '" << parentName << "'\n";

    // Loop through any parents, and call this function
    for (auto& parent: entToComp[parentName].parentNames)
    {
        // Make sure parents are never re-visited before recursing
        if (visitedParents.find(parent) == visitedParents.end())
            loadEntity(entityName, parent);
        //else
            //std::cout << "  Skipping '" << parent << "', already visited.\n";
    }

    // Load the components of this entity or parent
    loadComponents(entityName, entToComp[parentName].componentData);
    //std::cout << "  Loaded components from '" << parentName << "'\n";
}

void EntityPrototypeLoader::loadComponents(const std::string& entityName, const cfg::File::Section& section)
{
    // Load each component from a string in the section
    for (auto& option: section)
        objManager.addComponentToPrototypeFromString(entityName, option.first, option.second.toString());
}