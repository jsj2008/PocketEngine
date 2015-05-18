//
//  SpriteMeshSystem.h
//  PocketEngine
//
//  Created by Jeppe Nielsen on 9/1/13.
//  Copyright (c) 2013 Jeppe Nielsen. All rights reserved.
//
#pragma once
#include "GameWorld.hpp"
#include "Mesh.hpp"
#include "Sizeable.hpp"
#include "Sprite.hpp"

namespace Pocket {
    class Sizeable;
    class SpriteMeshSystem : public GameSystem {
    public:
        void Initialize();
    protected:    
        void ObjectAdded(GameObject* object);
        void ObjectRemoved(GameObject* object);
        
        void SizeChanged(Sizeable* sizeable, GameObject* object);
        void CornerSizeChanged(Sprite* sprite, GameObject* object);
        
        void UpdateMesh(GameObject* object);
    };
}