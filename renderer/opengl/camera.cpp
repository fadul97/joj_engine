#include "camera.h"

#include <iostream>
using namespace std;

JojRenderer::Camera::Camera(DirectX::XMFLOAT3 pos, DirectX::XMFLOAT3 up, f32 yaw, f32 pitch)
    : front(DirectX::XMFLOAT3{ 0.0f, 0.0, -1.0f }), movement_speed(SPEED), mouse_sensitivity(SENSITIVITY), zoom(ZOOM)
{
    position = pos;
    world_up = up;
    this->yaw = yaw;
    this->pitch = pitch;
    update_camera_vectors();
}

JojRenderer::Camera::Camera(f32 posX, f32 posY, f32 posZ, f32 upX, f32 upY, f32 upZ, f32 yaw, f32 pitch)
    : front(DirectX::XMFLOAT3{ 0.0f, 0.0, -1.0f }), movement_speed(SPEED), mouse_sensitivity(SENSITIVITY), zoom(ZOOM)
{
    position = DirectX::XMFLOAT3{ posX, posY, posZ };
    world_up = DirectX::XMFLOAT3{ upX, upY, upZ };
    this->yaw = yaw;
    this->pitch = pitch;
    update_camera_vectors();
}

JojRenderer::Camera::~Camera()
{
}

void JojRenderer::Camera::process_keyboard(CameraMovement direction, f32 delta_time)
{
    f32 velocity = movement_speed * delta_time;
    DirectX::XMVECTOR p;
    DirectX::XMVECTOR f;
    DirectX::XMVECTOR r;
    if (direction == CameraMovement::FORWARD)
    {
        // Convert XMFLOAT3 to XMVECTOR
        p = DirectX::XMLoadFloat3(&position);
        f = DirectX::XMLoadFloat3(&front);

        // Perform scalar multiplication and vector addition
        r = DirectX::XMVectorAdd(p, DirectX::XMVectorScale(f, velocity));
        
        // Store the result back in the XMFLOAT3
        DirectX::XMStoreFloat3(&position, r);

        cout << "Position: " << position.x << position.y << position.z << endl;

    }
    if (direction == CameraMovement::BACKWARD)
    {
        // Convert XMFLOAT3 to XMVECTOR
        p = DirectX::XMLoadFloat3(&position);
        f = DirectX::XMLoadFloat3(&front);

        // Perform scalar multiplication and vector subtraction
        r = DirectX::XMVectorSubtract(p, DirectX::XMVectorScale(f, velocity));

        // Store the result back in the XMFLOAT3
        DirectX::XMStoreFloat3(&position, r);

        cout << "Position: " << position.x << position.y << position.z << endl;

    }
    if (direction == CameraMovement::LEFT)
    {
        // Convert XMFLOAT3 to XMVECTOR
        p = DirectX::XMLoadFloat3(&position);
        r = DirectX::XMLoadFloat3(&right);

        // Perform scalar multiplication and vector addition
        r = DirectX::XMVectorAdd(p, DirectX::XMVectorScale(r, velocity));

        // Store the result back in the XMFLOAT3
        DirectX::XMStoreFloat3(&position, r);

        cout << "Position: " << position.x << " " << position.y << " " << position.z << endl;
    }
    if (direction == CameraMovement::RIGHT)
    {
        // Convert XMFLOAT3 to XMVECTOR
        p = DirectX::XMLoadFloat3(&position);
        r = DirectX::XMLoadFloat3(&right);

        // Perform scalar multiplication and vector subtraction
        r = DirectX::XMVectorSubtract(p, DirectX::XMVectorScale(r, velocity));

        // Store the result back in the XMFLOAT3
        DirectX::XMStoreFloat3(&position, r);

        cout << "Position: " << position.x << position.y << position.z << endl;

    }
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
    // TODO: Check why f.x needs to be negated, X-axis movement works correctly
    // Create f vector
    DirectX::XMFLOAT3 f = DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f };
    f.x = cos(DirectX::XMConvertToRadians(yaw)) * cos(DirectX::XMConvertToRadians(pitch)) * -1;
    f.y = sin(DirectX::XMConvertToRadians(pitch));
    f.z = sin(DirectX::XMConvertToRadians(yaw)) * cos(DirectX::XMConvertToRadians(pitch));

    // Normalize fv vector
    DirectX::XMVECTOR fv = DirectX::XMLoadFloat3(&f);
    fv = DirectX::XMVector3Normalize(fv);

    // Store result in front member
    DirectX::XMStoreFloat3(&front, fv);

    // Calculate right and up vectors
    DirectX::XMVECTOR wupv = DirectX::XMLoadFloat3(&world_up);
    DirectX::XMVECTOR rv = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(fv, wupv));
    DirectX::XMVECTOR uv = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(rv, fv));

    // Store the results back in the XMFLOAT3 members
    DirectX::XMStoreFloat3(&right, rv);
    DirectX::XMStoreFloat3(&up, uv);
}