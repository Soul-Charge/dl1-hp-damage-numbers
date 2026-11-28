#include "d3d11_renderer.h"
#include "config.h"
#include "hook.h"
#include "game_hooks.h"

#include <d3dcompiler.h>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <string>

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "gdi32.lib")

namespace {

struct Vertex2D {
    float x, y;
    float u, v;
    float r, g, b, a;
};

struct GlyphInfo {
    float u0, v0, u1, v1;
    float w, h;
    float advance;
};

struct Color {
    float r, g, b, a;
};

static constexpr int ATLAS_W = 512;
static constexpr int ATLAS_H = 512;
static constexpr int FONT_PX = 28;
static constexpr size_t MAX_VERTS = 16384;

static const char* kShaderSrc = R"(
struct VS_IN {
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
struct PS_IN {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
    float4 col : COLOR0;
};
cbuffer CB0 : register(b0) {
    float2 invScreen;
    float2 pad;
};
PS_IN VS_Main(VS_IN input) {
    PS_IN o;
    float ndcX = input.pos.x * invScreen.x * 2.0f - 1.0f;
    float ndcY = 1.0f - input.pos.y * invScreen.y * 2.0f;
    o.pos = float4(ndcX, ndcY, 0.0f, 1.0f);
    o.uv  = input.uv;
    o.col = input.col;
    return o;
}
Texture2D tex0 : register(t0);
SamplerState samp0 : register(s0);
float4 PS_Main(PS_IN input) : SV_TARGET {
    float4 t = tex0.Sample(samp0, input.uv);
    return t * input.col;
}
)";

// ---- DX11 Hook state ----
using FnPresent       = HRESULT (STDMETHODCALLTYPE*)(IDXGISwapChain*, UINT, UINT);
using FnResizeBuffers = HRESULT (STDMETHODCALLTYPE*)(IDXGISwapChain*, UINT, UINT, UINT, DXGI_FORMAT, UINT);

static void**          g_swapChainVTable = nullptr;
static FnPresent       g_origPresent     = nullptr;
static FnResizeBuffers g_origResize      = nullptr;

// ---- Renderer resources ----
static ID3D11Device*             g_device   = nullptr;
static ID3D11DeviceContext*      g_ctx      = nullptr;
static ID3D11RenderTargetView*   g_rtv      = nullptr;
static ID3D11VertexShader*       g_vs       = nullptr;
static ID3D11PixelShader*        g_ps       = nullptr;
static ID3D11InputLayout*        g_layout   = nullptr;
static ID3D11Buffer*             g_vb       = nullptr;
static ID3D11Buffer*             g_cb       = nullptr;
static ID3D11BlendState*         g_blend    = nullptr;
static ID3D11DepthStencilState*  g_depth    = nullptr;
static ID3D11RasterizerState*    g_raster   = nullptr;
static ID3D11ShaderResourceView* g_atlasSRV = nullptr;
static ID3D11SamplerState*       g_sampler  = nullptr;

static GlyphInfo g_glyphs[128] = {};
static std::unordered_map<wchar_t, GlyphInfo> g_extraGlyphs;
static float     g_lineHeight  = 28.0f;
static float     g_screenW     = 1920.0f;
static float     g_screenH     = 1080.0f;
static bool      g_inited      = false;

static std::vector<Vertex2D> g_verts;

// Timing & hotkeys
static LARGE_INTEGER g_qpf = {};
static LARGE_INTEGER g_lastFrame = {};
static bool g_prevToggleDown = false;
static bool g_prevModeDown   = false;
static std::wstring g_toastText;
static float g_toastTimer = 0.0f;

static void ReleaseRTV() {
    if (g_rtv) { g_rtv->Release(); g_rtv = nullptr; }
}

static void ReleaseAllResources() {
    ReleaseRTV();
    if (g_sampler)  { g_sampler->Release();  g_sampler  = nullptr; }
    if (g_atlasSRV) { g_atlasSRV->Release(); g_atlasSRV = nullptr; }
    if (g_raster)   { g_raster->Release();   g_raster   = nullptr; }
    if (g_depth)    { g_depth->Release();    g_depth    = nullptr; }
    if (g_blend)    { g_blend->Release();    g_blend    = nullptr; }
    if (g_cb)       { g_cb->Release();       g_cb       = nullptr; }
    if (g_vb)       { g_vb->Release();       g_vb       = nullptr; }
    if (g_layout)   { g_layout->Release();   g_layout   = nullptr; }
    if (g_ps)       { g_ps->Release();       g_ps       = nullptr; }
    if (g_vs)       { g_vs->Release();       g_vs       = nullptr; }
    if (g_ctx)      { g_ctx->Release();      g_ctx      = nullptr; }
    if (g_device)   { g_device->Release();   g_device   = nullptr; }
    g_inited = false;
}

static bool CreateFontAtlas(ID3D11Device* dev) {
    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth       = ATLAS_W;
    bmi.bmiHeader.biHeight      = -ATLAS_H; // top-down
    bmi.bmiHeader.biPlanes      = 1;
    bmi.bmiHeader.biBitCount    = 32;
    bmi.bmiHeader.biCompression = BI_RGB;

    void* bits = nullptr;
    HDC hdc = CreateCompatibleDC(nullptr);
    HBITMAP hbm = CreateDIBSection(hdc, &bmi, DIB_RGB_COLORS, &bits, nullptr, 0);
    if (!hbm || !bits) {
        if (hbm) DeleteObject(hbm);
        DeleteDC(hdc);
        return false;
    }

    HGDIOBJ oldBmp = SelectObject(hdc, hbm);
    memset(bits, 0, ATLAS_W * ATLAS_H * 4);

    HFONT hFont = CreateFontW(
        -FONT_PX, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Segoe UI");
    HGDIOBJ oldFont = SelectObject(hdc, hFont);

    SetTextColor(hdc, RGB(255, 255, 255));
    SetBkMode(hdc, TRANSPARENT);

    TEXTMETRICW tm = {};
    GetTextMetricsW(hdc, &tm);
    g_lineHeight = static_cast<float>(tm.tmHeight);

    // Reserve top-left 6x6 pixels for solid white (solid rect UVs at 2/ATLAS_W, 2/ATLAS_H)
    int curX = 10;
    int curY = 2;
    int rowH = tm.tmHeight + 4;

    for (int c = 32; c < 127; ++c) {
        wchar_t ch = static_cast<wchar_t>(c);
        SIZE sz = {};
        GetTextExtentPoint32W(hdc, &ch, 1, &sz);
        int gw = sz.cx + 2;
        int gh = tm.tmHeight;

        if (curX + gw + 2 >= ATLAS_W) {
            curX = 2;
            curY += rowH;
        }
        if (curY + gh + 2 >= ATLAS_H) break;

        TextOutW(hdc, curX, curY, &ch, 1);

        GlyphInfo& g = g_glyphs[c];
        g.u0 = static_cast<float>(curX) / static_cast<float>(ATLAS_W);
        g.v0 = static_cast<float>(curY) / static_cast<float>(ATLAS_H);
        g.u1 = static_cast<float>(curX + gw) / static_cast<float>(ATLAS_W);
        g.v1 = static_cast<float>(curY + gh) / static_cast<float>(ATLAS_H);
        g.w  = static_cast<float>(gw);
        g.h  = static_cast<float>(gh);
        g.advance = static_cast<float>(sz.cx);

        curX += gw + 2;
    }

    // Rasterize CJK UI characters for built-in Simplified Chinese toast notifications
    // Characters: 血条与伤害数字开启关闭模式自定义仅原版显示跳
    static const wchar_t kCjkChars[] =
        L"\u8840\u6761\u4e0e\u4f24\u5bb3\u6570\u5b57\u5f00\u542f\u5173\u95ed"
        L"\u6a21\u5f0f\u81ea\u5b9a\u4e49\u4ec5\u539f\u7248\u663e\u793a\u8df3\u91cf";

    g_extraGlyphs.clear();
    HFONT hCjkFont = CreateFontW(
        -FONT_PX, 0, 0, 0, FW_BOLD, FALSE, FALSE, FALSE,
        GB2312_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS,
        ANTIALIASED_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Microsoft YaHei UI");
    if (hCjkFont) {
        SelectObject(hdc, hCjkFont);
        TEXTMETRICW tmCjk = {};
        GetTextMetricsW(hdc, &tmCjk);
        int cjkRowH = (std::max)(rowH, static_cast<int>(tmCjk.tmHeight) + 4);
        curX = 2;
        curY += rowH;

        for (const wchar_t* p = kCjkChars; *p; ++p) {
            wchar_t ch = *p;
            if (g_extraGlyphs.count(ch)) continue;
            SIZE sz = {};
            GetTextExtentPoint32W(hdc, &ch, 1, &sz);
            int gw = sz.cx + 2;
            int gh = tmCjk.tmHeight;

            if (curX + gw + 2 >= ATLAS_W) {
                curX = 2;
                curY += cjkRowH;
            }
            if (curY + gh + 2 >= ATLAS_H) break;

            TextOutW(hdc, curX, curY, &ch, 1);

            GlyphInfo g = {};
            g.u0 = static_cast<float>(curX) / static_cast<float>(ATLAS_W);
            g.v0 = static_cast<float>(curY) / static_cast<float>(ATLAS_H);
            g.u1 = static_cast<float>(curX + gw) / static_cast<float>(ATLAS_W);
            g.v1 = static_cast<float>(curY + gh) / static_cast<float>(ATLAS_H);
            g.w  = static_cast<float>(gw);
            g.h  = static_cast<float>(gh);
            g.advance = static_cast<float>(sz.cx);
            g_extraGlyphs[ch] = g;

            curX += gw + 2;
        }
        SelectObject(hdc, hFont);
        DeleteObject(hCjkFont);
    }

    GdiFlush();

    // Convert grayscale DIB into white RGB + luminance Alpha
    std::vector<uint32_t> rgba(ATLAS_W * ATLAS_H, 0);
    const uint32_t* src = static_cast<const uint32_t*>(bits);
    for (int i = 0; i < ATLAS_W * ATLAS_H; ++i) {
        uint32_t px = src[i];
        uint32_t lum = (px & 0xFFu); // B channel (white text on black)
        // Slight gamma boost on alpha for crisper readability at small sizes
        if (lum > 0) {
            float a = powf(static_cast<float>(lum) / 255.0f, 0.80f);
            uint32_t ia = static_cast<uint32_t>((std::min)(255.0f, a * 255.0f + 0.5f));
            rgba[i] = (ia << 24) | 0x00FFFFFFu;
        }
    }

    // Fill top-left 6x6 solid white block
    for (int y = 0; y < 6; ++y) {
        for (int x = 0; x < 6; ++x) {
            rgba[y * ATLAS_W + x] = 0xFFFFFFFFu;
        }
    }

    SelectObject(hdc, oldFont);
    SelectObject(hdc, oldBmp);
    DeleteObject(hFont);
    DeleteObject(hbm);
    DeleteDC(hdc);

    D3D11_TEXTURE2D_DESC td = {};
    td.Width              = ATLAS_W;
    td.Height             = ATLAS_H;
    td.MipLevels          = 1;
    td.ArraySize          = 1;
    td.Format             = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count   = 1;
    td.Usage              = D3D11_USAGE_IMMUTABLE;
    td.BindFlags          = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem     = rgba.data();
    initData.SysMemPitch = ATLAS_W * 4;

    ID3D11Texture2D* tex = nullptr;
    HRESULT hr = dev->CreateTexture2D(&td, &initData, &tex);
    if (FAILED(hr) || !tex) return false;

    hr = dev->CreateShaderResourceView(tex, nullptr, &g_atlasSRV);
    tex->Release();
    return SUCCEEDED(hr);
}

static bool EnsureRTV(IDXGISwapChain* sc) {
    if (g_rtv) return true;
    ID3D11Texture2D* backBuf = nullptr;
    if (FAILED(sc->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&backBuf))) || !backBuf) {
        return false;
    }
    D3D11_TEXTURE2D_DESC desc = {};
    backBuf->GetDesc(&desc);
    g_screenW = static_cast<float>(desc.Width);
    g_screenH = static_cast<float>(desc.Height);

    HRESULT hr = g_device->CreateRenderTargetView(backBuf, nullptr, &g_rtv);
    backBuf->Release();
    return SUCCEEDED(hr);
}

static bool InitDeviceResources(IDXGISwapChain* sc) {
    if (g_inited) return EnsureRTV(sc);

    if (FAILED(sc->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&g_device))) || !g_device) {
        return false;
    }
    g_device->GetImmediateContext(&g_ctx);
    if (!g_ctx) return false;

    ID3DBlob* vsBlob = nullptr;
    ID3DBlob* psBlob = nullptr;
    ID3DBlob* errBlob = nullptr;

    HRESULT hr = D3DCompile(kShaderSrc, strlen(kShaderSrc), nullptr, nullptr, nullptr,
                            "VS_Main", "vs_4_0", 0, 0, &vsBlob, &errBlob);
    if (errBlob) { errBlob->Release(); errBlob = nullptr; }
    if (FAILED(hr)) { ReleaseAllResources(); return false; }

    hr = D3DCompile(kShaderSrc, strlen(kShaderSrc), nullptr, nullptr, nullptr,
                    "PS_Main", "ps_4_0", 0, 0, &psBlob, &errBlob);
    if (errBlob) { errBlob->Release(); errBlob = nullptr; }
    if (FAILED(hr)) { vsBlob->Release(); ReleaseAllResources(); return false; }

    hr = g_device->CreateVertexShader(vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), nullptr, &g_vs);
    if (SUCCEEDED(hr)) {
        hr = g_device->CreatePixelShader(psBlob->GetBufferPointer(), psBlob->GetBufferSize(), nullptr, &g_ps);
    }
    if (SUCCEEDED(hr)) {
        D3D11_INPUT_ELEMENT_DESC ied[] = {
            {"POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 8,  D3D11_INPUT_PER_VERTEX_DATA, 0},
            {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA, 0},
        };
        hr = g_device->CreateInputLayout(ied, 3, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &g_layout);
    }
    vsBlob->Release();
    psBlob->Release();
    if (FAILED(hr)) { ReleaseAllResources(); return false; }

    D3D11_BUFFER_DESC vbd = {};
    vbd.ByteWidth      = static_cast<UINT>(MAX_VERTS * sizeof(Vertex2D));
    vbd.Usage          = D3D11_USAGE_DYNAMIC;
    vbd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;
    vbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(g_device->CreateBuffer(&vbd, nullptr, &g_vb))) { ReleaseAllResources(); return false; }

    D3D11_BUFFER_DESC cbd = {};
    cbd.ByteWidth      = 16;
    cbd.Usage          = D3D11_USAGE_DYNAMIC;
    cbd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    if (FAILED(g_device->CreateBuffer(&cbd, nullptr, &g_cb))) { ReleaseAllResources(); return false; }

    D3D11_BLEND_DESC bd = {};
    bd.RenderTarget[0].BlendEnable           = TRUE;
    bd.RenderTarget[0].SrcBlend              = D3D11_BLEND_SRC_ALPHA;
    bd.RenderTarget[0].DestBlend             = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOp               = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].SrcBlendAlpha         = D3D11_BLEND_ONE;
    bd.RenderTarget[0].DestBlendAlpha        = D3D11_BLEND_INV_SRC_ALPHA;
    bd.RenderTarget[0].BlendOpAlpha          = D3D11_BLEND_OP_ADD;
    bd.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    if (FAILED(g_device->CreateBlendState(&bd, &g_blend))) { ReleaseAllResources(); return false; }

    D3D11_DEPTH_STENCIL_DESC dsd = {};
    dsd.DepthEnable    = FALSE;
    dsd.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
    if (FAILED(g_device->CreateDepthStencilState(&dsd, &g_depth))) { ReleaseAllResources(); return false; }

    D3D11_RASTERIZER_DESC rd = {};
    rd.FillMode        = D3D11_FILL_SOLID;
    rd.CullMode        = D3D11_CULL_NONE;
    rd.DepthClipEnable = FALSE;
    if (FAILED(g_device->CreateRasterizerState(&rd, &g_raster))) { ReleaseAllResources(); return false; }

    D3D11_SAMPLER_DESC sd = {};
    sd.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU       = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressV       = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.AddressW       = D3D11_TEXTURE_ADDRESS_CLAMP;
    sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
    if (FAILED(g_device->CreateSamplerState(&sd, &g_sampler))) { ReleaseAllResources(); return false; }

    if (!CreateFontAtlas(g_device)) { ReleaseAllResources(); return false; }
    if (!EnsureRTV(sc)) { ReleaseAllResources(); return false; }

    g_verts.reserve(2048);
    g_inited = true;
    LogPrintf("D3D11 overlay initialized (%.0fx%.0f).", g_screenW, g_screenH);
    return true;
}

