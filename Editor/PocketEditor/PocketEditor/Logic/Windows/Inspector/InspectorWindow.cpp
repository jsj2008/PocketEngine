//
//  CompilationWindow.cpp
//  PocketEditor
//
//  Created by Jeppe Nielsen on 05/04/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "InspectorWindow.hpp"
#include "VirtualTreeListSystem.hpp"
#include "VirtualTreeListSpawnerSystem.hpp"
#include "EditorObject.hpp"
#include "FieldEditorSystem.hpp"
#include "GameObjectEditorSystem.hpp"
#include "Project.hpp"

std::string InspectorWindow::Name() { return "Inspector"; }

void InspectorWindow::OnInitialize() {
    
    //selectables = world.CreateSystem<SelectableCollection<EditorObject>>();
    //selectables->SelectionChanged.Bind(this, &InspectorWindow::SelectionChanged);
    
    GameObject& guiRoot = context->GuiRoot();
    
    guiRoot.CreateSystem<FieldEditorSystem>()->gui = &context->Gui();
    guiRoot.CreateSystem<GameObjectEditorSystem>()->gui = &context->Gui();
    guiRoot.CreateSystem<GameObjectEditorSystem>()->Predicate = [] (int componentId) -> bool {
        return componentId != GameIdHelper::GetComponentID<EditorObject>();
    };
    
    guiRoot.CreateSystem<LayoutSystem>();

    selectables = 0;
    
    removeComponentMenu.InitializePopUp();
    removeComponentMenu.AddChild("Add Reference").Clicked.Bind([this] () {
        if (!currentWorld) return;
        int componentIndex;
        if (!context->World().TryGetComponentIndex(clickedComponent.typeInfo.name, componentIndex)) {
            return;
        }
        
        GameObject* referenceObject = TryGetFirstObjectWithComponent(componentIndex);
        if (referenceObject) {
            for(auto o : selectables->Selected()) {
                o->GetComponent<EditorObject>()->gameObject->ReplaceComponent(componentIndex, referenceObject);
            }
        }
        
        context->postActions.emplace_back([this] () {
            RefreshInspector();
        });
    });
    removeComponentMenu.AddChild("Remove").Clicked.Bind([this] () {
        if (!currentWorld) return;
        int componentIndex;
        if (!context->World().TryGetComponentIndex(clickedComponent.typeInfo.name, componentIndex)) {
            return;
        }
        for(auto o : selectables->Selected()) {
            o->GetComponent<EditorObject>()->gameObject->RemoveComponent(componentIndex);
        }
        
        context->postActions.emplace_back([this] () {
            RefreshInspector();
        });
    });
}

void InspectorWindow::ActiveWorldChanged(OpenWorld* old, OpenWorld* current) {
    if (old) {
        old->EditorRoot.Changed.Unbind(this, &InspectorWindow::EditorRootChanged, old);
    }
    if (current) {
        current->EditorRoot.Changed.Bind(this, &InspectorWindow::EditorRootChanged, current);
    }
    ChangeEditorRoot(old ? old->EditorRoot : 0, current ? current->EditorRoot : 0);
}

void InspectorWindow::EditorRootChanged(OpenWorld *world) {
    ChangeEditorRoot(world->EditorRoot.PreviousValue(), world->EditorRoot);
}

void InspectorWindow::ChangeEditorRoot(Pocket::GameObject *old, Pocket::GameObject *current) {
    if (old) {
        old->CreateSystem<SelectableCollection<EditorObject>>()->SelectionChanged.Unbind(this, &InspectorWindow::SelectionChanged);
    }
    if (current) {
        selectables = current->CreateSystem<SelectableCollection<EditorObject>>();
        selectables->SelectionChanged.Bind(this, &InspectorWindow::SelectionChanged);
    } else {
        selectables = 0;
    }
    inspectorEditor->GetComponent<GameObjectEditor>()->Object = 0;
}

