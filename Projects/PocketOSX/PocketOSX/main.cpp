//
//  main.cpp
//  PocketOSX
//
//  Created by Jeppe Nielsen on 06/03/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#include "Engine.hpp"
#include "GameWorld.hpp"
#include "RenderSystem.hpp"
#include "DraggableSystem.hpp"
#include "TouchSystem.hpp"
#include "FirstPersonMoverSystem.hpp"
#include <fstream>

using namespace Pocket;

class Game : public GameState<Game> {
public:
    GameWorld world;
    GameObject* camera;
    GameObject* cube;
    float rotation;
    
    struct ClickColorSystem : GameSystem<Mesh, Touchable> {
    
        int number = 5;
    
        void Click(TouchData e, GameObject* go) {
            auto& verts = go->GetComponent<Mesh>()->GetMesh<Vertex>().vertices;
        
            for (int i=0; i<verts.size(); i++) {
                verts[i].Color = Colour::HslToRgb(i * 10 + number*10, 1, 1, 1);
            }
            
            number++;
        }
    
        void ObjectAdded(GameObject* go) {
            go->GetComponent<Touchable>()->Click.Bind(this, &ClickColorSystem::Click, go);
        }
        
        void ObjectRemoved(GameObject* go) {
            go->GetComponent<Touchable>()->Click.Unbind(this, &ClickColorSystem::Click, go);
        }
    };
    
    void Initialize() {
        
        world.CreateSystem<RenderSystem>();
        world.CreateSystem<TouchSystem>()->Input = &Input;
        world.CreateSystem<DraggableSystem>();
        world.CreateSystem<ClickColorSystem>();
        world.CreateSystem<FirstPersonMoverSystem>()->Input = &Input;
        
        camera = world.CreateObject();
        camera->AddComponent<Camera>();
        camera->AddComponent<Transform>()->Position = { 0, 0, 10 };
        camera->GetComponent<Camera>()->FieldOfView = 40;
        camera->AddComponent<FirstPersonMover>();
        
        /*
        
        cube = world.CreateObject();
        cube->AddComponent<Transform>();
        cube->AddComponent<Mesh>()->GetMesh<Vertex>().AddCube(0, 1);
        cube->AddComponent<Material>();
        cube->AddComponent<Draggable>();
        cube->AddComponent<Touchable>();
        
        std::ofstream file;
        file.open("mesh.json");
        cube->ToJson(file);
        file.close();
        */
        
        std::ifstream file;
        file.open("mesh.json");
        cube = world.CreateObject(file);
        file.close();
        
        
        auto& verts = cube->GetComponent<Mesh>()->GetMesh<Vertex>().vertices;
        
        for (int i=0; i<verts.size(); i++) {
            verts[i].Color = Colour::HslToRgb(i * 10, 1, 1, 1);
        }
        
        rotation = 0;
    
        
    
    
    }
    
    void Update(float dt) {
        cube->GetComponent<Transform>()->Rotation = Quaternion(rotation, Vector3(1,0.2f,0.5f).Normalized());
        rotation += dt;
        world.Update(dt);
    }
    
    void Render() {
        world.Render();
    }
};

int main() {
    Engine e;
    e.Start<Game>();
	return 0;
}