// ---- 2D Batch Primitives ----
static void PushQuad(float x0, float y0, float x1, float y1,
                     float u0, float v0, float u1, float v1, const Color& c) {
    if (g_verts.size() + 6 > MAX_VERTS) return;
    Vertex2D v[4] = {
        {x0, y0, u0, v0, c.r, c.g, c.b, c.a},
        {x1, y0, u1, v0, c.r, c.g, c.b, c.a},
        {x1, y1, u1, v1, c.r, c.g, c.b, c.a},
        {x0, y1, u0, v1, c.r, c.g, c.b, c.a},
    };
    g_verts.push_back(v[0]);
    g_verts.push_back(v[1]);
    g_verts.push_back(v[2]);
    g_verts.push_back(v[0]);
    g_verts.push_back(v[2]);
    g_verts.push_back(v[3]);
}

static void DrawRect(float x, float y, float w, float h, const Color& c) {
    constexpr float wu = 2.5f / static_cast<float>(ATLAS_W);
    constexpr float wv = 2.5f / static_cast<float>(ATLAS_H);
    PushQuad(x, y, x + w, y + h, wu, wv, wu, wv, c);
}

static float MeasureTextWidth(const char* text, float scale) {
    float w = 0.0f;
    for (const char* p = text; *p; ++p) {
        unsigned char c = static_cast<unsigned char>(*p);
        if (c >= 32 && c < 127) w += g_glyphs[c].advance * scale;
    }
    return w;
}

