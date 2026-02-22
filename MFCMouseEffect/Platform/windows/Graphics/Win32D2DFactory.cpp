#include "pch.h"
#include "Win32D2DFactory.h"

#include <wrl/client.h>

#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")

namespace mousefx {

ID2D1Factory* SharedD2D1Factory() {
    // C++11 magic static: thread-safe, lazy, leaked intentionally (process lifetime).
    static Microsoft::WRL::ComPtr<ID2D1Factory> factory = [] {
        Microsoft::WRL::ComPtr<ID2D1Factory> f;
        D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, f.GetAddressOf());
        return f;
    }();
    return factory.Get();
}

IDWriteFactory* SharedDWriteFactory() {
    static Microsoft::WRL::ComPtr<IDWriteFactory> factory = [] {
        Microsoft::WRL::ComPtr<IDWriteFactory> f;
        DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory),
            reinterpret_cast<IUnknown**>(f.GetAddressOf()));
        return f;
    }();
    return factory.Get();
}

} // namespace mousefx
