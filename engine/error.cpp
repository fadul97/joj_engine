#include "error.h"

#include <comdef.h>
#include <sstream>

JojEngine::Error::Error()
    : hres_code(S_OK), line_num(-1)
{
    // Empty function and file names
}

JojEngine::Error::Error(HRESULT hr, const std::string& func, const std::string& file, int line)
    : hres_code(hr), func_name(func), line_num(line)
{
    // Position of the last slash in the file name
    size_t pos = file.find_last_of('\\');

    // If bar was found
    if (pos != std::string::npos)
    {
        // Only saves the file name and not the full path
        file_name = file.substr(pos + 1);
    }
}

JojEngine::Error::~Error()
{
}

std::string JojEngine::Error::to_string() const
{
    // Get the description of the COM error code in string format
    _com_error err(hres_code);

    std::stringstream text;
    text << func_name
        << " failed in " << file_name
        << ", line " << std::to_string(line_num)
        << ":\n" << err.ErrorMessage();

    return text.str();
}