static void DrawTextRaw(float x, float y, const char* text, float scale, const Color& c) {
    float cx = x;
    for (const char* p = text; *p; ++p) {
        unsigned char ch = static_cast<unsigned char>(*p);
        if (ch < 32 || ch >= 127) continue;
        const GlyphInfo& g = g_glyphs[ch];
        if (ch != ' ') {
            PushQuad(cx, y, cx + g.w * scale, y + g.h * scale, g.u0, g.v0, g.u1, g.v1, c);
        }
        cx += g.advance * scale;
    }
}

static void DrawTextOutlined(float cx, float y, const char* text, float scale, const Color& fg, bool centered) {
    float x = centered ? (cx - MeasureTextWidth(text, scale) * 0.5f) : cx;
    x = floorf(x + 0.5f);
    y = floorf(y + 0.5f);

    Color outline = {0.0f, 0.0f, 0.0f, fg.a * 0.90f};
    float d = (std::max)(1.0f, scale * 1.15f);
    DrawTextRaw(x - d, y,     text, scale, outline);
    DrawTextRaw(x + d, y,     text, scale, outline);
    DrawTextRaw(x,     y - d, text, scale, outline);
    DrawTextRaw(x,     y + d, text, scale, outline);
    DrawTextRaw(x + d, y + d, text, scale, {0.0f, 0.0f, 0.0f, fg.a * 0.65f});
    DrawTextRaw(x,     y,     text, scale, fg);
}

