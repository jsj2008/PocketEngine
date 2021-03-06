//
//  GameObject.hpp
//  TestComponentSystem
//
//  Created by Jeppe Nielsen on 17/10/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <vector>
#include "Property.hpp"
#include "DirtyProperty.hpp"
#include "IGameObject.hpp"
#include "InputManager.hpp"
#include "TypeInfo.hpp"

namespace minijson {
  class array_writer;
}

namespace Pocket {
    class GameWorld;
    class GameScene;
    class GameObject;
    class IGameSystem;
    class GameObjectHandle;
    
    using ObjectCollection = std::vector<GameObject*>;
    
    using SerializePredicate = std::function<bool(const GameObject*, int)>;
    
    class GameObject : public IGameObject {
    private:
        friend class GameWorld;
        friend class GameScene;
        friend class Handle<GameObject>;
        friend class Container<GameObject>;
        friend class std::allocator<GameObject>;
        friend class ScriptWorld;
        friend class GameObjectHandle;
        
        using ComponentIndicies = std::vector<int>;
        
        GameScene* scene;
        ComponentIndicies componentIndicies;
        
        Bitset activeComponents;
        Bitset enabledComponents;
    public:
        Property<GameObject*> Parent;
        
    private:
        ObjectCollection children;
        
        bool removed;
        int index;
        int rootId;
        
    public:
        Property<bool> Enabled;
        DirtyProperty<bool> WorldEnabled;
        Property<int> Order;
        
        struct ReferenceComponent {
            GameObject* object;
            int componentId;
            std::string name;
        };
        
    private:
    
        struct AddReferenceComponent {
            GameObject* object;
            int componentID;
            std::string referenceId;
        };
        
        using AddReferenceComponentList = std::vector<AddReferenceComponent>;
        
        struct CloneReferenceComponent {
            GameObject* object;
            int componentId;
            int referenceObjectId;
        };
    
        GameObject();
        ~GameObject();
        GameObject(const GameObject& other);
    
        void ParentChanged();
        void Reset();
        void TrySetComponentEnabled(ComponentId id, bool enable);
        void SetWorldEnableDirty();
        void SetEnabled(bool enabled);
        void TryAddToSystem(int systemId);
        void TryRemoveFromSystem(int systemId);
        void WriteJson(minijson::object_writer& writer, const SerializePredicate& predicate) const;
        void WriteJsonComponents(minijson::object_writer& writer, const SerializePredicate& predicate) const;
        void RemoveComponents(const SerializePredicate& predicate);
        
        void SerializeComponent(int componentID, minijson::array_writer& writer, bool isReference, const GameObject* referenceObject) const;
        void AddComponent(AddReferenceComponentList& addReferenceComponents, minijson::istream_context& context, std::string componentName);
        
        static bool GetAddReferenceComponent(AddReferenceComponentList& addReferenceComponents, Pocket::GameObject **object, int &componentID, Pocket::GameObject** referenceObject);
        static void EndGetAddReferenceComponent();
        GameObject* CreateChildCloneInternal(std::vector<CloneReferenceComponent>& referenceComponents, GameObject* source, const std::function<bool(GameObject*)>& predicate = 0);
    public:
        bool Recurse(const std::function<bool(const GameObject* object)>& function) const;
        bool HasComponent(ComponentId id) const override;
        void* GetComponent(ComponentId id) const override;
        void AddComponent(ComponentId id) override;
        void AddComponent(ComponentId id, GameObject* referenceObject) override;
        void RemoveComponent(ComponentId id) override;
        void CloneComponent(ComponentId id, GameObject* object) override;
        void ReplaceComponent(ComponentId id, GameObject* referenceObject) override;
        void EnableComponent(ComponentId id, bool enable) override;
        GameObject* GetComponentOwner(ComponentId id) const;
        
        template<typename T>
        bool HasComponent() const {
            return HasComponent(GameIdHelper::GetComponentID<T>());
        }
        
        template<typename T>
        T* GetComponent() const {
            return static_cast<T*>(GetComponent(GameIdHelper::GetComponentID<T>()));
        }
        
        template<typename T>
        T* AddComponent();
        
        template<typename T>
        T* AddComponent(GameObject* source);
        
        template<typename T>
        void RemoveComponent() {
            RemoveComponent(GameIdHelper::GetComponentID<T>());
        }
        
        template<typename T>
        T* CloneComponent(GameObject* source) {
            ComponentId componentId = GameIdHelper::GetComponentID<T>();
            CloneComponent(componentId, source);
            return static_cast<T*>(GetComponent(componentId));
        }
        
        template<typename T>
        T* ReplaceComponent(GameObject* source);
        
        template<typename T>
        void EnableComponent(bool enable) {
            EnableComponent(GameIdHelper::GetComponentID<T>(), enable);
        }
        
        template<typename T>
        Handle<T> GetComponentHandle() const;
        
        std::vector<TypeInfo> GetComponentTypes(const std::function<bool(int componentID)>& predicate = 0) const ;
        
        struct ComponentEditor {
            TypeInfo type;
            IFieldEditor* editor;
        };
        
        std::vector<ComponentEditor> GetComponentEditors(const std::function<bool(int componentID)>& predicate = 0) const;
        std::vector<int> GetComponentIndicies() const;
        
        InputManager& Input() const;
        
        void Remove() override;
        bool IsRemoved() const;
        
        GameObject* CreateChild();
        GameObject* CreateObject();
        GameObject* Root() const;
        GameObject* CreateChildFromJson(std::istream& jsonStream, const std::function<void(GameObject*)>& objectCreated = 0);
        GameObject* CreateChildClone(GameObject* source, const std::function<bool(GameObject*)>& predicate = 0);
        GameObject* CreateCopy(const std::function<bool(GameObject*)>& predicate = 0);
        
        void ToJson(std::ostream& stream, const SerializePredicate& predicate = 0) const;
        
        bool IsRoot() const;
        
        const ObjectCollection& Children() const;
        
        Handle<GameObject> GetHandle() const;
        
        int RootId() const;
        std::string& RootGuid() const;
        
        template<typename T>
        T* CreateSystem();
        
        template<typename T>
        void RemoveSystem();
        
        GameWorld* World() const;
        
        TypeInfo GetComponentTypeInfo(int index) const;
    
        Property<bool>& UpdateEnabled();
        Property<float>& TimeScale();
        Property<bool>& RenderEnabled();
        
        GameObject* FindObject(int objectId) const;
        
        std::string TryGetRootPath() const;
        
        template<typename T>
        GameObject* GetComponentOwner();
        
        void SetCallbacks(
       		const std::function<void(GameObject* object)>& ObjectCreated,
        	const std::function<void(GameObject* object)>& ObjectRemoved,
        	const std::function<void(GameObject* object, ComponentId componentId)>& ComponentCreated,
        	const std::function<void(GameObject* object, ComponentId componentId)>& ComponentRemoved
        ) const;
        
        bool HasAncestor(GameObject* ancestor) const;
        
        inline friend std::ostream& operator<<(std::ostream& stream, const GameObject& object) {
            stream<<object.RootGuid()<<":"<<object.rootId;
            return stream;
        }
        
        static GameObject* Deserialize(const std::string& data);
    };
}
