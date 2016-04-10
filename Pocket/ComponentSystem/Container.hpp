//
//  Container.hpp
//  ComponentSystem
//
//  Created by Jeppe Nielsen on 29/12/15.
//  Copyright © 2015 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include <vector>
#include <assert.h>

namespace Pocket {
  
class IContainer {
public:
    virtual ~IContainer() {}
    virtual void Initialize() = 0;
};

template<typename Object>
class Container : public IContainer {
public:
    struct ObjectInstance {
        Object object;
        int references;
        int index;
        void* owner;
    };
    
    ObjectInstance* CreateObject() {
        TryGrow();
        ObjectInstance* instance = objects[size++];
        assert(instance->references == 0);
        ++instance->references;
        ResetInstance(instance);
        return instance;
    }
    
    ObjectInstance* CreateObjectNoReset() {
        TryGrow();
        ObjectInstance* instance = objects[size++];
        ++instance->references;
        return instance;
    }
    
    int Size() const { return size; }
    
    void RemoveObject(ObjectInstance* instance) {
        --size;
        objects[size]->index = instance->index;
        std::swap(objects[instance->index], objects[size]);
        instance->index = size;
        instance->owner = 0;
    }
    
    void DestroyObject(ObjectInstance* instance) {
        --capacity;
        --size;
        objects[instance->index] = objects[size];
        objects[size] = objects[capacity];
        objects[instance->index]->index = instance->index;
        objects[size]->index = size;

        objects.pop_back();
        instance->owner = 0;
    }
    
    Object* Get(int index) {
        return &objects[index]->object;
    }
    
    void Clear() {
        for (int i=0; i<capacity; ++i) {
            if (objects[i]->references==0) {
                if (!objects[i]->owner) {
                    DeleteInstance(objects[i]);
                }
            } else {
                objects[i]->owner = 0;
            }
        }
        size = 0;
        capacity = 0;
        objects.clear();
    }
    
private:
    
    void TryGrow() {
        if (size<capacity) return;
        Grow((capacity + 20) * 2);
    }

    void Grow(int newCapacity) {
        objects.resize(newCapacity);
        for (int i=size; i<newCapacity; ++i) {
            objects[i] = CreateInstance();
            objects[i]->references = 0;
            objects[i]->owner = 0;
            objects[i]->index = i;
        }
        capacity = newCapacity;
    }
    
    using Objects = std::vector<ObjectInstance*>;
    Objects objects;
    
    int size;
    int capacity;
    
    ObjectInstance* CreateInstance() {
        return new ObjectInstance();
    }
    
    void DeleteInstance(ObjectInstance* instance) {
        delete instance;
    }
    
    void ResetInstance(ObjectInstance* instance) {
        instance->object = defaultObject->object;
    }
    
public:
    Container() : size(0), capacity(0), defaultObject(0) { }
    ~Container() {
        Clear();
        DeleteInstance(defaultObject);
    }
    
    void Initialize() override {
        defaultObject = CreateInstance();
    }
    
    void* createContext;
    void* deleteContext;
    void* resetContext;
    int contextIndex;
    
    ObjectInstance* defaultObject;
};

}