static const GlyphInfo* FindGlyphW(wchar_t ch) {
    if (ch >= 32 && ch < 127) return &g_glyphs[ch];
    auto it = g_extraGlyphs.find(ch);
    return (it != g_extraGlyphs.end()) ? &it->second : nullptr;
}

static float MeasureTextWidthW(const wchar_t* text, float scale) {
    float w = 0.0f;
    for (const wchar_t* p = text; *p; ++p) {
        if (const GlyphInfo* g = FindGlyphW(*p)) w += g->advance * scale;
    }
    return w;
}

static void DrawTextRawW(float x, float y, const wchar_t* text, float scale, const Color& c) {
    float cx = x;
    for (const wchar_t* p = text; *p; ++p) {
        const GlyphInfo* g = FindGlyphW(*p);
        if (!g) continue;
        if (*p != L' ') {
            PushQuad(cx, y, cx + g->w * scale, y + g->h * scale, g->u0, g->v0, g->u1, g->v1, c);
        }
        cx += g->advance * scale;
    }
}

static void DrawTextOutlinedW(float cx, float y, const wchar_t* text, float scale, const Color& fg, bool centered) {
    float x = centered ? (cx - MeasureTextWidthW(text, scale) * 0.5f) : cx;
    x = floorf(x + 0.5f);
    y = floorf(y + 0.5f);

    Color outline = {0.0f, 0.0f, 0.0f, fg.a * 0.90f};
    float d = (std::max)(1.0f, scale * 1.15f);
    DrawTextRawW(x - d, y,     text, scale, outline);
    DrawTextRawW(x + d, y,     text, scale, outline);
    DrawTextRawW(x,     y - d, text, scale, outline);
    DrawTextRawW(x,     y + d, text, scale, outline);
    DrawTextRawW(x + d, y + d, text, scale, {0.0f, 0.0f, 0.0f, fg.a * 0.65f});
    DrawTextRawW(x,     y,     text, scale, fg);
}

