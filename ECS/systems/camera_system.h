#include "system.h"

#include "components/camera_component.h"

namespace ECS
{
    namespace SYSTEMS
    {
        class FCameraSystem : public FSystem
        {
        private:
            COMPONENTS::FCameraComponent& GetComponent(FEntity CameraEntity);

        public:
            void MoveCameraForward(FEntity CameraEntity, float Value);
            void MoveCameraRight(FEntity CameraEntity, float Value);
            void MoveCameraUpward(FEntity CameraEntity, float Value);
            void LookUp(FEntity CameraEntity, float Value);
            void LookRight(FEntity CameraEntity, float Value);
            void Roll(FEntity CameraEntity, float Value);
            FMatrix4 GetProjectionMatrix(FEntity CameraEntity);
            FMatrix4 GetViewMatrix(FEntity CameraEntity);
            void Orthogonize(FEntity CameraEntity);
        };
    }
}