#include "systems/camera_system.h"
#include "controller.h"

FController::FController(std::shared_ptr<FRender> RenderIn) : Render(RenderIn)
{
    Camera = RenderIn->CreateCamera();
	RenderIn->SetActiveCamera(Camera);
}

void FController::SetWindow(GLFWwindow* WindowIn)
{
    Window = WindowIn;
}

void FController::SetCameraDirection(float XPos, float YPos)
{
    XCurrent = XPos;
    YCurrent = YPos;
}

void FController::EnableCameraControlMode()
{
    CameraControlMode = true;
}

void FController::DisableCameraControlMode()
{
    CameraControlMode = false;
}

void FController::ToggleFirstUpdateSinceRMB()
{
    FirstUpdateSinceRMB = true;
}

void FController::Update(float Time)
{
    static float CameraMovementSpeed = 3.4f;
    static float CameraRotationSpeed = 0.4f;

    if (!CameraControlMode)
        return;

	float SpeedModifier = 1.;

	if (glfwGetKey(Window, GLFW_KEY_LEFT_CONTROL))
	{
		SpeedModifier = 0.1;
	}

    if (glfwGetKey(Window, GLFW_KEY_W))
    {
        CAMERA_SYSTEM()->MoveCameraForward(Camera, Time * CameraMovementSpeed * SpeedModifier);
    }
    if (glfwGetKey(Window, GLFW_KEY_S))
    {
        CAMERA_SYSTEM()->MoveCameraForward(Camera, -Time * CameraMovementSpeed * SpeedModifier);
    }
    if (glfwGetKey(Window, GLFW_KEY_A))
    {
        CAMERA_SYSTEM()->MoveCameraRight(Camera, -Time * CameraMovementSpeed * SpeedModifier);
    }
    if (glfwGetKey(Window, GLFW_KEY_D))
    {
        CAMERA_SYSTEM()->MoveCameraRight(Camera, Time * CameraMovementSpeed * SpeedModifier);
    }
    if (glfwGetKey(Window, GLFW_KEY_Z))
    {
        CAMERA_SYSTEM()->MoveCameraUpward(Camera, -Time * CameraMovementSpeed * SpeedModifier);
    }
    if (glfwGetKey(Window, GLFW_KEY_C))
    {
        CAMERA_SYSTEM()->MoveCameraUpward(Camera, Time * CameraMovementSpeed * SpeedModifier);
    }
    if (glfwGetKey(Window, GLFW_KEY_Q))
    {
        CAMERA_SYSTEM()->Roll(Camera, -Time * CameraRotationSpeed);
    }

    if (glfwGetKey(Window, GLFW_KEY_E))
    {
        CAMERA_SYSTEM()->Roll(Camera, Time * CameraRotationSpeed);
    }

    {
        double XDelta = XCurrent - XPrevious;
        double YDelta = YCurrent - YPrevious;
        XPrevious = XCurrent;
        YPrevious = YCurrent;

        /// GLFW doesn't work well with switching cursor between "disabled" and "normal"
        /// This bool and check protect from camera sudden moves
        if (FirstUpdateSinceRMB)
        {
            FirstUpdateSinceRMB = false;
        }
        else
        {
            static double Sensitivity = 0.001;
            CAMERA_SYSTEM()->LookRight(Camera, float(XDelta * Sensitivity));
            CAMERA_SYSTEM()->LookUp(Camera, float(YDelta * Sensitivity));
        }
    }
}
