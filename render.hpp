#include <array>
#include <immintrin.h>
#include <D3DX11tex.h>
#include <wrl/client.h>
#include <vector>
#include "overlay.hpp"
#include "menu.hpp"

#define LIKELY(x) (x)
#define UNLIKELY(x) (x)

using Microsoft::WRL::ComPtr;

// DirectX Core Resources
ComPtr<ID3D11Device> d3d_device;
ComPtr<ID3D11DeviceContext> d3d_device_ctx;
ComPtr<IDXGISwapChain> d3d_swap_chain;
ComPtr<ID3D11RenderTargetView> d3d_render_target;
D3DPRESENT_PARAMETERS d3d_present_params;

// Legacy DirectX Resources
ComPtr<IDirect3DTexture9> bgs, foto, merfthefurry, ghoul;

// Drawing Resources
ImDrawList* drawlist;
ImDrawList* tabdrawlist;

enum RENDER_INFORMATION : int {
    RENDER_HIJACK_FAILED = 0,
    RENDER_IMGUI_FAILED = 1,
    RENDER_DRAW_FAILED = 2,
    RENDER_SETUP_SUCCESSFUL = 3
};

struct DiscordOverlay {
    HANDLE fileMapping = nullptr;
    struct Header {
        UINT Magic;
        UINT FrameCount;
        UINT Reserved;
        UINT Width;
        UINT Height;
        alignas(16) BYTE Buffer[1];
    }*mappedHeader = nullptr;
    DWORD targetProcessId = 0;
};

namespace render {
    class c_render {
    private:
        OverlayCord::Communication::ConnectedProcessInfo g_Discord;
        ComPtr<ID3D11Texture2D> g_RenderTexture;
        ComPtr<ID3D11ShaderResourceView> g_RenderView;

    public:
        auto Render() -> bool {
//your render here
        }

        auto ImGui() -> bool {
            DXGI_SWAP_CHAIN_DESC swap_chain_description = {};
            swap_chain_description.BufferCount = 2;
            swap_chain_description.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
            swap_chain_description.BufferDesc.RefreshRate.Numerator = 60;
            swap_chain_description.BufferDesc.RefreshRate.Denominator = 1;
            swap_chain_description.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
            swap_chain_description.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
            swap_chain_description.OutputWindow = window_handle;
            swap_chain_description.SampleDesc.Count = 1;
            swap_chain_description.Windowed = TRUE;
            swap_chain_description.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

            D3D_FEATURE_LEVEL d3d_feature_lvl;
            const D3D_FEATURE_LEVEL d3d_feature_array[1] = { D3D_FEATURE_LEVEL_11_0 };

            HRESULT hr = D3D11CreateDeviceAndSwapChain(
                nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0, d3d_feature_array, 1,
                D3D11_SDK_VERSION, &swap_chain_description, d3d_swap_chain.GetAddressOf(),
                d3d_device.GetAddressOf(), &d3d_feature_lvl, d3d_device_ctx.GetAddressOf());

            if (FAILED(hr)) return false;

            ComPtr<ID3D11Texture2D> pBackBuffer;
            hr = d3d_swap_chain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
            if (FAILED(hr)) return false;

            hr = d3d_device->CreateRenderTargetView(pBackBuffer.Get(), NULL, d3d_render_target.GetAddressOf());
            if (FAILED(hr)) return false;

            ImGui::CreateContext();
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = NULL;

            D3DX11_IMAGE_LOAD_INFO info;
            ID3DX11ThreadPump* pump{ nullptr };



            D3DX11CreateShaderResourceViewFromMemory(d3d_device.Get(), gingerbread, sizeof(gingerbread), &info, pump, &normal, 0);
            D3DX11CreateShaderResourceViewFromMemory(d3d_device.Get(), gingerskel, sizeof(gingerskel), &info, pump, &skeleton, 0);
            D3DX11CreateShaderResourceViewFromMemory(d3d_device.Get(), gingerhead, sizeof(gingerhead), &info, pump, &head, 0);
            D3DX11CreateShaderResourceViewFromMemory(d3d_device.Get(), headskels, sizeof(headskels), &info, pump, &headskel, 0);
            D3DX11CreateShaderResourceViewFromMemory(d3d_device.Get(), circles, sizeof(circles), &info, pump, &circle, 0);

            IconFont = io.Fonts->AddFontFromMemoryTTF(Icons, sizeof(Icons), 23, NULL, io.Fonts->GetGlyphRangesCyrillic());
            IconFontLogs = io.Fonts->AddFontFromMemoryTTF(IconFontLog, sizeof(IconFontLog), 25, NULL, io.Fonts->GetGlyphRangesCyrillic());
            InterMedium = io.Fonts->AddFontFromMemoryTTF(LexendDeca, sizeof(LexendDeca), 17, NULL, io.Fonts->GetGlyphRangesCyrillic());
            TestFont = io.Fonts->AddFontFromMemoryTTF(Intermedium, sizeof(Intermedium), 40, NULL, io.Fonts->GetGlyphRangesCyrillic());
            InterMediumone = io.Fonts->AddFontFromMemoryTTF(Intermedium, sizeof(Intermedium), 14, NULL, io.Fonts->GetGlyphRangesCyrillic());


            ImGui_ImplWin32_Init(window_handle);
            ImGui_ImplDX11_Init(d3d_device.Get(), d3d_device_ctx.Get());

            return true;
        }

