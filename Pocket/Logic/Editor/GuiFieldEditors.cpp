//
//  FieldInfoEditors.cpp
//  PocketEditor
//
//  Created by Jeppe Nielsen on 28/11/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "GuiFieldEditors.hpp"
#include <fstream>
#include "FileReader.hpp"

using namespace Pocket;

template<typename T, int Size>
struct FieldInfoEditorTextboxes : public GuiFieldEditor {

    void SetField(void* field) override {
        this->field = static_cast<T*>(field);
    }

    void Initialize(Gui* gui, GameObject* parent) override {
        GameObject* control = gui->CreateControl(parent);
        control->AddComponent<Layouter>()->ChildrenLayoutMode = Layouter::LayoutMode::Horizontal;
        
        for(int i=0; i<Size; ++i) {
            GameObject* textBox = gui->CreateTextBox(control, "TextBox", 0, 0, 0, "", 15.0f);
            textBox->GetComponent<Touchable>()->ClickThrough = true;
            textBox->Children()[0]->GetComponent<TextBox>()->Active.Changed.Bind(this, &FieldInfoEditorTextboxes<T, Size>::TextChanged, textBox);
            textBox->Children()[0]->GetComponent<Colorable>()->Color = Colour::Black();
            textBox->AddComponent<Layouter>()->Min = {20, 20};
            textBox->GetComponent<Layouter>()->Desired = {100, 20};
            textBox->GetComponent<Layouter>()->Max = {5000, 20};
            
            textBoxes[i] = textBox;
        }
        prev = (*this->field);
        forceUpdate = true;
    }
    
    void Destroy() override {
        for(int i=0; i<Size; ++i) {
             textBoxes[i]->Children()[0]->template GetComponent<TextBox>()->Active.Changed.Unbind(this, &FieldInfoEditorTextboxes<T, Size>::TextChanged, textBoxes[i]);
             textBoxes[i]->Remove();
        }
    }
    
    void TextChanged(GameObject* textBoxGO) {
        TextBox* textBox = textBoxGO->Children()[0]->GetComponent<TextBox>();
        if (textBox->Active) return;
        
        for(int i=0; i<Size; ++i) {
             if (textBoxes[i] == textBoxGO) {
                TextboxChanged(i, textBox->Text);
             }
        }
    }
    
    void Update(float dt) override {
        bool changed = prev!=(*this->field) || forceUpdate;
        if (changed) {
            forceUpdate = false;
            for(int i=0; i<Size; ++i) {
                if (!textBoxes[i]->Children()[0]->template GetComponent<TextBox>()->Active()) {
                    std::stringstream s;
                    UpdateTextbox(i,s);
                    textBoxes[i]->Children()[0]->template GetComponent<TextBox>()->Text = s.str();
                }
            }
        }
        prev = (*this->field);
    }

    virtual void TextboxChanged(int index, std::string text) = 0;
    virtual void UpdateTextbox(int index, std::stringstream& stream) = 0;
    GameObject* textBoxes[Size];
    
    T* field;
    T prev;
    bool forceUpdate;
};

struct FieldInfoEditorFloat : public FieldInfoEditorTextboxes<float, 1> {
    void TextboxChanged(int index, std::string text) override {
        float value = (float)atof(text.c_str());
        (*field) = value;
    }
    void UpdateTextbox(int index, std::stringstream& stream) override {
        stream<<(*field);
    }
};

struct FieldInfoEditorInt : public FieldInfoEditorTextboxes<int, 1> {
    void TextboxChanged(int index, std::string text) override {
        int value = (int)atoi(text.c_str());
        (*field) = value;
    }
    void UpdateTextbox(int index, std::stringstream& stream) override {
        stream<<(*field);
    }
};

struct FieldInfoEditorVector2 : public FieldInfoEditorTextboxes<Vector2, 2> {
    void TextboxChanged(int index, std::string text) override {
        float value = (float)atof(text.c_str());
        (*field).Set(index, value);
    }
    void UpdateTextbox(int index, std::stringstream& stream) override {
        stream<<(*field)[index];
    }
};

