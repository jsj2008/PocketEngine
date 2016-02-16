#include "Engine.hpp"
#include "GameWorld.hpp"
#include "RenderSystem.hpp"

struct TimerDestroyer {
    float time;
};

template<typename T>
struct TimerDestroyerSystem : GameSystem<T, TimerDestroyer> {
    void ObjectAdded(GameObject<T>* o) {}
    void ObjectRemoved(GameObject<T>* o) {}
    void Update(float dt) {
        for(auto o : this->Objects()) {
            TimerDestroyer* d = o->template GetComponent<TimerDestroyer>();
            d->time -= dt;
            if (d->time<0) {
                o->Remove();
            }
        }
    }
};

struct Spinner {
    Vector3 speed;
};

template<typename T>
struct SpinnerSystem : GameSystem<T, Spinner, Transform> {
    void ObjectAdded(GameObject<T>* o) {}
    void ObjectRemoved(GameObject<T>* o) {}
    void Update(float dt) {
        for(auto o : this->Objects()) {
            Transform* t = o->template GetComponent<Transform>();
            Spinner* s = o->template GetComponent<Spinner>();
            Quaternion rot = Quaternion(s->speed * dt);
            Quaternion val = t->Rotation * rot;
            t->Rotation = val;
        }
    }
};


struct GameWorldSettings :
    GameSettings<
        RenderSystem<GameWorldSettings>,
        TimerDestroyerSystem<GameWorldSettings>,
        SpinnerSystem<GameWorldSettings>
    > {
};


using namespace Pocket;

class Game : public GameState<Game> {
public:
    GameWorld<GameWorldSettings> world;
    
    using GameObject = GameObject<GameWorldSettings>;
    
    GameObject* camera;
    GameObject* cube;
    float rotation;
    
    void Initialize() {
    
        world.Initialize();
        
        camera = world.CreateObject();
        camera->AddComponent<Camera>()->Viewport = Manager().Viewport();
        camera->AddComponent<Transform>()->Position = { 50, 50,100 };
        camera->GetComponent<Camera>()->FieldOfView = 40;
        
        
    }
    
    void CreateObjects() {
    for(int x = 0; x<450; x++)
        {
        
        cube = world.CreateObject();
        cube->AddComponent<Transform>()->Position = {fmodf(x, 10)*2.5f,floorf(x / 10.0f)*2.5f,0};
        cube->AddComponent<Mesh>()->GetMesh<Vertex>().AddCube(0, 1);
        cube->AddComponent<Material>();
        
        cube->AddComponent<TimerDestroyer>()->time = 0.13f + x * 0.015f;
        cube->AddComponent<Spinner>()->speed = {1,2,0};
        
        auto& verts = cube->GetComponent<Mesh>()->GetMesh<Vertex>().vertices;
        
        for (int i=0; i<verts.size(); i++) {
            verts[i].Color = Colour::HslToRgb(i * 10, 1, 1, 1);
        }
        
        }

    }
    
    void Update(float dt) {
        world.Update(dt);
        
        if (world.ObjectCount()==1) {
            CreateObjects();
        }
        
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