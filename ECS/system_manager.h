#pragma once

#include "entities/entity.h"
#include "systems/system.h"

#include <cassert>
#include <memory>
#include <unordered_map>

namespace ECS
{
    class FSystemManager
    {
    public:
        template<typename T>
        std::shared_ptr<T> RegisterSystem()
        {
            const char* TypeName = typeid(T).name();

            assert(Systems.find(TypeName) == Systems.end() && "Registering system more than once!");

            auto System = std::make_shared<T>();
            Systems.insert({TypeName, System});
            return System;
        }

        template<typename T>
        std::shared_ptr<T> GetSystem()
        {
            const char* TypeName = typeid(T).name();
            assert(Systems.find(TypeName) != Systems.end() && "System not registered!");

            return std::static_pointer_cast<T>(Systems.find(TypeName)->second);
        }

        template<typename T>
        void SetSignature(FSignature Signature)
        {
            const char* TypeName = typeid(T).name();

            assert(Systems.find(TypeName) != Systems.end() && "System doesn't exist!");

            Signatures.insert({TypeName, Signature});
        }

        void EntityDestroyed(FEntity Entity)
        {
            for (auto const& Pair : Systems)
            {
                auto const& System = Pair.second;
                System->Entities.erase(Entity);
            }
        }

        void EntitySignatureChanged(FEntity Entity, FSignature EntitySignature)
        {
            for(auto const& Pair : Systems)
            {
                auto const& Type = Pair.first;
                auto const& System = Pair.second;
                auto const& SystemSignature = Signatures[Type];

                if ((EntitySignature & SystemSignature) == SystemSignature)
                {
                    System->Entities.insert(Entity);
                }
                else
                {
                    System->Entities.erase(Entity);
                }
            }
        }
    private:
        std::unordered_map<const char*, FSignature> Signatures{};
        std::unordered_map<const char*, std::shared_ptr<FSystem>> Systems{};
    };
}