struct FieldInfoEditorVector3 : public FieldInfoEditorTextboxes<Vector3, 3> {
    void TextboxChanged(int index, std::string text) override {
        float value = (float)atof(text.c_str());
        (*field).Set(index, value);
    }
    void UpdateTextbox(int index, std::stringstream& stream) override {
        stream<<(*field)[index];
    }
};



struct FieldInfoEditorQuaternion : public FieldInfoEditorTextboxes<Quaternion, 3> {
    void TextboxChanged(int index, std::string text) override {
        float value = (float)atof(text.c_str());
    
        Vector3 euler = (*field).ToEuler();
        euler *= MathHelper::RadToDeg;
        euler.Set(index, value);
        euler *= MathHelper::DegToRad;
        (*field) = Quaternion(euler);
    }
    
    void UpdateTextbox(int index, std::stringstream& stream) override {
        Vector3 euler = (*field).ToEuler();
        euler *= MathHelper::RadToDeg;
        stream<<euler[index];
    }
};

struct FieldInfoEditorString : public FieldInfoEditorTextboxes<std::string, 1> {
    void TextboxChanged(int index, std::string text) override {
        (*field) = text;
    }
    void UpdateTextbox(int index, std::stringstream& stream) override {
        stream<<(*field);
    }
};

template<> IFieldEditor* FieldEditorCreator<int>::Create(int* ptr) {
    FieldInfoEditorInt* editor = new FieldInfoEditorInt();
    editor->SetField(ptr);
    return editor;
}

template<> IFieldEditor* FieldEditorCreator<float>::Create(float* ptr) {
    FieldInfoEditorFloat* editor = new FieldInfoEditorFloat();
    editor->SetField(ptr);
    return editor;
}

template<> IFieldEditor* FieldEditorCreator<Vector2>::Create(Vector2* ptr) {
    FieldInfoEditorVector2* editor = new FieldInfoEditorVector2();
    editor->SetField(ptr);
    return editor;
}

template<> IFieldEditor* FieldEditorCreator<Vector3>::Create(Vector3* ptr) {
    FieldInfoEditorVector3* editor = new FieldInfoEditorVector3();
    editor->SetField(ptr);
    return editor;
}

template<> IFieldEditor* FieldEditorCreator<Quaternion>::Create(Quaternion* ptr) {
    FieldInfoEditorQuaternion* editor = new FieldInfoEditorQuaternion();
    editor->SetField(ptr);
    return editor;
}

template<> IFieldEditor* FieldEditorCreator<std::string>::Create(std::string* ptr) {
    FieldInfoEditorString* editor = new FieldInfoEditorString();
    editor->SetField(ptr);
    return editor;
}

TypeEditorTitle::Callback TypeEditorTitle::Title = [] (void* guiPtr, void* parentPtr, const std::string& title) -> void* {
    Gui* gui = static_cast<Gui*>(guiPtr);
    GameObject* parent = static_cast<GameObject*>(parentPtr);
    
    GameObject* labelGo = gui->CreateLabel(parent, 0, 0, 0, title, 15);
    labelGo->AddComponent<Layouter>()->Min = {20,20};
    labelGo->AddComponent<Layouter>()->Desired = {100,20};
    labelGo->AddComponent<Layouter>()->Max = {5000,20};
    Label* label = labelGo->GetComponent<Label>();
    labelGo->GetComponent<Colorable>()->Color = Colour::Black();
    label->HAlignment = Font::Center;
    label->VAlignment = Font::Middle;
    
    return 0;
};

struct ReferenceComponentEditor : public GuiFieldEditor {

    GameObject::ReferenceComponent component;
    GameObject* control;
    GameObject* parent;

    void SetField(void* field) override {
        component = *static_cast<GameObject::ReferenceComponent*>(field);
        menu = 0;
    }

