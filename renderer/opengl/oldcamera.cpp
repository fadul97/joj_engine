#include "oldcamera.h"

JojRendererOld::Camera::Camera(Vec3 pos, Vec3 up, f32 yaw, f32 pitch)
    : front(vec3_create(0.0f, 0.0, -1.0f)), movement_speed(SPEED), mouse_sensitivity(SENSITIVITY), zoom(ZOOM)
{
    position = pos;
    world_up = up;
    this->yaw = yaw;
    this->pitch = pitch;

    update_camera_vectors();
}

JojRendererOld::Camera::Camera(f32 posX, f32 posY, f32 posZ, f32 upX, f32 upY, f32 upZ, f32 yaw, f32 pitch)
    : front(vec3_create(0.0f, 0.0, -1.0f)), movement_speed(SPEED), mouse_sensitivity(SENSITIVITY), zoom(ZOOM)
{
}

JojRendererOld::Camera::~Camera()
{
}

void JojRendererOld::Camera::process_keyboard(CameraMovement direction, f32 delta_time)
{
    f32 velocity = movement_speed * delta_time;
    if (direction == CameraMovement::FORWARD)
    {
        /*
        printf("\n\n========== Position ==========\n");
        vec3_print(position);

        printf("\n\n========== Front ==========\n");
        vec3_print(front);

        printf("\n\n========== Front * velocity ==========\n");
        Vec3 r = vec3_multiply_by_scalar(front, velocity);
        vec3_print(r);

        printf("\n\n========== INLINE Front * velocity ==========\n");
        vec3_print(vec3_multiply_by_scalar(front, velocity));

        printf("\n\n========== Position + Resultado ==========\n");
        vec3_print(vec3_add(position, vec3_multiply_by_scalar(front, velocity)));


        printf("\n\n========== FINALLY Position ==========\n");
        vec3_print(position);
        */
        position = vec3_add(position, vec3_multiply_by_scalar(front, velocity));
    }
    if (direction == CameraMovement::BACKWARD)
        position = vec3_minus(position, vec3_multiply_by_scalar(front, velocity));
    if (direction == CameraMovement::LEFT)
        position = vec3_minus(position, vec3_multiply_by_scalar(right, velocity));
    if (direction == CameraMovement::RIGHT)
        position = vec3_add(position, vec3_multiply_by_scalar(right, velocity));

    printf("\n\n========== Position ==========\n");
    vec3_print(position);
}

void JojRendererOld::Camera::process_mouse_movement(f32 xoffset, f32 yoffset, b8 constrain_pitch)
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

void JojRendererOld::Camera::process_mouse_scroll(f32 yoffset)
{
    zoom -= yoffset;
    if (zoom < 1.0f)
        zoom = 1.0f;
    if (zoom > 45.0f)
        zoom = 45.0f;
}

void JojRendererOld::Camera::update_camera_vectors()
{
    Vec3 front = vec3_create(1.0f, 1.0f, 1.0f);
    front.x = cos(to_radians(yaw)) * cos(to_radians(pitch));
    front.y = sin(to_radians(pitch));
    front.z = sin(to_radians(yaw)) * cos(to_radians(pitch));
    front = vec3_normalize(front);

    right = vec3_normalize(vec3_cross_product(front, world_up));
    up = vec3_normalize(vec3_cross_product(right, front));
}