static Color HealthColor(float ratio, float alpha) {
    if (ratio > 0.66f) return {0.30f, 0.85f, 0.42f, alpha}; // Tactical green
    if (ratio > 0.33f) return {0.96f, 0.77f, 0.24f, alpha}; // Warm amber
    return {0.92f, 0.26f, 0.26f, alpha};                    // Crimson red
}

static void RenderOverlayContent(const FrameSnapshot& snap, float dt) {
    g_verts.clear();

    if (g_toastTimer > 0.0f) {
        g_toastTimer -= dt;
        float a = (std::min)(1.0f, g_toastTimer * 2.0f);
        DrawTextOutlinedW(24.0f, 24.0f, g_toastText.c_str(), 0.78f, {0.95f, 0.95f, 0.95f, a}, false);
    }

    if (!g_cfg.enabled || !snap.hasCamera) return;

    // 1. Enemy Health Bars + Exact HP Numbers
    if (g_cfg.showHealthBar) {
        for (const auto& e : snap.enemies) {
            if (e.maxHP <= 0.0f) continue;

            float dx = e.worldPos.x - snap.camPos.x;
            float dy = e.worldPos.y - snap.camPos.y;
            float dz = e.worldPos.z - snap.camPos.z;
            float dist = sqrtf(dx * dx + dy * dy + dz * dz);
            if (dist > g_cfg.maxDistance) continue;

            float sx = 0.0f, sy = 0.0f;
            if (!WorldToScreen(snap.viewProj, e.worldPos, g_screenW, g_screenH, sx, sy)) continue;

            // Distance fade & gentle scaling
            float distFade = 1.0f;
            if (dist > g_cfg.maxDistance * 0.78f) {
                distFade = 1.0f - (dist - g_cfg.maxDistance * 0.78f) / (g_cfg.maxDistance * 0.22f);
                distFade = (std::max)(0.0f, (std::min)(1.0f, distFade));
            }
            float alpha = distFade * (e.isDead ? 0.55f : 0.96f);
            if (alpha <= 0.02f) continue;

            float distScale = 1.0f - (dist / 85.0f);
            distScale = (std::max)(0.74f, (std::min)(1.0f, distScale));

            float barW = g_cfg.barWidth * distScale;
            float barH = g_cfg.barHeight * distScale;
            float bx = floorf(sx - barW * 0.5f + 0.5f);
            float by = floorf(sy + g_cfg.barOffsetY * distScale + 0.5f);

            float hpRatio   = (std::max)(0.0f, (std::min)(1.0f, e.curHP / e.maxHP));
            float trailRatio = (std::max)(hpRatio, (std::min)(1.0f, e.displayHP / e.maxHP));

            if (g_cfg.drawCustomBar) {
                // Drop shadow
                DrawRect(bx - 1.0f, by - 1.0f, barW + 3.0f, barH + 3.0f, {0.0f, 0.0f, 0.0f, alpha * 0.55f});
                // Outer crisp border
                DrawRect(bx - 1.0f, by - 1.0f, barW + 2.0f, barH + 2.0f, {0.04f, 0.05f, 0.06f, alpha * 0.92f});
                // Dark background track
                DrawRect(bx, by, barW, barH, {0.10f, 0.11f, 0.13f, alpha * 0.85f});
                // Trailing damage-drain bar
                if (trailRatio > hpRatio + 0.003f) {
                    DrawRect(bx, by, barW * trailRatio, barH, {1.0f, 0.88f, 0.58f, alpha * 0.88f});
                }
                // Main HP fill
                if (hpRatio > 0.001f) {
                    Color fillCol = HealthColor(hpRatio, alpha);
                    DrawRect(bx, by, barW * hpRatio, barH, fillCol);
                    // Subtle 1px top highlight for depth
                    DrawRect(bx, by, barW * hpRatio, (std::max)(1.0f, barH * 0.22f),
                             {1.0f, 1.0f, 1.0f, alpha * 0.20f});
                }
            }

            if (g_cfg.showHpNumbers) {
                char hpBuf[64];
                int curInt = static_cast<int>(ceilf(e.curHP - 0.01f));
                if (curInt < 0) curInt = 0;
                int maxInt = static_cast<int>(floorf(e.maxHP + 0.5f));
                if (g_cfg.showPercent) {
                    int pct = static_cast<int>(floorf(hpRatio * 100.0f + 0.5f));
                    snprintf(hpBuf, sizeof(hpBuf), "%d / %d (%d%%)", curInt, maxInt, pct);
                } else {
                    snprintf(hpBuf, sizeof(hpBuf), "%d / %d", curInt, maxInt);
                }
                float txtScale = g_cfg.hpFontScale * distScale;
                float txtY = by - g_lineHeight * txtScale - 2.0f;
                Color txtCol = e.isDead
                    ? Color{0.85f, 0.35f, 0.35f, alpha}
                    : Color{0.97f, 0.97f, 0.97f, alpha};
                DrawTextOutlined(sx, txtY, hpBuf, txtScale, txtCol, true);
            }
        }
    }

    // 2. Floating Damage Numbers
    if (g_cfg.showDamageNumbers) {
        for (const auto& pop : snap.popups) {
            float sx = 0.0f, sy = 0.0f;
            if (!WorldToScreen(snap.viewProj, pop.worldPos, g_screenW, g_screenH, sx, sy)) continue;

            float t = (pop.duration > 0.0f) ? (pop.age / pop.duration) : 1.0f;
            if (t >= 1.0f) continue;

            // Pop-in scale bounce in first 0.14s
            float popScale = 1.0f;
            if (pop.age < 0.14f) {
                float p = pop.age / 0.14f;
                popScale = 1.38f - 0.38f * p * p;
            }

            // Smooth fade out in last 35%
            float alpha = 1.0f;
            if (t > 0.65f) {
                alpha = 1.0f - (t - 0.65f) / 0.35f;
            }
            alpha = (std::max)(0.0f, (std::min)(1.0f, alpha));

            float scale = g_cfg.dmgFontScale * popScale;
            Color col = {1.0f, 1.0f, 1.0f, alpha};
            if (pop.isKill) {
                col = {1.0f, 0.28f, 0.24f, alpha}; // Vivid crimson for kill blow
                scale *= 1.28f;
            } else if (pop.isCrit) {
                col = {1.0f, 0.82f, 0.22f, alpha}; // Bright gold for heavy/crit
                scale *= 1.20f;
            }

            char dmgBuf[48];
            if (pop.amount >= 10.0f) {
                snprintf(dmgBuf, sizeof(dmgBuf), "%d%s",
                         static_cast<int>(floorf(pop.amount + 0.5f)),
                         (pop.isKill && g_cfg.showKillBadge) ? "!" : "");
            } else {
                snprintf(dmgBuf, sizeof(dmgBuf), "%.1f%s",
                         pop.amount,
                         (pop.isKill && g_cfg.showKillBadge) ? "!" : "");
            }

            DrawTextOutlined(sx + pop.screenOffsetX, sy + pop.screenOffsetY, dmgBuf, scale, col, true);
        }
    }
}

