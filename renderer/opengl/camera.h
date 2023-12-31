#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include <DirectXMath.h>

namespace JojRenderer
{
    enum class CameraMovement
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };

    const f32 YAW = -90.0f;
    const f32 PITCH = 0.0f;
    const f32 SPEED = 10.0f;
    const f32 SENSITIVITY = 0.1f;
    const f32 ZOOM = 45.0f;

    class Camera
    {
    public:
        Camera(DirectX::XMFLOAT3 pos = DirectX::XMFLOAT3{ 0.0f, 0.0f, 0.0f }, DirectX::XMFLOAT3 up = DirectX::XMFLOAT3{ 0.0f, 1.0f, 0.0f }, f32 yaw = YAW, f32 pitch = PITCH);
        Camera(f32 posX, f32 posY, f32 posZ, f32 upX, f32 upY, f32 upZ, f32 yaw, f32 pitch);
        ~Camera();

        DirectX::XMMATRIX get_view_mat() const;

        void process_keyboard(CameraMovement direction, f32 delta_time);
        void process_mouse_movement(f32 xoffset, f32 yoffset, b8 constrain_pitch = true);
        void process_mouse_scroll(f32 yoffset);

        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 front;
        DirectX::XMFLOAT3 up;
        DirectX::XMFLOAT3 right;
        DirectX::XMFLOAT3 world_up;

        f32 yaw;
        f32 pitch;

        f32 movement_speed;
        f32 mouse_sensitivity;
        f32 zoom;
    private:

        void update_camera_vectors();
    };

    inline DirectX::XMMATRIX Camera::get_view_mat() const
    {
        DirectX::XMFLOAT3 r = DirectX::XMFLOAT3{ position.x + front.x, position.y + front.y, position.z + front.z };
        return DirectX::XMMatrixLookAtLH(
            DirectX::XMLoadFloat3(&position),
            DirectX::XMLoadFloat3(&r),
            DirectX::XMLoadFloat3(&up)
        );
    }
}

#endif // PLATFORM_WINDOWS