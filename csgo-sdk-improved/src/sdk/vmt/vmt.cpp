#include "vmt.hpp"

#include "../../logger/logger.hpp"

void *vmt::GetVirtual(void *pClass, int index, const char *szFilePath, int line)
{
    SDK_ASSERT(pClass, "pClass can't be NULL.");
    if (!pClass)
    {
        if (szFilePath && line)
        {
            SDK_TRACE("Called vmt::GetVirtual({}, {}, '{}', {}).", fmt::ptr(pClass), index, szFilePath, line);
        }

        SDK_WARN("Tried vmt::GetVirtual() on NULL pClass.");
        return nullptr;
    }

    void **vTable = *reinterpret_cast<decltype(vTable) *>(pClass);

    SDK_ASSERT(vTable, "vTable can't be NULL.");
    if (!vTable)
    {
        if (szFilePath && line)
        {
            SDK_TRACE("Called vmt::GetVirtual({}, {}, '{}', {}).", fmt::ptr(pClass), index, szFilePath, line);
        }

        SDK_WARN("Tried vmt::GetVirtual() on NULL vTable.");
        return nullptr;
    }

    return vTable[index];
}
