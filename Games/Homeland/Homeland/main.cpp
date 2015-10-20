#include "Engine.hpp"
#include "GameWorld.hpp"
#include "RenderSystem.hpp"
#include "MapMeshSystem.h"
#include "FirstPersonMoverSystem.hpp"
#include "TransformHierarchy.hpp"
#include "DragSelector.hpp"
#include "SelectionVisualizer.hpp"
#include "VelocitySystem.hpp"
#include "GroundSystem.h"
#include "MoveSystem.h"
#include "PathFinderSystem.h"
#include "ClickTargetSystem.h"
#include "TouchSystem.hpp"
#include "MovableAlignmentSystem.h"
#include "MovableCollisionSystem.h"
#include "NavMesh.hpp"
#include "Timer.hpp"

using namespace Pocket;

class Game : public GameState<Game> {
public:
    GameWorld world;
    RenderSystem* renderer;
    GameObject* camera;
    GameObject* cube;
    float rotation;
    GameObject* map;
    GameObject* cameraObject;
    
    void Initialize() {
        
        auto fpm = world.CreateSystem<FirstPersonMoverSystem>();
        fpm->Input = &Input;
        fpm->FlipControls = true;
        world.CreateSystem<TransformHierarchy>();
        world.CreateSystem<TouchSystem>()->Input = &Input;
        world.CreateSystem<MapMeshSystem>();
        renderer = world.CreateSystem<RenderSystem>();
        renderer->Shaders.LitTextured.SetValue("LightDirection", Vector3(-1,-1,-1).Normalized());
        world.CreateSystem<DragSelector>()->Setup(Manager().Viewport(), Input);
        world.CreateSystem<SelectionVisualizer>();
        world.CreateSystem<VelocitySystem>();
        world.CreateSystem<PathFinderSystem>();
        world.CreateSystem<MoveSystem>();
        world.CreateSystem<MovableAlignmentSystem>();
        world.CreateSystem<GroundSystem>();
        world.CreateSystem<ClickTargetSystem>();
        world.CreateSystem<MovableCollisionSystem>();
        
        map = world.CreateObject();
        
        
        map->AddComponent<Map>()->CreateMap(64, 64);
        map->GetComponent<Map>()->Randomize(-13.1f, 15.0f);
        map->GetComponent<Map>()->Smooth(5);
        map->GetComponent<Map>()->SetMaxHeight(1.0f);
        //map->GetComponent<Map>()->SetHeight(0.5f);
        
       // map->GetComponent<Map>()->AddHill(35, 45, 8, -3.0f);
        
        /*
        map->GetComponent<Map>()->SetHeight(1.0f);
        map->GetComponent<Map>()->AddHill(32, 32, 16, 5.0f);
        map->GetComponent<Map>()->AddHill(32, 32, 4, -7.0f);
        //map->GetComponent<Map>()->AddHill(5, 5, 4, 7.0f);
        map->GetComponent<Map>()->AddHill(35, 5, 4, 7.0f);
        map->GetComponent<Map>()->AddHill(35, 45, 8, -3.0f);
        */
        //map->GetComponent<Map>()->GetNode(10, 10).height = 0;
        //map->GetComponent<Map>()->GetNode(11, 10).height = 0;
        //map->GetComponent<Map>()->GetNode(11, 11).height = 0;
        //map->GetComponent<Map>()->GetNode(10, 11).height = 0;
        
        //map->GetComponent<Map>()->SetEdges(1.0f);
        
        
        Timer timer;
        timer.Begin();
        auto mesh = map->GetComponent<Map>()->CreateNavigationMesh();
        double time = timer.End();
        std::cout << "Nav mesh generation time = " << time << std::endl;
        
        map->GetComponent<Map>()->renderSystem = renderer;
        
        
        GameObject* navMesh = world.CreateObject();
        navMesh->AddComponent<Transform>();
        navMesh->AddComponent<Material>()->Shader = &renderer->Shaders.Colored;
        navMesh->GetComponent<Material>()->BlendMode = BlendModeType::Alpha;
        auto& navMeshMesh = navMesh->AddComponent<Mesh>()->GetMesh<Vertex>();
        
        int i=0;
        for (auto p : mesh) {
            
            Vertex v;
            v.Position = {p.x, 1.05f, p.y};
            v.Color = Colour::HslToRgb((i/3)*10, 1, 1, 1);
            navMeshMesh.vertices.push_back(v);
            navMeshMesh.triangles.push_back(i);
            i++;
        }
        navMeshMesh.Flip();
        
        
        cameraObject = world.CreateObject();
        cameraObject->AddComponent<Transform>()->Position = {0,0,0};
        cameraObject->AddComponent<Mesh>();
        cameraObject->AddComponent<MapRenderer>()->width = 70;
        cameraObject->GetComponent<MapRenderer>()->depth = 48;
        cameraObject->AddComponent<Map>(map);
        cameraObject->AddComponent<TextureComponent>()->Texture().LoadFromPng("grass.png");
        cameraObject->AddComponent<Material>()->Shader = &renderer->Shaders.LitTextured;
        cameraObject->AddComponent<FirstPersonMover>()->RotationSpeed = 0;
        cameraObject->AddComponent<Touchable>();
        
        camera = world.CreateObject();
        camera->Parent = cameraObject;
        camera->AddComponent<Camera>()->Viewport = Manager().Viewport();
        camera->AddComponent<Transform>()->Position = Vector3(0, 10, 5) * 1.5f;
        camera->GetComponent<Transform>()->Rotation = Quaternion::LookAt(camera->GetComponent<Transform>()->Position, {0,0,0}, Vector3(0,1,0));
        camera->GetComponent<Camera>()->FieldOfView = 70;
        
        GameObject* waterPlane = world.CreateObject();
        waterPlane->Parent = cameraObject;
        waterPlane->AddComponent<Transform>()->Position = {0,0.1f,0};
        waterPlane->AddComponent<Mesh>()->GetMesh<Vertex>().AddPlane(0, {76, 48}, {0,0,1,1});
        waterPlane->GetComponent<Mesh>()->GetMesh<Vertex>().SetColor(Colour(0,0,1.0f,0.5f));
        waterPlane->AddComponent<Material>()->BlendMode = BlendModeType::Alpha;
        waterPlane->GetComponent<Material>()->Shader = &renderer->Shaders.Colored;
        
        for (int i=0; i<10; i++) {
    
        cube = world.CreateObject();
        cube->AddComponent<Mappable>()->Map = map->GetComponent<Map>();
        cube->AddComponent<Transform>()->Position = {15.0f+i*2.0f,1.0f,20.0f};
        cube->AddComponent<Mesh>()->GetMesh<Vertex>().AddCube(0, {0.95f,0.2f,1.0f});
        cube->AddComponent<Material>()->Shader = &renderer->Shaders.LitColored;
        cube->AddComponent<Selectable>();
        cube->AddComponent<Groundable>();
        cube->AddComponent<Movable>()->Speed = 10.0f;
        cube->GetComponent<Movable>()->Target = cube->GetComponent<Transform>()->Position;
        
        GameObject* turret = world.CreateObject();
        turret->Parent = cube;
        turret->AddComponent<Transform>()->Position = {0,0.4f,0};
        turret->AddComponent<Mesh>()->GetMesh<Vertex>().AddCube({0.0f,0,0.5f}, {0.04f, 0.04f, 0.8f});
        turret->AddComponent<Material>()->Shader = &renderer->Shaders.LitColored;
        
        }
        
        
        GameObject* obj = world.CreateObject();
        obj->AddComponent<Transform>()->Position = {155,0,155};
        obj->AddComponent<Mesh>()->GetMesh<Vertex>().AddCube(0, 1);
        obj->AddComponent<Material>();
    
        follow = false;
    
        Input.ButtonDown += event_handler(this, &Game::ButtonDown);
        wireframe = false;
    }
    
    bool follow;
    
    void ButtonDown(std::string b) {
        if (b == "m") {
            cube->GetComponent<Movable>()->Target = {155,0,155};
        } else if (b == "n") {
            cube->GetComponent<Movable>()->Target = {15,0,20};
        } else if (b=="f") {
            follow = !follow;
        } else if (b=="w") {
            wireframe = !wireframe;
        }
        
    }
    
    void Update(float dt) {
        world.Update(dt);
        if (follow) {
            Vector3 pos = cube->GetComponent<Transform>()->Position;
            pos.y = 0;
            cameraObject->GetComponent<Transform>()->Position = pos;
        }
    }
    
    void Render() {
        if (wireframe) {
            glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        } else {
            glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        }
        world.Render();
    }
    
    bool wireframe;
};

int main_behes() {
    Engine e;
    e.Start<Game>();
	return 0;
}