static void FlushVertices() {
    if (g_verts.empty() || !g_ctx || !g_rtv) return;

    // ---- Backup D3D11 state ----
    ID3D11RenderTargetView*  prevRTV = nullptr;
    ID3D11DepthStencilView*  prevDSV = nullptr;
    g_ctx->OMGetRenderTargets(1, &prevRTV, &prevDSV);

    UINT numVP = D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE;
    D3D11_VIEWPORT prevVPs[D3D11_VIEWPORT_AND_SCISSORRECT_OBJECT_COUNT_PER_PIPELINE];
    g_ctx->RSGetViewports(&numVP, prevVPs);

    D3D11_PRIMITIVE_TOPOLOGY prevTopo;
    g_ctx->IAGetPrimitiveTopology(&prevTopo);
    ID3D11InputLayout* prevLayout = nullptr;
    g_ctx->IAGetInputLayout(&prevLayout);
    ID3D11Buffer* prevVB = nullptr;
    UINT prevStride = 0, prevOffset = 0;
    g_ctx->IAGetVertexBuffers(0, 1, &prevVB, &prevStride, &prevOffset);

    ID3D11VertexShader* prevVS = nullptr;
    ID3D11PixelShader*  prevPS = nullptr;
    ID3D11GeometryShader* prevGS = nullptr;
    g_ctx->VSGetShader(&prevVS, nullptr, nullptr);
    g_ctx->PSGetShader(&prevPS, nullptr, nullptr);
    g_ctx->GSGetShader(&prevGS, nullptr, nullptr);

    ID3D11Buffer* prevVSCB = nullptr;
    g_ctx->VSGetConstantBuffers(0, 1, &prevVSCB);
    ID3D11ShaderResourceView* prevSRV = nullptr;
    g_ctx->PSGetShaderResources(0, 1, &prevSRV);
    ID3D11SamplerState* prevSamp = nullptr;
    g_ctx->PSGetSamplers(0, 1, &prevSamp);

    ID3D11BlendState* prevBlend = nullptr;
    FLOAT prevBlendFactor[4] = {};
    UINT prevSampleMask = 0;
    g_ctx->OMGetBlendState(&prevBlend, prevBlendFactor, &prevSampleMask);

    ID3D11DepthStencilState* prevDepth = nullptr;
    UINT prevStencilRef = 0;
    g_ctx->OMGetDepthStencilState(&prevDepth, &prevStencilRef);

    ID3D11RasterizerState* prevRaster = nullptr;
    g_ctx->RSGetState(&prevRaster);

    // ---- Upload CB & VB ----
    D3D11_MAPPED_SUBRESOURCE mapped = {};
    if (SUCCEEDED(g_ctx->Map(g_cb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        float cbData[4] = {1.0f / g_screenW, 1.0f / g_screenH, 0.0f, 0.0f};
        memcpy(mapped.pData, cbData, sizeof(cbData));
        g_ctx->Unmap(g_cb, 0);
    }

    if (SUCCEEDED(g_ctx->Map(g_vb, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped))) {
        memcpy(mapped.pData, g_verts.data(), g_verts.size() * sizeof(Vertex2D));
        g_ctx->Unmap(g_vb, 0);
    }

    // ---- Set overlay pipeline & draw ----
    D3D11_VIEWPORT vp = {0.0f, 0.0f, g_screenW, g_screenH, 0.0f, 1.0f};
    g_ctx->OMSetRenderTargets(1, &g_rtv, nullptr);
    g_ctx->RSSetViewports(1, &vp);
    g_ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    g_ctx->IASetInputLayout(g_layout);
    UINT stride = sizeof(Vertex2D), offset = 0;
    g_ctx->IASetVertexBuffers(0, 1, &g_vb, &stride, &offset);
    g_ctx->VSSetShader(g_vs, nullptr, 0);
    g_ctx->VSSetConstantBuffers(0, 1, &g_cb);
    g_ctx->PSSetShader(g_ps, nullptr, 0);
    g_ctx->PSSetShaderResources(0, 1, &g_atlasSRV);
    g_ctx->PSSetSamplers(0, 1, &g_sampler);
    g_ctx->GSSetShader(nullptr, nullptr, 0);
    float bf[4] = {0, 0, 0, 0};
    g_ctx->OMSetBlendState(g_blend, bf, 0xFFFFFFFFu);
    g_ctx->OMSetDepthStencilState(g_depth, 0);
    g_ctx->RSSetState(g_raster);

    g_ctx->Draw(static_cast<UINT>(g_verts.size()), 0);

    // ---- Restore D3D11 state ----
    g_ctx->OMSetRenderTargets(1, &prevRTV, prevDSV);
    g_ctx->RSSetViewports(numVP, prevVPs);
    g_ctx->IASetPrimitiveTopology(prevTopo);
    g_ctx->IASetInputLayout(prevLayout);
    g_ctx->IASetVertexBuffers(0, 1, &prevVB, &prevStride, &prevOffset);
    g_ctx->VSSetShader(prevVS, nullptr, 0);
    g_ctx->VSSetConstantBuffers(0, 1, &prevVSCB);
    g_ctx->PSSetShader(prevPS, nullptr, 0);
    g_ctx->PSSetShaderResources(0, 1, &prevSRV);
    g_ctx->PSSetSamplers(0, 1, &prevSamp);
    g_ctx->GSSetShader(prevGS, nullptr, 0);
    g_ctx->OMSetBlendState(prevBlend, prevBlendFactor, prevSampleMask);
    g_ctx->OMSetDepthStencilState(prevDepth, prevStencilRef);
    g_ctx->RSSetState(prevRaster);

    if (prevRTV)    prevRTV->Release();
    if (prevDSV)    prevDSV->Release();
    if (prevLayout) prevLayout->Release();
    if (prevVB)     prevVB->Release();
    if (prevVS)     prevVS->Release();
    if (prevPS)     prevPS->Release();
    if (prevGS)     prevGS->Release();
    if (prevVSCB)   prevVSCB->Release();
    if (prevSRV)    prevSRV->Release();
    if (prevSamp)   prevSamp->Release();
    if (prevBlend)  prevBlend->Release();
    if (prevDepth)  prevDepth->Release();
    if (prevRaster) prevRaster->Release();
}

static void HandleHotkeys() {
    if (g_cfg.toggleKey > 0) {
        bool down = (GetAsyncKeyState(g_cfg.toggleKey) & 0x8000) != 0;
        if (down && !g_prevToggleDown) {
            g_cfg.enabled = !g_cfg.enabled;
            if (g_cfg.useChinese) {
                g_toastText = g_cfg.enabled
                    ? L"\u8840\u6761\u4e0e\u4f24\u5bb3\u6570\u5b57: \u5f00\u542f"
                    : L"\u8840\u6761\u4e0e\u4f24\u5bb3\u6570\u5b57: \u5173\u95ed";
            } else {
                g_toastText = g_cfg.enabled ? L"HP & Damage Overlay: ON" : L"HP & Damage Overlay: OFF";
            }
            g_toastTimer = 2.0f;
        }
        g_prevToggleDown = down;
    }
    if (g_cfg.modeKey > 0) {
        bool down = (GetAsyncKeyState(g_cfg.modeKey) & 0x8000) != 0;
        if (down && !g_prevModeDown) {
            if (g_cfg.showHealthBar && g_cfg.drawCustomBar) {
                g_cfg.drawCustomBar = false;
                g_toastText = g_cfg.useChinese
                    ? L"\u8840\u6761\u6a21\u5f0f: \u4ec5\u8840\u91cf\u6570\u5b57 (\u539f\u7248\u8840\u6761)"
                    : L"Bar Mode: HP Numbers Only (Vanilla Bar)";
            } else if (g_cfg.showHealthBar && !g_cfg.drawCustomBar) {
                g_cfg.showHealthBar = false;
                g_toastText = g_cfg.useChinese
                    ? L"\u8840\u6761\u6a21\u5f0f: \u4ec5\u663e\u793a\u4f24\u5bb3\u8df3\u5b57"
                    : L"Bar Mode: Damage Numbers Only";
            } else {
                g_cfg.showHealthBar = true;
                g_cfg.drawCustomBar = true;
                g_toastText = g_cfg.useChinese
                    ? L"\u8840\u6761\u6a21\u5f0f: \u81ea\u5b9a\u4e49\u8840\u6761 + \u8840\u91cf\u6570\u5b57"
                    : L"Bar Mode: Custom Health Bar + HP Numbers";
            }
            g_toastTimer = 2.2f;
        }
        g_prevModeDown = down;
    }
}

static HRESULT STDMETHODCALLTYPE HkPresent(IDXGISwapChain* sc, UINT syncInterval, UINT flags) {
    if (g_qpf.QuadPart == 0) {
        QueryPerformanceFrequency(&g_qpf);
        QueryPerformanceCounter(&g_lastFrame);
    }
    LARGE_INTEGER now = {};
    QueryPerformanceCounter(&now);
    float dt = static_cast<float>(now.QuadPart - g_lastFrame.QuadPart) / static_cast<float>(g_qpf.QuadPart);
    g_lastFrame = now;

    HandleHotkeys();

    if (InitDeviceResources(sc)) {
        FrameSnapshot snap;
        GameHooksUpdateAndSnapshot(dt, snap);
        RenderOverlayContent(snap, dt);
        FlushVertices();
    }

    return g_origPresent(sc, syncInterval, flags);
}

static HRESULT STDMETHODCALLTYPE HkResizeBuffers(IDXGISwapChain* sc, UINT bufferCount,
                                                  UINT width, UINT height,
                                                  DXGI_FORMAT newFormat, UINT swapChainFlags) {
    ReleaseRTV();
    return g_origResize(sc, bufferCount, width, height, newFormat, swapChainFlags);
}

} // namespace

bool D3D11OverlayInit() {
    // Create a hidden dummy window + D3D11 swapchain to obtain the active IDXGISwapChain vtable.
    WNDCLASSEXA wc = {};
    wc.cbSize        = sizeof(wc);
    wc.lpfnWndProc   = DefWindowProcA;
    wc.hInstance     = GetModuleHandleA(nullptr);
    wc.lpszClassName = "DL1HpDmgDummyWnd";
    RegisterClassExA(&wc);

    HWND hwnd = CreateWindowExA(0, wc.lpszClassName, "", WS_OVERLAPPEDWINDOW,
                                0, 0, 100, 100, nullptr, nullptr, wc.hInstance, nullptr);
    if (!hwnd) return false;

    DXGI_SWAP_CHAIN_DESC sd = {};
    sd.BufferCount       = 1;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage       = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow      = hwnd;
    sd.SampleDesc.Count  = 1;
    sd.Windowed          = TRUE;
    sd.SwapEffect        = DXGI_SWAP_EFFECT_DISCARD;

    IDXGISwapChain* dummySC = nullptr;
    ID3D11Device* dummyDev = nullptr;
    ID3D11DeviceContext* dummyCtx = nullptr;
    D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, 0,
        &fl, 1, D3D11_SDK_VERSION, &sd, &dummySC, &dummyDev, nullptr, &dummyCtx);

    if (FAILED(hr) || !dummySC) {
        LogPrintf("ERROR: Dummy D3D11CreateDeviceAndSwapChain failed (hr=0x%08X)", static_cast<unsigned>(hr));
        DestroyWindow(hwnd);
        UnregisterClassA(wc.lpszClassName, wc.hInstance);
        return false;
    }

    g_swapChainVTable = *reinterpret_cast<void***>(dummySC);
    dummySC->Release();
    if (dummyCtx) dummyCtx->Release();
    if (dummyDev) dummyDev->Release();
    DestroyWindow(hwnd);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);

    if (!g_swapChainVTable) return false;

    void* origPres = nullptr;
    void* origRes  = nullptr;
    bool ok1 = PatchVTableSlot(g_swapChainVTable, 8,  reinterpret_cast<void*>(&HkPresent), &origPres);
    bool ok2 = PatchVTableSlot(g_swapChainVTable, 13, reinterpret_cast<void*>(&HkResizeBuffers), &origRes);
    g_origPresent = reinterpret_cast<FnPresent>(origPres);
    g_origResize  = reinterpret_cast<FnResizeBuffers>(origRes);

    LogPrintf("Hooked IDXGISwapChain vtable %p: Present=%p (%d) ResizeBuffers=%p (%d)",
              g_swapChainVTable, origPres, ok1, origRes, ok2);
    return ok1;
}

void D3D11OverlayShutdown() {
    if (g_swapChainVTable) {
        if (g_origPresent) PatchVTableSlot(g_swapChainVTable, 8, reinterpret_cast<void*>(g_origPresent), nullptr);
        if (g_origResize)  PatchVTableSlot(g_swapChainVTable, 13, reinterpret_cast<void*>(g_origResize), nullptr);
    }
    ReleaseAllResources();
}
