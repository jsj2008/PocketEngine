//
//  GameObjectEditorSystem.cpp
//  GUIEditor
//
//  Created by Jeppe Nielsen on 24/09/15.
//  Copyright (c) 2015 Jeppe Nielsen. All rights reserved.
//

#include "GameObjectEditorSystem.hpp"
#include "FieldEditor.hpp"
#include "Layoutable.hpp"
using namespace Pocket;

GameObjectEditorSystem::GameObjectEditorSystem() : scriptWorld(0) {}

void GameObjectEditorSystem::Initialize(GameWorld *world) {
    this->world = world;
}

void GameObjectEditorSystem::ObjectAdded(GameObject *object) {
    object->GetComponent<GameObjectEditor>()->Object.Changed.Bind(this, &GameObjectEditorSystem::ObjectChanged, object);
    ObjectChanged(object);
}

void GameObjectEditorSystem::ObjectRemoved(GameObject *object) {
    object->GetComponent<GameObjectEditor>()->Object.Changed.Unbind(this, &GameObjectEditorSystem::ObjectChanged, object);
}

void GameObjectEditorSystem::ObjectChanged(GameObject* object) {
    GameObjectEditor *editor = object->GetComponent<GameObjectEditor>();
    for (auto child : object->Children()) {
        child->Remove();
    }
    if (!editor->Object()) return;
    
    Vector2 size = { 200, 100 };
    
    auto infos = editor->Object()->GetComponentTypes([this] (int componentID) {
        return std::find(ignoredComponents.begin(), ignoredComponents.end(), componentID)==ignoredComponents.end();
    });
    
    if (scriptWorld) {
        int numberOfScriptComponents = scriptWorld->ComponentCount();
        
        for(int i=0; i<numberOfScriptComponents; ++i) {
            TypeInfo info = scriptWorld->GetTypeInfo(*editor->Object(), i);
            if (!info.fields.empty()) {
                infos.push_back(info);
            }
        }
    }
    
    int counter = 0;
    for (auto info : infos) {
        std::cout << "Field : " << info.name << std::endl;
    
        bool hasNoEditors = true;
        for (auto field : info.fields) {
            if (field->HasEditor()) {
                hasNoEditors = false;
                break;
            }
        }
        if (hasNoEditors) {
            continue;
        }
        
        
        GameObject* componentChild = world->CreateObject();
        componentChild->Parent = object;
        componentChild->AddComponent<Transform>()->Position = {0, counter*size.y,0};
        componentChild->AddComponent<Sizeable>()->Size = size;
        componentChild->AddComponent<Layoutable>()->HorizontalAlignment = Layoutable::HAlignment::Relative;
        componentChild->GetComponent<Layoutable>()->ChildLayouting = Layoutable::ChildLayouting::VerticalStackedFit;
        
        GameObject* space = world->CreateObject();
        space->Parent = componentChild;
        space->AddComponent<Transform>();
        space->AddComponent<Sizeable>()->Size = {size.x, 10};
        space->AddComponent<Layoutable>();
        for (auto field : info.fields) {
            if (!field->HasEditor()) continue;
            
            GameObject* editor = world->CreateObject();
            editor->Parent = componentChild;
            editor->AddComponent<Transform>();
            editor->AddComponent<Sizeable>()->Size = {size.x, size.y/info.fields.size()};
            editor->AddComponent<Layoutable>();
            FieldEditor* fieldEditor = editor->AddComponent<FieldEditor>();
            fieldEditor->SetType(info);
            fieldEditor->Field = field->name;
        }
        
        GameObject* componentName = gui->CreateLabel(componentChild, 0, {size.x, 20}, 0, info.name, 14);
        componentName->GetComponent<Label>()->HAlignment = Font::Center;
        componentName->GetComponent<Label>()->VAlignment = Font::Middle;
        componentName->GetComponent<Colorable>()->Color = Colour::Black();
        componentName->AddComponent<Layoutable>();
    
        counter++;
    }
}