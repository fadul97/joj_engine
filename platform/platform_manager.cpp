#include "platform_manager.h"

JojPlatform::PlatformManager::PlatformManager()
{
    window = std::make_unique<Window>();

    // ATTENTION: input must be initialized after window creation
    input = std::make_unique<Input>();

    timer = std::make_unique<Timer>();
}

JojPlatform::PlatformManager::~PlatformManager()
{
}

b8 JojPlatform::PlatformManager::init(i32 width, i32 height, std::string title)
{
    // Setup window
    window->set_mode(JojPlatform::WindowMode::WINDOWED);
    window->set_size(width, height);
    window->set_color(60, 60, 60);
    window->set_title(title);

    return true;
}