#include "triangle_d3d11.h"

void D3D11Triangle::init()
{
}

void D3D11Triangle::update()
{
    // Exit with ESCAPE key
    if (input->is_key_press(VK_ESCAPE))
        window->close();
}

void D3D11Triangle::draw()
{
    
}

void D3D11Triangle::shutdown()
{

}