#pragma once

#include <QtGlobal>

#ifdef Q_OS_WIN
#include <windows.h>
#include <dwrite.h>

namespace source::utils {

namespace detail {

// GUID oficial da interface IDWriteFontFace7 (DWriteCore / Windows 11 COLRv1):
// {3945B85B-BC95-40F7-B72C-8B73BFC7E13B}
static const GUID kIID_IDWriteFontFace7 = 
    { 0x3945b85b, 0xbc95, 0x40f7, { 0xb7, 0x2c, 0x8b, 0x73, 0xbf, 0xc7, 0xe1, 0x3b } };

typedef HRESULT (STDMETHODCALLTYPE *DWriteQueryInterface_t)(IUnknown *This, REFIID riid, void **ppvObject);
inline DWriteQueryInterface_t s_originalDWriteQueryInterface = nullptr;

inline HRESULT STDMETHODCALLTYPE HookedDWriteQueryInterface(IUnknown *This, REFIID riid, void **ppvObject)
{
    // Intercepta e bloqueia a consulta pela interface IDWriteFontFace7 (COLRv1) no DirectWrite.
    // Isso instrui o motor de fontes do Qt (QWindowsFontEngineDirectWrite) a ignorar o pipeline
    // COLRv1 (que projeta esferas 3D com degradê radial, brilho especular e sombras no Windows 11)
    // e utilizar de forma nativa e estável a tabela COLRv0 (DirectWrite3 TranslateColorGlyphRun).
    // Desta forma, glifos como "🔴, 🟠, 🟡, 🟢" são renderizados como círculos sólidos monocromáticos
    // 2D (planos), padronizando a renderização do MSVC com o MinGW sem afetar a nitidez ou
    // o ClearType das fontes convencionais.
    if (IsEqualGUID(riid, kIID_IDWriteFontFace7)) {
        if (ppvObject) {
            *ppvObject = nullptr;
        }
        return E_NOINTERFACE;
    }
    return s_originalDWriteQueryInterface(This, riid, ppvObject);
}

} // namespace detail

/**
 * @brief Configura o DirectWrite no Windows para renderizar emojis coloridos
 * em formato COLRv0 (plano / monocromático sólido), eliminando o degradê esférico 3D (COLRv1).
 *
 * Deve ser invocado antes da instanciação do QApplication no main().
 */
inline void configure_emoji_flat_rendering()
{
    typedef HRESULT (WINAPI *DWriteCreateFactory_t)(DWRITE_FACTORY_TYPE, REFIID, IUnknown**);
    HMODULE hDWrite = LoadLibraryW(L"dwrite.dll");
    if (!hDWrite) {
        return;
    }

    auto pfnDWriteCreateFactory = reinterpret_cast<DWriteCreateFactory_t>(
        GetProcAddress(hDWrite, "DWriteCreateFactory"));
    if (!pfnDWriteCreateFactory) {
        return;
    }

    // GUID IDWriteFactory: {b859ee5a-d838-4b5b-a2e8-1adc7d93db48}
    static const GUID kIID_IDWriteFactory = 
        { 0xb859ee5a, 0xd838, 0x4b5b, { 0xa2, 0xe8, 0x1a, 0xdc, 0x7d, 0x93, 0xdb, 0x48 } };

    IDWriteFactory *factory = nullptr;
    HRESULT hr = pfnDWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, kIID_IDWriteFactory, reinterpret_cast<IUnknown**>(&factory));
    if (FAILED(hr) || !factory) {
        return;
    }

    IDWriteFontCollection *collection = nullptr;
    hr = factory->GetSystemFontCollection(&collection, FALSE);
    if (SUCCEEDED(hr) && collection) {
        UINT32 index = 0;
        BOOL exists = FALSE;
        collection->FindFamilyName(L"Segoe UI Emoji", &index, &exists);
        if (!exists) {
            index = 0;
        }

        IDWriteFontFamily *family = nullptr;
        if (SUCCEEDED(collection->GetFontFamily(index, &family)) && family) {
            IDWriteFont *font = nullptr;
            if (SUCCEEDED(family->GetFont(0, &font)) && font) {
                IDWriteFontFace *fontFace = nullptr;
                if (SUCCEEDED(font->CreateFontFace(&fontFace)) && fontFace) {
                    void **vtable = *reinterpret_cast<void***>(fontFace);
                    if (vtable && vtable[0] != detail::HookedDWriteQueryInterface) {
                        detail::s_originalDWriteQueryInterface = reinterpret_cast<detail::DWriteQueryInterface_t>(vtable[0]);
                        DWORD oldProtect = 0;
                        if (VirtualProtect(&vtable[0], sizeof(void*), PAGE_READWRITE, &oldProtect)) {
                            vtable[0] = reinterpret_cast<void*>(detail::HookedDWriteQueryInterface);
                            DWORD dummy = 0;
                            VirtualProtect(&vtable[0], sizeof(void*), oldProtect, &dummy);
                        }
                    }
                    fontFace->Release();
                }
                font->Release();
            }
            family->Release();
        }
        collection->Release();
    }
    factory->Release();
}

} // namespace source::utils
#else
namespace source::utils {
inline void configure_emoji_flat_rendering() {}
} // namespace source::utils
#endif