        auto hijack() -> bool {
            window_handle = FindWindowA_Spoofed("UnrealWindow", nullptr);
            if (!window_handle) return false;

            DWORD processId;
            GetWindowThreadProcessId(window_handle, &processId);

            g_Discord = { 0 };
            g_Discord.ProcessId = processId;

            if (!OverlayCord::Communication::ConnectToProcess(g_Discord)) {
                return false;
            }

            return true;
        }

        auto Menu() -> void {
// implement your menu here
        }

        auto Draw() -> void {
         //implement your drawing here in this function

            ImGui_ImplDX11_NewFrame();
            ImGui_ImplWin32_NewFrame();
            ImGui::NewFrame();

            d3d_device_ctx->OMSetRenderTargets(1, d3d_render_target.GetAddressOf(), nullptr);
            d3d_device_ctx->ClearRenderTargetView(d3d_render_target.Get(), clearColor);


            Menu();


            ImGui::Render();
            ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        
            static int discordOverlayCounter = 0;
            if (g_Discord.MappedAddress != nullptr && ++discordOverlayCounter >= 1) { 
                discordOverlayCounter = 0;
                ProcessDiscordOverlay();
            }

            d3d_swap_chain->Present(1, 0);
        }

    private:


        void ProcessDiscordOverlay() {
            ComPtr<ID3D11Texture2D> backBuffer;
            if (FAILED(d3d_swap_chain->GetBuffer(0, IID_PPV_ARGS(&backBuffer)))) {
                return;
            }

            D3D11_TEXTURE2D_DESC desc;
            backBuffer->GetDesc(&desc);

            ComPtr<ID3D11Texture2D> stagingTex;
            desc.Usage = D3D11_USAGE_STAGING;
            desc.BindFlags = 0;
            desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;

            if (SUCCEEDED(d3d_device->CreateTexture2D(&desc, nullptr, &stagingTex))) {
                d3d_device_ctx->CopyResource(stagingTex.Get(), backBuffer.Get());

                D3D11_MAPPED_SUBRESOURCE mapped = { 0 };
                if (SUCCEEDED(d3d_device_ctx->Map(stagingTex.Get(), 0, D3D11_MAP_READ, 0, &mapped))) {
                    ProcessDiscordPixels(mapped, desc);
                    d3d_device_ctx->Unmap(stagingTex.Get(), 0);
                }
            }
        }

        void ProcessDiscordPixels(const D3D11_MAPPED_SUBRESOURCE& mapped, const D3D11_TEXTURE2D_DESC& desc) {
            const SIZE_T bufferSize = min(mapped.RowPitch * desc.Height,
                g_Discord.MappedAddress->Width * g_Discord.MappedAddress->Height * 4);

            g_Discord.MappedAddress->Width = desc.Width;
            g_Discord.MappedAddress->Height = desc.Height;

            const BYTE* srcPixels = static_cast<const BYTE*>(mapped.pData);
            BYTE* dstPixels = static_cast<BYTE*>(g_Discord.MappedAddress->Buffer);

            const __m128i shuffleMask = _mm_set_epi8(15, 12, 13, 14, 11, 8, 9, 10, 7, 4, 5, 6, 3, 0, 1, 2);

            for (UINT row = 0; row < desc.Height; ++row) {
                for (UINT col = 0; col < desc.Width; col += 4) {
                    const UINT srcIdx = row * mapped.RowPitch + col * 4;
                    const UINT dstIdx = row * (desc.Width * 4) + col * 4;

                    __m128i src = _mm_loadu_si128(reinterpret_cast<const __m128i*>(&srcPixels[srcIdx]));
                    __m128i shuffled = _mm_shuffle_epi8(src, shuffleMask);
                    _mm_storeu_si128(reinterpret_cast<__m128i*>(&dstPixels[dstIdx]), shuffled);
                }
            }

            g_Discord.MappedAddress->FrameCount++;
        }
    };
}

static render::c_render* Render = new render::c_render();
