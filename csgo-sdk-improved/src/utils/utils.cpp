#include "utils.hpp"

// Defined in 'api/library/unload.cpp'.
void SDK_UnloadSelf();

void Utils::Unload()
{
    ::SDK_UnloadSelf();
}
