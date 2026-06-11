#pragma once

#include "Entity.h"

namespace World
{
    class Scene final
    {
    public:
        Scene() = default;
        ~Scene() = default;

        Scene(const Scene&) = delete;
        Scene& operator=(const Scene&) = delete;

        void Clear();

        FEntity* CreateEntity(
            Core::StringView name,
            const FTransform& transform = FTransform {}
        );

        bool DestroyEntity(EntityId entityId);

        [[nodiscard]] FEntity* FindEntity(EntityId entityId);
        [[nodiscard]] const FEntity* FindEntity(EntityId entityId) const;

        [[nodiscard]] const Core::Vector<FEntity>& GetEntities() const { return mEntities; }
        [[nodiscard]] Core::usize GetEntityCount() const { return mEntities.size(); }

        [[nodiscard]] bool IsEmpty() const { return mEntities.empty(); }

    private:
        [[nodiscard]] EntityId AllocateEntityId();

    private:
        Core::Vector<FEntity> mEntities;
        EntityId mNextEntityId = 1;
    };
}