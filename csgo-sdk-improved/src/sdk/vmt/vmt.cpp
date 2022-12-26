#include "vmt.hpp"

#include "../../logger/logger.hpp"

void *vmt::GetVirtual(void *pClass, int index, const char *szFilePath, int line)
{
    if (!pClass)
    {
        if (szFilePath && line)
        {
            SDK_TRACE("Called vmt::GetVirtualEx({}, {}, '{}', {}).", fmt::ptr(pClass), index, szFilePath, line);
        }

        SDK_WARN("Tried vmt::GetVirtual() on NULL pClass.");
        return nullptr;
    }

    void **vTable = *reinterpret_cast<decltype(vTable) *>(pClass);
    if (!vTable)
    {
        if (szFilePath && line)
        {
            SDK_TRACE("Called vmt::GetVirtualEx({}, {}, '{}', {}).", fmt::ptr(pClass), index, szFilePath, line);
        }

        SDK_WARN("Tried vmt::GetVirtual() on NULL vTable.");
        return nullptr;
    }

    return vTable[index];
}
