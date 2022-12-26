#include <Windows.h>
#include <d3d9.h>

#include "../../chook.hpp"

#include "../../../logger/logger.hpp"
#include "../../../sdk/vmt/vmt.hpp"
#include "../../../menu/menu.hpp"

#include <imgui/imgui.h>
#include <imgui/imgui_impl_dx9.h>
#include <imgui/imgui_impl_win32.h>

static LPDIRECT3D9 g_pD3D = NULL;
static LPDIRECT3DDEVICE9 g_pd3dDevice = NULL;

static bool CreateDeviceD3D9(HWND hWnd);
static void CleanupDeviceD3D9();
static void RenderImGui_DX9(IDirect3DDevice9 *pDevice);

static CHook<HRESULT WINAPI(IDirect3DDevice9 *, D3DPRESENT_PARAMETERS *)> g_reset;
static HRESULT WINAPI hkReset(IDirect3DDevice9 *pDevice, D3DPRESENT_PARAMETERS *pPresentationParameters)
{
    ImGui_ImplDX9_InvalidateDeviceObjects();

    return g_reset.m_pOriginalFn(pDevice, pPresentationParameters);
}

static CHook<HRESULT WINAPI(IDirect3DDevice9 *, const RECT *, const RECT *, HWND, const RGNDATA *)> g_present;
static HRESULT WINAPI hkPresent(IDirect3DDevice9 *pDevice, const RECT *pSourceRect, const RECT *pDestRect,
                                HWND hDestWindowOverride, const RGNDATA *pDirtyRegion)
{
    RenderImGui_DX9(pDevice);

    return g_present.m_pOriginalFn(pDevice, pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

// [THIS]: Called in 'hooks.cpp'.
void SDK_HookGraphicsAPI()
{
    if (!CreateDeviceD3D9(GetConsoleWindow()))
    {
        SDK_ERROR("CreateDeviceD3D9() failed.");
        return;
    }

    SDK_INFO("'{}:{}': g_pD3D: {}", FILE_AND_LINE, fmt::ptr(g_pD3D));
    SDK_INFO("'{}:{}': g_pd3dDevice: {}", FILE_AND_LINE, fmt::ptr(g_pd3dDevice));

    void *pResentFn = vmt::GetVirtual(g_pd3dDevice, 16);
    void *pPresentFn = vmt::GetVirtual(g_pd3dDevice, 17);

    CleanupDeviceD3D9();

    g_reset.Hook(pResentFn, hkReset, FILE_AND_LINE);
    g_present.Hook(pPresentFn, hkPresent, FILE_AND_LINE);
}

// [THIS]: Called in 'hooks.cpp'.
void SDK_UnhookGraphicsAPI()
{
    if (!ImGui::GetCurrentContext())
        return;

    ImGuiIO &io = ImGui::GetIO();
    if (io.BackendRendererUserData)
        ImGui_ImplDX9_Shutdown();

    if (io.BackendPlatformUserData)
        ImGui_ImplWin32_Shutdown();

    ImGui::DestroyContext();
}

static bool CreateDeviceD3D9(HWND hWnd)
{
    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (g_pD3D == NULL)
    {
        SDK_ERROR("Direct3DCreate9() failed.");
        return false;
    }

    D3DPRESENT_PARAMETERS d3dpp = {};
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;

    HRESULT hr = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_NULLREF, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
                                      &d3dpp, &g_pd3dDevice);
    if (hr != D3D_OK)
    {
        SDK_ERROR("CreateDevice() failed. [rv: %lu]", hr);
        return false;
    }

    return true;
}

static void CleanupDeviceD3D9()
{
    if (g_pD3D)
    {
        g_pD3D->Release();
        g_pD3D = NULL;
    }

    if (g_pd3dDevice)
    {
        g_pd3dDevice->Release();
        g_pd3dDevice = NULL;
    }
}

static void RenderImGui_DX9(IDirect3DDevice9 *pDevice)
{
    if (!ImGui::GetCurrentContext())
        return;

    if (!ImGui::GetIO().BackendRendererUserData)
    {
        ImGui_ImplDX9_Init(pDevice);

        Menu::Initialize();

        // If this is called 2 times it should be a problem.
        SDK_TRACE("Called ImGui_ImplDX9_Init().");
    }

    if (ImGui::GetCurrentContext())
    {
        DWORD SRGBWriteEnable;
        pDevice->GetRenderState(D3DRS_SRGBWRITEENABLE, &SRGBWriteEnable);
        pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, false);

        ImGui_ImplDX9_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        Menu::Render();

        ImGui::EndFrame();
        if (pDevice->BeginScene() == D3D_OK)
        {
            ImGui::Render();
            ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
            pDevice->EndScene();
        }

        pDevice->SetRenderState(D3DRS_SRGBWRITEENABLE, SRGBWriteEnable);
    }
}
