//
//  Editor.cpp
//  CraneBuilder
//
//  Created by Jeppe Nielsen on 07/11/15.
//  Copyright (c) 2015 Jeppe Nielsen. All rights reserved.
//

#include "Editor.h"
#include "RenderSystem.hpp"
#include "TerrainMeshSystem.h"
#include "CameraDragSystem.h"
#include "TerrainVertexEditor.h"
#include "BackgroundSystem.h"
#include "DragSelector.hpp"
#include "SelectableDragSystem.hpp"
#include "EditorSpawnerSystem.h"
#include "TouchCancelSystem.hpp"
#include "EditorFactory.h"
#include "TransformHierarchy.hpp"

void Editor::Initialize() {

    world.CreateSystem<RenderSystem>();
    
    world.CreateSystem<CameraDragSystem>()->Input = &Input;
    
    state = world.CreateSystem<ComponentEnablerSystem>();
    world.CreateSystem<EditorSpawnerSystem>()->state = state;
    world.CreateSystem<TouchCancelSystem>();
    
    world.CreateSystem<DragSelector>()->Setup(Manager().Viewport(), Input);
    world.CreateSystem<SelectableDragSystem>();
    world.CreateSystem<TerrainVertexEditor>()->SetInput(&Input);
    BackgroundSystem* backgroundSystem = world.CreateSystem<BackgroundSystem>();
    world.CreateSystem<TerrainMeshSystem>();
    
    factory = world.CreateFactory<EditorFactory>();
    simulationFactory = world.CreateFactory<SimulationFactory>();
    
    factory->Setup();
    simulationFactory->Setup(&Input);
    
    GameObject* camera = world.CreateObject();
    camera->AddComponent<Transform>()->Position = {0,0,40};
    camera->AddComponent<Camera>()->Viewport = Manager().Viewport();
    camera->AddComponent<CameraDragger>();
    
    backgroundSystem->CameraObject = camera;
    
    GameObject* background = world.CreateObject();
    background->AddComponent<Transform>();
    background->AddComponent<Material>();
    background->AddComponent<Mesh>();
    background->AddComponent<Orderable>()->Order = -5000;
    Background* back = background->AddComponent<Background>();
    back->colors[0] = Colour((Colour::Component)255, 223, 166);
    back->colors[3] = back->colors[0];
    back->colors[1] = Colour((Colour::Component)226, 172,102);
    back->colors[2] = back->colors[1];
    
    level = world.CreateObject();
    
     {
        
        GameObject* go = world.CreateObject();
        go->AddComponent<Transform>()->Position = {0,0,-1};
        go->AddComponent<Mesh>();
        go->AddComponent<Touchable>();
        go->AddComponent<TouchableCanceller>()->MovementToCancel = 10;
        go->GetComponent<TouchableCanceller>()->trackTouchPosition = true;
        go->AddComponent<EditorSpawner>()->CurrentObject = "object";
        
        EditorSpawner::StateObjects terrainStates = {
            { "object", [this] (Vector3 p) {
                GameObject* g = factory->CreateTerrain(p);
                g->Parent = level;
                return g;
            }}
        };
        
        EditorSpawner::StateObjects physicsStates = {
            { "object", [this] (Vector3 p) {
                GameObject* g = simulationFactory->CreateParticle(p);
                g->GetComponent<Particle>()->immovable = true;
                g->AddComponent<ComponentEnabler>()->AddComponent<Touchable>("physics");
                g->Parent = level;
                return g;
            }}
        };
        
        go->GetComponent<EditorSpawner>()->objects =
            {
                {
                    "terrain",
                    terrainStates
                },
                {
                    "physics",
                    physicsStates
                }
            };
    }
    
    Input.ButtonDown += event_handler(this, &Editor::ButtonDown);
}

void Editor::ButtonDown(std::string button) {
    if (button == "1") {
        state->CurrentState = "none";
    } else if (button == "2") {
        state->CurrentState = "terrain";
    }else if (button == "3") {
        state->CurrentState = "physics";
    } else if (button == " ") {
        SaveLevel("Level.txt");
    } else if (button == "l") {
        LoadLevel("Level.txt");
    }
}

void Editor::Update(float dt) {
    world.Update(dt);
}

void Editor::Render() {
    world.Render();
}

void Editor::SaveLevel(std::string filename) {
    std::ofstream file;
    file.open (filename);
    level->ToJson(file, [] (GameObject* o, int componentType) {
        if (componentType == TerrainEditableVertices::ID) return false;
        if (componentType == Selectable::ID) return false;
        if (componentType == Draggable::ID) return false;
        if (o->GetComponent<Terrain>()) {
            if (componentType == TouchableCanceller::ID) return false;
            if (componentType == Touchable::ID) return false;
            if (componentType == TerrainEditableVertices::ID) return false;
        }
        if (componentType == ComponentEnabler::ID) return false;
        return true;
    });
    file.close();
}

void Editor::LoadLevel(std::string filename) {
    std::ifstream file;
    file.open(filename);
    level->Remove();
    level = world.CreateObjectFromJson(file, [this] (GameObject* o) {
        if (o->GetComponent<Terrain>()) {
            o->AddComponent<Touchable>();
            o->AddComponent<TouchableCanceller>()->MovementToCancel = 10.0f;
            o->GetComponent<TouchableCanceller>()->trackTouchPosition = true;
            o->AddComponent<TerrainEditableVertices>();
        } else if (o->GetComponent<Particle>()) {
            o->AddComponent<Touchable>();
            o->AddComponent<Draggable>()->Movement = Draggable::MovementMode::XYPlane;
            o->AddComponent<Selectable>();
        }
    });
}
