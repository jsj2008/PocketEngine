//
//  DraggerSystem.h
//  PocketEngine
//
//  Created by Jeppe Nielsen on 10/12/13.
//  Copyright (c) 2013 Jeppe Nielsen. All rights reserved.
//

#pragma once
#include "GameSystem.hpp"
#include "Draggable.hpp"
#include "Touchable.hpp"
#include "Transform.hpp"
#include "Plane.hpp"
#include <map>

namespace Pocket {

    template<typename T>
    class DraggableSystem : public GameSystem<T, Transform, Touchable, Draggable> {
    public:
        using GameObject = GameObject<T>;
        
        void ObjectAdded(GameObject *object) {
            Touchable* touchable = object->template GetComponent<Touchable>();
            touchable->Down += event_handler(this, &DraggableSystem::Down, object);
            touchable->Up += event_handler(this, &DraggableSystem::Up, object);
        }

        void ObjectRemoved(GameObject *object) {
            Touchable* touchable = object->template GetComponent<Touchable>();
            touchable->Down -= event_handler(this, &DraggableSystem::Down, object);
            touchable->Up -= event_handler(this, &DraggableSystem::Up, object);
            TouchData d;
            d.Touchable = touchable;
            Up(d, object);
        }

        void Update(float dt) {
            
            for (auto it = draggingObjects.begin(); it!=draggingObjects.end(); ++it) {
                
                DraggingObject& d = it->second;
                Vector2 touchPosition = d.touch.Input->GetTouchPosition(d.touch.Index);
                
                Ray ray = d.touch.Camera->GetRay(d.touch.CameraTransform, touchPosition);
                
                float distance;
                
                if (d.dragPlane.IntersectsRay(ray, &distance)) {
                    Vector3 worldPosition = ray.position + ray.direction * distance;
                    Vector3 localPosition = d.transform->WorldInverse.GetValue()->TransformPosition(worldPosition);
                    
                    Vector3 local = localPosition - d.offset + d.transform->Anchor;
                    
                    Vector3 currentPosition = local;
                    
                    switch (d.draggable->Movement) {
                        case Draggable::MovementMode::XAxis:
                            currentPosition.y = currentPosition.z = 0;
                            break;
                        case Draggable::MovementMode::YAxis:
                            currentPosition.x = currentPosition.z = 0;
                            break;
                        case Draggable::MovementMode::ZAxis:
                            currentPosition.x = currentPosition.y = 0;
                            break;
                        
                        case Draggable::MovementMode::XYPlane:
                            currentPosition.z = 0;
                            break;
                        case Draggable::MovementMode::XZPlane:
                            currentPosition.y = 0;
                            break;
                        case Draggable::MovementMode::YZPlane:
                            currentPosition.x = 0;
                            break;
                        default:
                            currentPosition = local;
                            break;
                    }
                               
                    Vector3 position = d.transform->Local.GetValue()->TransformPosition(currentPosition);//localPosition - d.offset + d.transform->Anchor);
                    d.transform->Position = position;
                }
                
            }
        }

        bool IsTouchIndexUsed(int touchIndex) {
            if (draggingObjects.empty()) return false;
            for (auto it = draggingObjects.begin(); it!=draggingObjects.end(); ++it) {
                if (it->second.touch.Index == touchIndex) return true;
            }
            return false;
        }
        
    private:
        struct DraggingObject {
            TouchData touch;
            Vector3 offset;
            Plane dragPlane;
            Vector3 touchPosition;
            
            Touchable* touchable;
            Transform* transform;
            Draggable* draggable;
        };
        
        typedef std::map<Touchable*, DraggingObject> DraggingObjects;
        DraggingObjects draggingObjects;
        
        void Down(TouchData d, GameObject* object) {
            auto it = draggingObjects.find(d.Touchable);
            if (it!=draggingObjects.end()) return;
            DraggingObject& dragging = draggingObjects[d.Touchable];
            dragging.touchable = d.Touchable;
            dragging.transform = object->template GetComponent<Transform>();
            dragging.touch = d;
            
            dragging.offset = dragging.transform->WorldInverse.GetValue()->TransformPosition(d.WorldPosition);
            dragging.draggable = object->template GetComponent<Draggable>();
            
            Vector3 forward = d.CameraTransform->World.GetValue()->TransformVector(Vector3(0,0,-1));
            
            switch (dragging.draggable->Movement) {
                case  Draggable::MovementMode::XAxis: {
                    Vector3 axis = dragging.transform->World.GetValue()->TransformVector(Vector3(1,0,0));
                    Vector3 up = axis.Cross(forward);
                    dragging.dragPlane = Plane(axis.Cross(up).Normalized(), d.WorldPosition);
                }
                break;
                case  Draggable::MovementMode::YAxis: {
                    Vector3 axis = dragging.transform->World.GetValue()->TransformVector(Vector3(0,1,0));
                    Vector3 up = axis.Cross(forward);
                    dragging.dragPlane = Plane(axis.Cross(up).Normalized(), d.WorldPosition);
                }
                break;
                case  Draggable::MovementMode::ZAxis: {
                    Vector3 axis = dragging.transform->World.GetValue()->TransformVector(Vector3(0,0,1));
                    Vector3 up = axis.Cross(forward);
                    dragging.dragPlane = Plane(axis.Cross(up).Normalized(), d.WorldPosition);
                }
                break;
                case  Draggable::MovementMode::XYPlane: {
                    Vector3 axis = dragging.transform->World.GetValue()->TransformVector(Vector3(0,0,1));
                    dragging.dragPlane = Plane(axis.Normalized(), d.WorldPosition);
                }
                break;
                case  Draggable::MovementMode::XZPlane: {
                    Vector3 axis = dragging.transform->World.GetValue()->TransformVector(Vector3(0,1,0));
                    dragging.dragPlane = Plane(axis.Normalized(), d.WorldPosition);
                }
                break;
                case  Draggable::MovementMode::YZPlane: {
                    Vector3 axis = dragging.transform->World.GetValue()->TransformVector(Vector3(1,0,0));
                    dragging.dragPlane = Plane(axis.Normalized(), d.WorldPosition);
                }
                break;
                    
                default:{
                    dragging.dragPlane = Plane(d.WorldNormal, d.WorldPosition);
                }
                break;
            }
            dragging.draggable->IsDragging = true;
            
        }

        void Up(TouchData d, GameObject* object) {
            auto it = draggingObjects.find(d.Touchable);
            if (it==draggingObjects.end()) return;
            it->second.draggable->IsDragging = false;
            draggingObjects.erase(it);
        }
    };
}