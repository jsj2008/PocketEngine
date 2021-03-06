//
//  IGameSystem.hpp
//  TestComponentSystem
//
//  Created by Jeppe Nielsen on 17/10/16.
//  Copyright © 2016 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "Property.hpp"

namespace Pocket {
    class GameObject;
    
    struct IGameSystem {
        virtual ~IGameSystem() = default;
        virtual void Initialize() = 0;
        virtual void Destroy() = 0;
        virtual void ObjectAdded(GameObject* object) = 0;
        virtual void ObjectRemoved(GameObject* object) = 0;
        virtual void Update(float dt) = 0;
        virtual void Render() = 0;
        virtual int AddObject(GameObject* object) = 0;
        virtual void RemoveObject(GameObject* object) = 0;
        virtual int ObjectCount() = 0;
        virtual int GetOrder() = 0;
        virtual void SetOrder(int order) = 0;
        virtual int GetIndex() = 0;
        virtual void SetIndex(int index) = 0;
        
    };
}