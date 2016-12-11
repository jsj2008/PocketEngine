//
//  OpenWorld.cpp
//  PocketEditor
//
//  Created by Jeppe Nielsen on 13/09/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "OpenWorld.hpp"
#include "RenderSystem.hpp"
#include "TouchSystem.hpp"
#include "EditorObjectCreatorSystem.hpp"
#include "TransformHierarchy.hpp"
#include "FileHelper.hpp"
#include "InputManager.hpp"

#include "DraggableSystem.hpp"
#include "EditorTransformSystem.hpp"
#include "EditorMeshSystem.hpp"
#include "ClickSelectorSystem.hpp"
#include "DragSelector.hpp"
#include "SelectableDragSystem.hpp"
#include "SelectedColorerSystem.hpp"
#include "ClonerSystem.hpp"
#include "FirstPersonMoverSystem.hpp"
#include "ScriptWorld.hpp"
#include "InputMapperSystem.hpp"
#include <fstream>
#include "Project.hpp"
#include "CloneVariable.hpp"
#include "VelocitySystem.hpp"

struct Turner {
    
    Turner() {
        speed.push_back({0,1,2});
        speed.push_back({0,4,3});
    }
    
    struct Movement {
        int power;
        TYPE_FIELDS_BEGIN
        TYPE_FIELD(power)
        TYPE_FIELDS_END
    };
    
    Movement movement;

    std::vector<Vector3> speed;
    
    TYPE_FIELDS_BEGIN
    TYPE_FIELD(speed)
    TYPE_FIELD(movement)
    TYPE_FIELDS_END
};

struct TurnerSystem : public GameSystem<Transform, Turner> {
    void Update(float dt) {
        for(auto o : Objects()) {
            Turner* turner = o->GetComponent<Turner>();
            for(auto& v : turner->speed) {
                std::cout << v << std::endl;
            }
           // o->GetComponent<Transform>()->EulerRotation += o->GetComponent<Turner>()->speed() * dt;
        }
    }
};

OpenWorld::OpenWorld() : root(0) {
    IsPlaying.Changed.Bind([this] () {
        UpdateTimeScale();
    });
}

void OpenWorld::CreateDefaultSystems(Pocket::GameObject &world) {
    world.CreateSystem<RenderSystem>();
    world.CreateSystem<TransformHierarchy>();
    world.CreateSystem<TouchSystem>()->TouchDepth = 0;
    world.CreateSystem<ClonerSystem>();
    world.CreateSystem<TurnerSystem>();
    world.CreateSystem<EditorObjectCreatorSystem>();
    world.CreateSystem<InputMapperSystem>();
    world.CreateSystem<VelocitySystem>();
}

void OpenWorld::CreateEditorSystems(Pocket::GameObject &editorWorld) {
    editorWorld.CreateSystem<RenderSystem>();
    editorWorld.CreateSystem<TouchSystem>();
    editorWorld.CreateSystem<DraggableSystem>();
    editorWorld.CreateSystem<EditorTransformSystem>();
    editorWorld.CreateSystem<EditorMeshSystem>();
    editorWorld.CreateSystem<ClickSelectorSystem>();
    editorWorld.CreateSystem<DragSelector>()->Setup({2000,2000});
    editorWorld.CreateSystem<SelectableDragSystem>();
    editorWorld.CreateSystem<TouchSystem>()->TouchDepth = 5;
    editorWorld.CreateSystem<SelectedColorerSystem>();
    editorWorld.CreateSystem<FirstPersonMoverSystem>();
    editorWorld.CreateSystem<SelectableCollection<EditorObject>>();
}

bool OpenWorld::Save() {
    Stop();
    bool succes = false;
    try {
        succes = true;
        std::ofstream file;
        file.open(Path);
        root->ToJson(file, [] (const GameObject* go, int componentID) {
            if (componentID == Pocket::GameIdHelper::GetComponentID<EditorObject>()) return false;
            if (go->Parent() && go->Parent()->GetComponent<Cloner>()) return false;
            return true;
        });
        file.close();
    } catch (std::exception e) {
        succes = false;
    }
    return succes;
}

