#include "camera.h"

JojRenderer::Camera::Camera(Vec3 pos, Vec3 up, f32 yaw, f32 pitch)
    : front(vec3_create(0.0f, 0.0, -1.0f)), movement_speed(SPEED), mouse_sensitivity(SENSITIVITY), zoom(ZOOM)
{
    position = pos;
    world_up = up;
    this->yaw = yaw;
    this->pitch = pitch;

    update_camera_vectors();
}

JojRenderer::Camera::Camera(f32 posX, f32 posY, f32 posZ, f32 upX, f32 upY, f32 upZ, f32 yaw, f32 pitch)
    : front(vec3_create(0.0f, 0.0, -1.0f)), movement_speed(SPEED), mouse_sensitivity(SENSITIVITY), zoom(ZOOM)
{
}

JojRenderer::Camera::~Camera()
{
}

void JojRenderer::Camera::process_keyboard(CameraMovement direction, f32 delta_time)
{
    f32 velocity = movement_speed * delta_time;
    if (direction == CameraMovement::FORWARD)
        vec3_add(position, vec3_multiply_by_scalar(front, velocity));
    if (direction == CameraMovement::BACKWARD)
        vec3_minus(position, vec3_multiply_by_scalar(front, velocity));
    if (direction == CameraMovement::LEFT)
        vec3_minus(position, vec3_multiply_by_scalar(right, velocity));
    if (direction == CameraMovement::RIGHT)
        vec3_add(position, vec3_multiply_by_scalar(right, velocity));
}

void JojRenderer::Camera::process_mouse_movement(f32 xoffset, f32 yoffset, b8 constrain_pitch)
{
    xoffset *= mouse_sensitivity;
    yoffset *= mouse_sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrain_pitch)
    {
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    update_camera_vectors();
}

void JojRenderer::Camera::process_mouse_scroll(f32 yoffset)
{
    zoom -= yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

void JojRenderer::Camera::update_camera_vectors()
{
    Vec3 front = vec3_create(1.0f, 1.0f, 1.0f);
    front.x = cos(to_radians(yaw)) * cos(to_radians(pitch));
    front.y = sin(to_radians(pitch));
    front.z = sin(to_radians(yaw)) * cos(to_radians(pitch));
    front = vec3_normalize(front);

    right = vec3_normalize(vec3_cross_product(front, world_up));
    up = vec3_normalize(vec3_cross_product(right, front));
}