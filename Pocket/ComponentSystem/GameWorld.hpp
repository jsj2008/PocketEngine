//
//  GameWorld.hpp
//  EntitySystem
//
//  Created by Jeppe Nielsen on 06/06/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "GameObject.hpp"
#include "Container.hpp"
#include "GameSystem.hpp"
#include "InputManager.hpp"

namespace Pocket {
    class ScriptWorld;
    class GameWorld {
    public:
        GameWorld();
        ~GameWorld();
        
        const GameObject* Root();
        
        GameObject* CreateObject();
        GameObject* CreateObject(std::istream &jsonStream, GameObject* parent = 0, std::function<void(GameObject*)> onCreated = 0);
        
        template<typename T>
        T* CreateSystem() {
            return static_cast<T*>(
            TryAddSystem(
            GameIDHelper::GetSystemID<T>(),
            [this](std::vector<int>& components)
                {
                    T* system = new T;
                    GameWorld** worldC = ((GameWorld**)&system->world);
                    *(worldC) = this;
                    system->ExtractAllComponents(components);
                    return system;
                }
            ));
        }

		template<typename T>
		T* TryGetSystem() {
			int systemId = GameIDHelper::GetSystemID<T>();
			if (systemId < systemsIndexed.size()) {
				return static_cast<T*>(systemsIndexed[systemId].system);
			}
			else {
				return 0;
			}
		}
        
        template<typename T>
        void RemoveSystem() {
            TryRemoveSystem(GameIDHelper::GetSystemID<T>());
        }
        
        void Update(float dt);
        void Render();
        
        int ObjectCount() const;
        int CapacityCount() const;
        
        void Clear();
        void Trim();
        
        using ComponentInfos = std::vector<GameObject::ComponentInfo>;
        const ComponentInfos& ComponentTypes();
        
        using SerializePredicate = std::function<bool(GameObject*, int)>;
        void ToJson(std::ostream& stream, SerializePredicate predicate = 0);
        
        static std::function<void(int, GameObject::ComponentInfo&)> OnGetTypeInfo;
        
        InputManager& Input();
    private:
    
        GameObject root;
        
        struct GameObjectEntry {
            GameObject object;
        };
    
        using Objects = std::deque<GameObjectEntry>;
        Objects objects;
        using ObjectsFreeIndicies = std::vector<int>;
        ObjectsFreeIndicies objectsFreeIndicies;
        
        using Components = std::vector<IContainer*>;
        Components components;
        
        struct ObjectComponent {
            int index;
            IContainer* container;
        };
        
        using ObjectComponents = std::vector<std::vector<ObjectComponent>>;
        ObjectComponents objectComponents;
        
        struct SystemEntry {
            SystemEntry() : system(0), deleteFunction(0) {}
            IGameSystem* system;
            Bitset bitset;
            std::function<void()> deleteFunction;
        };
        
        using SystemEntries = std::vector<SystemEntry>;
        SystemEntries systemsIndexed;
        
        using Systems = std::vector<IGameSystem*>;
        Systems systems;
        
        using SystemsPerComponent = std::vector<std::vector<int>>;
        SystemsPerComponent systemsPerComponent;
        
        using Action = std::function<void()>;
        using Actions = std::deque<Action>;
        Actions delayedActions;
        
        ComponentInfos componentInfos;
        
        int objectCount;
        int numComponentTypes;
        
        struct ObjectID {
            GameObject* object;
            std::string id;
        };
        std::vector<ObjectID> objectIDs;
        
        InputManager input;
        
        IGameSystem* TryAddSystem(SystemID id, std::function<IGameSystem*(std::vector<int>& components)> constructor);
        void TryRemoveSystem(SystemID id);
        void DoActions(Actions& actions);
        void IterateObjects(std::function<void(GameObject*)> callback);
        void TryAddComponentContainer(ComponentID id, std::function<IContainer *(GameObject::ComponentInfo&)>&& constructor);
        std::string* GetObjectID(GameObject* object);
        void AddObjectID(GameObject* object, std::string id);
        std::string* FindIDFromReferenceObject(GameObject* referenceObject, int componentID);
        GameObject* FindObjectFromID(const std::string &id);
        GameObject* FindFirstObjectWithComponentID(int componentID);
        GameObject* LoadObject(GameObject* parent, minijson::istream_context &context, std::function<void(GameObject*)>& onCreated);
        bool TryGetComponentIndex(const std::string& componentName, int& index);
        bool TryGetComponentIndex(const std::string& componentName, int& index, bool& isReference);
        
        friend class GameObject;
        friend class GameSystemBase;
        friend class ScriptWorld;
    };
    
    
    template<typename T>
    T* GameObject::GetComponent() {
        ComponentID id = GameIDHelper::GetComponentID<T>();
        if (id>=world->numComponentTypes) return 0;
        auto& objectComponent = world->objectComponents[id][index];
        if (objectComponent.index == -1) return 0;
        Container<T>* container = static_cast<Container<T>*>(objectComponent.container);
        return &container->entries[objectComponent.index];
    }

    
    
}