bool OpenWorld::Load(const std::string &path, const std::string &filename, GameWorld& w, ScriptWorld& s) {
    Path = path;
    Filename = filename;
    world = &w;
    scriptWorld = &s;
    
    if (path != "") {
        std::ifstream file;
        
        file.open(path);
        std::string guid = world->ReadGuidFromJson(file);
        file.close();
            
        root = world->TryFindRoot(guid);
        if (!root) {
            return false;
        }
        
        /*
        if (!root) {
            root = world->CreateRootFromJson(file, [this] (GameObject* root) {
                OpenWorld::CreateDefaultSystems(*root);
                scriptWorld->AddGameRoot(root);
                root->AddComponent<EditorObject>();
            },
            [](GameObject* go) {
                go->AddComponent<EditorObject>();
            });
        }
        file.close();
        if (!root) {
            return false;
        }
        */
    } else {
        root = world->CreateRoot();
    }
    
    CreateDefaultSystems(*root);
    s.AddGameRoot(root);
    AddEditorObject(root);
    //auto var = root->AddComponent<CloneVariable>();
    //var->Variables.push_back({ GameIdHelper::GetClassName<Transform>(), "Rotation" });
    
    editorRoot = world->CreateRoot();
    //std::cout << "EditorRoot::scene " << editorRoot->scene<<std::endl;
    editorRoot->Order = 1;
    CreateEditorSystems(*editorRoot);
    selectables = editorRoot->CreateSystem<SelectableCollection<EditorObject>>();
    
    editorCamera = editorRoot->CreateObject();
    editorCamera->AddComponent<Camera>();
    editorCamera->AddComponent<Transform>()->Position = { 0, 0, 10 };
    editorCamera->GetComponent<Camera>()->FieldOfView = 70;
    editorCamera->AddComponent<FirstPersonMover>()->SetTouchIndices(2, 1);
    
    InitializeRoot();
    return true;
}

void OpenWorld::InitializeRoot() {
    root->CreateSystem<EditorObjectCreatorSystem>()->editorRoot = editorRoot;
    RenderSystem* worldRenderSystem = root->CreateSystem<RenderSystem>();
    RenderSystem* editorRenderSystem = editorRoot->CreateSystem<RenderSystem>();
    worldRenderSystem->SetCameras(editorRenderSystem->GetCameras());
    
    UpdateTimeScale();
}

void OpenWorld::Play() {
    if (IsPlaying) return;
    storedWorld.clear();
    root->ToJson(storedWorld);
    IsPlaying = true;
}

void OpenWorld::Stop() {
    if (!IsPlaying) return;
    root->Remove();
    
    //std::cout << storedWorld.str() << std::endl;
    root = world->CreateRootFromJson(storedWorld, [] (GameObject* root) {
        CreateDefaultSystems(*root);
    });
    root->CreateSystem<EditorObjectCreatorSystem>()->editorRoot = editorRoot;
    InitializeRoot();
    //std::cout << "EditorRoot::scene " << editorRoot->scene<<std::endl;
    
    IsPlaying = false;
}

GameObject* OpenWorld::Root() {
    return root;
}

void OpenWorld::Close() {
    root->Remove();
    editorRoot->Remove();
}

void OpenWorld::Enable() {
    root->UpdateEnabled() = true;
    root->RenderEnabled() = true;
    editorRoot->UpdateEnabled() = true;
    editorRoot->RenderEnabled() = true;
}

void OpenWorld::Disable() {
    root->UpdateEnabled() = false;
    root->RenderEnabled() = false;
    editorRoot->UpdateEnabled() = false;
    editorRoot->RenderEnabled() = false;
}

void OpenWorld::UpdateTimeScale() {
    if (root) {
        root->TimeScale() = IsPlaying() ? 1.0f : 0.0f;
    }
}

void OpenWorld::AddEditorObject(Pocket::GameObject *object) {
    object->AddComponent<EditorObject>();
    for (auto child : object->Children()) {
        AddEditorObject(child);
    }
}

void OpenWorld::PreCompile() {
    for(auto go : selectables->Selected()) {
        EditorObject* editorObject = go->GetComponent<EditorObject>();
        selectedObjectsAtCompileTime.push_back(editorObject->gameObject->RootId());
    }
    selectables->ClearSelection();
    root->ToJson(compilingWorld);
}

void OpenWorld::PostCompile() {
    root->Remove();
    std::cout << compilingWorld.str() << std::endl;
    root = world->CreateRootFromJson(compilingWorld, [this] (GameObject* root) {
        CreateDefaultSystems(*root);
        scriptWorld->AddGameRoot(root);
    });
    root->CreateSystem<EditorObjectCreatorSystem>()->editorRoot = editorRoot;
    InitializeRoot();
    Compiled();
    world->Update(0);
    for (auto id : selectedObjectsAtCompileTime) {
        GameObject* go = root->FindObject(id);
        if (!go) continue;
        EditorObject* editorObject = go->GetComponent<EditorObject>();
        Selectable* selectable = editorObject->editorObject->GetComponent<Selectable>();
        selectable->Selected = true;
    }
    selectedObjectsAtCompileTime.clear();
}




