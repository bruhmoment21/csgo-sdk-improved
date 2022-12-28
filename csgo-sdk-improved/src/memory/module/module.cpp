#ifdef __linux__
#include <dlfcn.h>
#include <link.h>
#else
#include <Windows.h>
#include <Psapi.h>
#endif

#include "module.hpp"

#include "../../logger/logger.hpp"
#include "../../sdk/classes/tier1/interface.hpp"

CModule::CModule(const char *moduleName, bool bParseModule)
{
    // 'moduleName' better not point to something like a local variable because
    // 'this->m_moduleName' will become a dangling pointer.

    SDK_TRACE("Called CModule::CModule('{}', {}).", moduleName, bParseModule);

    this->m_moduleName = moduleName;
    this->m_handle = nullptr;
    this->m_start = this->m_end = 0;

    if (bParseModule)
    {
        this->Parse();
    }
}

CModule &CModule::operator=(CModule &&rhs) noexcept
{
    if (this != &rhs)
    {
        if (this->m_handle)
        {
            this->ReleaseHandle();
        }

        this->m_moduleName = rhs.m_moduleName;
        this->m_handle = rhs.m_handle;
        this->m_start = rhs.m_start;
        this->m_end = rhs.m_end;

        rhs.m_moduleName = nullptr;
        rhs.m_handle = nullptr;
        rhs.m_start = rhs.m_end = 0;
    }

    return *this;
}

CModule::~CModule()
{
    if (this->m_handle)
    {
        SDK_TRACE("Called ~CModule::CModule() on module '{}', m_handle: {}.",
                  this->m_moduleName ? this->m_moduleName : "", fmt::ptr(this->m_handle));

        this->ReleaseHandle();
    }
}

bool CModule::Parse()
{
    //
    // Initializes: m_handle, m_start, m_end.
    //

    if (this->m_start)
    {
        SDK_WARN("Tried parsing '{}' which is already parsed.", this->m_moduleName);
        return true;
    }

#ifdef __linux__
    this->m_handle = dlopen(this->m_moduleName, RTLD_LAZY | RTLD_NOLOAD);
#else
    this->m_handle = static_cast<void *>(GetModuleHandle(this->m_moduleName));
#endif

    SDK_ASSERT(this->m_handle != NULL, "Module handle not found!");
    if (this->m_handle == NULL)
    {
        SDK_CRITICAL("Module '{}' handle couldn't be retrieved.", this->m_moduleName);
        return false;
    }

#ifdef __linux__
    dl_iterate_phdr(
        [](struct dl_phdr_info *info, size_t, void *data) {
            CModule *pCurrentModule = static_cast<CModule *>(data);
            if (strstr(info->dlpi_name, pCurrentModule->GetName()) != 0)
            {
                uintptr_t start = static_cast<uintptr_t>(info->dlpi_addr + info->dlpi_phdr[0].p_vaddr);
                uintptr_t end = static_cast<uintptr_t>(info->dlpi_phdr[0].p_memsz);

                pCurrentModule->SetModuleBounds(start, end);

                return 1;
            }

            return 0;
        },
        static_cast<void *>(this));
#else
    MODULEINFO mi;
    BOOL status = GetModuleInformation(GetCurrentProcess(), static_cast<HMODULE>(this->m_handle), &mi, sizeof(mi));
    SDK_ASSERT(status != 0, "GetModuleInformation() failed!");
    if (status == 0)
    {
        SDK_CRITICAL("GetModuleInformation() failed. GetLastError() returned {}.", GetLastError());
        return false;
    }

    this->m_start = reinterpret_cast<uintptr_t>(this->m_handle);
    this->m_end = static_cast<uintptr_t>(mi.SizeOfImage);
#endif

    SDK_INFO("Parsed '{}': m_handle: {}, m_start: {:x}, m_end: {:x}.", this->m_moduleName, fmt::ptr(this->m_handle),
             this->m_start, this->m_end);

    return true;
}

bool CModule::IsLoaded()
{
    return this->m_handle != nullptr;
}

void *CModule::GetProcAddress(const char *lpProcName)
{
    void *rv = nullptr;

#ifdef __linux__
    rv = ::dlsym(this->m_handle, lpProcName);
#else
    rv = static_cast<void *>(::GetProcAddress(static_cast<HMODULE>(this->m_handle), lpProcName));
#endif

    return rv;
}

void CModule::ReleaseHandle()
{
#ifdef __linux__
    dlclose(this->m_handle);
#endif

    SDK_INFO("Released '{}' handle!", fmt::ptr(this->m_handle));
}

uintptr_t CModule::FindPatternEx(int *sigBytes, size_t sigSize)
{
    uintptr_t rv = 0;
    if (!this->IsLoaded())
        return rv;

    uint8_t *pBytes = reinterpret_cast<uint8_t *>(this->m_start);
    for (size_t i = 0; i < this->m_end - sigSize; ++i)
    {
        bool found = true;
        for (size_t j = 0; j < sigSize; ++j)
        {
            if (pBytes[i + j] != sigBytes[j] && sigBytes[j] != -1)
            {
                found = false;
                break;
            }
        }

        if (found)
        {
            rv = reinterpret_cast<uintptr_t>(&pBytes[i]);
            break;
        }
    }

    return rv;
}

uintptr_t CModule::FindInterfaceEx(const char *szInterfaceName, const char *szFileName, int line)
{
    uintptr_t rv = 0;

    for (InterfaceReg *pRegList = this->GetRegisterLinkedList(szFileName, line); pRegList; pRegList = pRegList->m_pNext)
    {
        if (strcmp(szInterfaceName, pRegList->m_pName) == 0)
        {
            rv = reinterpret_cast<uintptr_t>(pRegList->m_CreateFn());
            break;
        }
    }

    if (rv == 0)
    {
        SDK_WARN("'{}' not found in '{}'!", szInterfaceName, this->m_moduleName);
    }
    else
    {
        SDK_INFO("'{}' found in '{}' at {:x}.", szInterfaceName, this->m_moduleName, rv);
    }

    return rv;
}

InterfaceReg *CModule::GetRegisterLinkedList(const char *szFileName, int line)
{
#ifdef __linux__
    void *s_pInterfaceRegs = this->GetProcAddress("s_pInterfaceRegs");
    if (!s_pInterfaceRegs)
        return nullptr;

    SDK_Pointer rv{s_pInterfaceRegs};
    rv.Deref(1);

    return rv.GetAs<InterfaceReg *>(szFileName, line);
#else
    void *pCreateInterface = this->GetProcAddress("CreateInterface");
    if (!pCreateInterface)
        return nullptr;

    SDK_Pointer rv{pCreateInterface};
    rv.ToAbs(5, 6).Deref(2);

    return rv.GetAs<InterfaceReg *>(szFileName, line);
#endif
}

uintptr_t SDK_Pointer::Get(const char *szFileName, int line)
{
    if (line != 0)
    {
        if (this->m_ptr == 0)
        {
            SDK_WARN("'{}:{}' GetAs() returned {:x}.", szFileName, line, this->m_ptr);
        }
        else
        {
            SDK_INFO("'{}:{}' GetAs() returned {:x}.", szFileName, line, this->m_ptr);
        }
    }

    return this->m_ptr;
}
