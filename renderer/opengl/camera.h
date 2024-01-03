#pragma once

#include "defines.h"

#if PLATFORM_WINDOWS

#include "fmath.h"

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
    const f32 SPEED = 2.5f;
    const f32 SENSITIVITY = 0.1f;
    const f32 ZOOM = 45.0f;

    class Camera
    {
    public:
        Camera(Vec3 pos = vec3_create(0.0f, 0.0f, 0.0f), Vec3 up = vec3_create(0.0f, 1.0f, 0.0f), f32 yaw = YAW, f32 pitch = PITCH);
        Camera(f32 posX, f32 posY, f32 posZ, f32 upX, f32 upY, f32 upZ, f32 yaw, f32 pitch);
        ~Camera();

        Mat4 get_view_mat() const;

        void process_keyboard(CameraMovement direction, f32 delta_time);
        void process_mouse_movement(f32 xoffset, f32 yoffset, b8 constrain_pitch = true);
        void process_mouse_scroll(f32 yoffset);
    private:
        Vec3 position;
        Vec3 front;
        Vec3 up;
        Vec3 right;
        Vec3 world_up;

        f32 yaw;
        f32 pitch;

        f32 movement_speed;
        f32 mouse_sensitivity;
        f32 zoom;
    
        void update_camera_vectors();
    };

    inline Mat4 Camera::get_view_mat() const
    { return look_at(position, vec3_add(position, front), up); }
}

#endif // PLATFORM_WINDOWS