void InspectorWindow::OnCreate() {

/*

    GameWorld& world = context->Project().World();
    GameObject* go = world.CreateObject();
 
    go->AddComponent<Transform>();
    go->AddComponent<Mesh>()->GetMesh<Vertex>().AddCube(0, {2,1,1});
    go->AddComponent<Renderable>();
    go->AddComponent<EditorObject>();
    go->AddComponent<Rotator>();

*/

    GameObject& guiWorld = context->GuiRoot();
    Gui& gui = context->Gui();
    
    guiWorld.CreateSystem<FieldEditorSystem>()->gui = &gui;
    GameObjectEditorSystem* gameObjectEditorSystem = guiWorld.CreateSystem<GameObjectEditorSystem>();
    gameObjectEditorSystem->gui = &gui;
    gameObjectEditorSystem->scriptWorld = &context->Project().ScriptWorld();
    
    addComponentButton = gui.CreateLabelControl(window, "Box", {0,400-40}, 40, 0, "+", 10);
    addComponentButton->GetComponent<Touchable>()->Click.Bind(this, &InspectorWindow::AddComponentClicked);
    addComponentButton->Enabled = false;
    
    GameObject* pivot;
    listBox = gui.CreateListbox(window, "Box", {0,0}, {200,400-80}, &pivot);
    listBox->RemoveComponent<Sprite>();
    
    
    inspectorEditor = gui.CreatePivot(pivot);
    inspectorEditor->AddComponent<Sizeable>()->Size = {200,400};
    inspectorEditor->AddComponent<GameObjectEditor>()->Object = 0;
    inspectorEditor->GetComponent<GameObjectEditor>()->ComponentEditorCreated.Bind(
    [this](GameObjectEditor::ComponentCreatedData data) {
        data.editorPivot->GetComponent<Touchable>()->Click.Bind(this, &InspectorWindow::ComponentClicked, data);
    });
    
    selectionBox = 0;
    
    window->GetComponent<Transform>()->Position += {600,200};
}

void InspectorWindow::ComponentClicked(Pocket::TouchData d, GameObjectEditor::ComponentCreatedData data) {
    if (d.Index != 1) return;
    clickedComponent = data;
    removeComponentMenu.ShowPopup(d.Input->GetTouchPosition(d.Index));
}

void InspectorWindow::SelectionChanged(SelectableCollection<EditorObject> *selectables) {
    inspectorEditor->GetComponent<GameObjectEditor>()->Object = selectables->Selected().empty() ? 0 : selectables->Selected()[0]->GetComponent<EditorObject>()->gameObject;
    addComponentButton->Enabled = !selectables->Selected().empty();
}

void InspectorWindow::AddComponentClicked(Pocket::TouchData d) {
    if (!selectionBox) {
        ShowSelectionBox(selectables->Selected()[0]->GetComponent<EditorObject>());
    } else {
        selectionBox->Remove();
        selectionBox = 0;
    }
}

void InspectorWindow::ShowSelectionBox(EditorObject *editorObject) {
    Gui& gui = context->Gui();

    selectionBox = gui.CreateControl(window);
    
    auto componentTypes = context->World().GetComponentTypes();
    
    Vector2 pos;
    pos.x = window->GetComponent<Sizeable>()->Size().x;
    pos.y = window->GetComponent<Sizeable>()->Size().y;
    for(int i=0; i<componentTypes.size(); ++i) {
        auto& componentType = componentTypes[i];
        if (editorObject->gameObject->GetComponent(i)) continue;
        //if (!componentType.getTypeInfo) continue;
        
        GameObject* button = gui.CreateLabelControl(selectionBox, "Box", pos, {200,20}, 0, componentType.name, 20);
        button->GetComponent<Touchable>()->Click.Bind(this, &InspectorWindow::SelectionClicked, i);
        pos += {0,-20};
    }
}

void InspectorWindow::SelectionClicked(TouchData d, int index) {
    selectionBox->Remove();
    selectionBox = 0;
    selectables->Selected()[0]->GetComponent<EditorObject>()->gameObject->AddComponent(index);
    RefreshInspector();
}

void InspectorWindow::RefreshInspector() {
    inspectorEditor->GetComponent<GameObjectEditor>()->Object = 0;
    inspectorEditor->GetComponent<GameObjectEditor>()->Object = selectables->Selected()[0]->GetComponent<EditorObject>()->gameObject;
}

void InspectorWindow::PostCompile() {
    inspectorEditor->GetComponent<GameObjectEditor>()->Object = 0;
}

GameObject* InspectorWindow::TryGetFirstObjectWithComponent(int componentId) {
    
    std::vector<std::string> guids;
    std::vector<std::string> paths;
    context->World().GetPaths(guids, paths);
    for (int i=0; i<paths.size(); ++i) {
        
        std::ifstream file;
        file.open(paths[i]);
            
        
        //std::cout << "Start parse file: "<<p.second<<std::endl;
        
        
        //context.World().TryParseJson(file, GameIdHelper::GetComponentID<Transform>(), [] (int parent, int object) {
        //    std::cout << " parent: " << parent << "  object: " << object << std::endl;
        //});

        bool objectFound = false;
        int objectId = 0;
        context->World().TryParseJson(file, componentId, [&](int parentId, int id) {
            objectFound = true;
            objectId = id;
        });
        
        if (objectFound) {
            GameObject* root = context->World().TryFindRoot(guids[i]);
            if (root) {
                GameObject* object = root->FindObject(objectId);
                if (object) {
                    return object;
                }
            }
        }
    }
    
    return 0;
}