    void Initialize(Gui* gui, GameObject* parent) override {
    
        this->parent = parent;
    
        control = gui->CreateControl(parent);
        control->AddComponent<Layouter>()->ChildrenLayoutMode = Layouter::LayoutMode::Horizontal;
        
        GameObject* textBox = gui->CreateControl(control, "TextBox");
        textBox->AddComponent<Layouter>()->Min = {20, 20};
        textBox->GetComponent<Layouter>()->Desired = {100, 20};
        textBox->GetComponent<Layouter>()->Max = {5000, 20};
        textBox->GetComponent<Touchable>()->Click.Bind([this] (TouchData d){
            MenuClicked();
        });
        
        GameObject* owner = component.object->GetComponentOwner(component.componentId);
        std::string path = owner->TryGetRootPath();
        std::string text = FileReader::GetFileNameFromPath(path);
        
        GameObject* label = gui->CreateLabel(textBox, 0, 10, 0, text, 20);
        label->AddComponent<Layouter>(textBox);
        label->AddComponent<Colorable>()->Color = Colour::Black();
        
        TypeEditorTitle::Title(gui, parent, component.name);
        
        this->gui = gui;
        
        this->parent->World()->Input().TouchDown.Bind(this, &ReferenceComponentEditor::TouchUp);
    }
    
    void TouchUp(TouchEvent e) {
        if (menu) {
            menu->Remove();
        }
        menu = 0;
    }
    
    void Destroy() override {
        this->parent->World()->Input().TouchDown.Unbind(this, &ReferenceComponentEditor::TouchUp);
    }
    
    void Update(float dt) override {
        
    }
    
    GameObject* menu;
    Gui* gui;
    
    struct ClickedData {
        std::string guid;
        int objectId;
    };
    
    void MenuClicked() {
        if (!component.object->World()->GetPaths) {
            return;
        }
        
        if (menu) {
            menu->Remove();
        }
        
        GameWorld* world =component.object->World();
        
        std::vector<std::string> guids;
        std::vector<std::string> paths;
        world->GetPaths(guids, paths);
        
        
    
        menu = gui->CreateControl(0, "TextBox", 0, {200,200});
        menu->AddComponent<Layouter>()->ChildrenLayoutMode = Layouter::LayoutMode::Vertical;
        menu->GetComponent<Transform>()->Position = control->GetComponent<Transform>()->World().TransformPosition(0);
        
        for (int i=0; i<guids.size(); ++i) {
        
            std::ifstream file;
            file.open(paths[i]);
            
            
            //std::cout << "Start parse file: "<<p.second<<std::endl;
            
            
            //context.World().TryParseJson(file, GameIdHelper::GetComponentID<Transform>(), [] (int parent, int object) {
            //    std::cout << " parent: " << parent << "  object: " << object << std::endl;
            //});

        
            world->TryParseJson(file, component.componentId, [&, this](int parentId, int id) {
                GameObject* button = gui->CreateControl(menu, "Box");
                button->AddComponent<Layouter>()->Desired = { 200, 30 };
                button->GetComponent<Layouter>()->Min = {50,30};
                button->GetComponent<Layouter>()->Max = {500,30};
                
                std::string text = FileReader::GetFileNameFromPath(paths[i]);
                
                GameObject* label = gui->CreateLabel(button, 0, 10, 0, text, 20);
                label->AddComponent<Layouter>(button);
                label->AddComponent<Colorable>()->Color = Colour::Black();
                
                button->GetComponent<Touchable>()->Down.Bind(this, &ReferenceComponentEditor::Clicked, { guids[i], id } );
                
            });
            
        
            std::cout << "guid: " << guids[i] << "  path:" << paths[i] << std::endl;
        }
        
        
    
    
    }
    
    void Clicked(TouchData touch, ClickedData d) {
        
        if (menu) {
            menu->Remove();
            menu = 0;
        }
        
        std::cout << "Guid : " << d.guid << "  object id :"<< d.objectId<<std::endl;
        
        GameObject* root = component.object->World()->TryFindRoot(d.guid);
        if (!root) return;
        GameObject* object = root->FindObject(d.objectId);
        component.object->ReplaceComponent(component.componentId, object);
    }
    
};

template<> IFieldEditor* FieldEditorCreator<GameObject::ReferenceComponent>::Create(Pocket::GameObject::ReferenceComponent *ptr) {
    ReferenceComponentEditor* editor = new ReferenceComponentEditor();
    editor->SetField(ptr);
    return editor;
}






