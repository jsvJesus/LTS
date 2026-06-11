#include "Scene.h"

#include <algorithm>

#include "Core/BaseTypes.h"

namespace World
{
    void Scene::Clear()
    {
        mEntities.clear();
        mNextEntityId = 1;
    }

    FEntity* Scene::CreateEntity(
        const Core::StringView name,
        const FTransform& transform
    )
    {
        FEntity entity {};
        entity.Id = AllocateEntityId();
        entity.Name = Core::String(name.data(), name.size());
        entity.Transform = transform;
        entity.Alive = true;

        mEntities.push_back(entity);

        return &mEntities.back();
    }

    bool Scene::DestroyEntity(const EntityId entityId)
    {
        const auto it = std::find_if(
            mEntities.begin(),
            mEntities.end(),
            [entityId](const FEntity& entity)
            {
                return entity.Id == entityId;
            }
        );

        if (it == mEntities.end())
            return false;

        mEntities.erase(it);
        return true;
    }

    FEntity* Scene::FindEntity(const EntityId entityId)
    {
        const auto it = std::find_if(
            mEntities.begin(),
            mEntities.end(),
            [entityId](const FEntity& entity)
            {
                return entity.Id == entityId;
            }
        );

        if (it == mEntities.end())
            return nullptr;

        return &(*it);
    }

    const FEntity* Scene::FindEntity(const EntityId entityId) const
    {
        const auto it = std::find_if(
            mEntities.begin(),
            mEntities.end(),
            [entityId](const FEntity& entity)
            {
                return entity.Id == entityId;
            }
        );

        if (it == mEntities.end())
            return nullptr;

        return &(*it);
    }

    EntityId Scene::AllocateEntityId()
    {
        const EntityId id = mNextEntityId;
        ++mNextEntityId;

        if (!IsValidEntityId(mNextEntityId))
        {
            mNextEntityId = 1;
        }

        return id;
    }
}
