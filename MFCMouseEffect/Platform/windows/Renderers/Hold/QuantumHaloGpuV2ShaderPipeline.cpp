#include "pch.h"
#include "QuantumHaloGpuV2ShaderPipeline.h"
#include "MouseFx/Utils/MathUtils.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <sstream>

namespace mousefx {

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

static std::string HexHr(HRESULT hr) {
    std::ostringstream ss;
    ss << "0x" << std::uppercase << std::hex << static_cast<unsigned long>(hr);
    return ss.str();
}

static float ComputeProgress(uint64_t elapsedMs, uint32_t holdMs, uint32_t durationMs) {
    const uint32_t threshold = (durationMs == 0u) ? 1u : durationMs;
    const float elapsedT = ClampFloat(static_cast<float>(elapsedMs) / static_cast<float>(threshold), 0.0f, 1.0f);
    if (holdMs == 0u) return elapsedT;
    const float holdT = ClampFloat(static_cast<float>(holdMs) / static_cast<float>(threshold), 0.0f, 1.0f);
    return (holdT > elapsedT) ? holdT : elapsedT;
}

// ---------------------------------------------------------------------------
// HLSL Shader Sources
// ---------------------------------------------------------------------------

static const char* kVsCode = R"(
struct VSOut {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};
VSOut main(uint vid : SV_VertexID) {
    float2 pos;
    pos.x = (vid == 2) ? 3.0 : -1.0;
    pos.y = (vid == 1) ? 3.0 : -1.0;
    VSOut o;
    o.pos = float4(pos, 0.0, 1.0);
    o.uv = float2((pos.x + 1.0) * 0.5, 1.0 - (pos.y + 1.0) * 0.5);
    return o;
}
)";

static const char* kPsCode = R"(
cbuffer Params : register(b0) {
    float2 Resolution;
    float2 Center;
    float TimeSec;
    float Progress;
    float BaseAlpha;
    float BaseRadius;
    float StrokeWidth;
    float _Pad0;
    float3 PrimaryColor;
    float _Pad1;
    float3 CyanColor;
    float _Pad2;
};
struct VSOut {
    float4 pos : SV_POSITION;
    float2 uv  : TEXCOORD0;
};
static const float PI = 3.14159265;
static const float TAU = 6.28318530;
float2 Rotate2(float2 p, float a) {
    float s = sin(a);
    float c = cos(a);
    return float2(c * p.x - s * p.y, s * p.x + c * p.y);
}
float Hash21(float2 p) {
    p = frac(p * float2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return frac(p.x * p.y);
}
float Noise2(float2 p) {
    float2 i = floor(p);
    float2 f = frac(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = Hash21(i);
    float b = Hash21(i + float2(1.0, 0.0));
    float c = Hash21(i + float2(0.0, 1.0));
    float d = Hash21(i + float2(1.0, 1.0));
    return lerp(lerp(a, b, f.x), lerp(c, d, f.x), f.y);
}
float Fbm(float2 p) {
    float v = 0.0;
    float a = 0.5;
    [unroll]
    for (int i = 0; i < 5; ++i) {
        v += a * Noise2(p);
        p = Rotate2(p * 1.85 + float2(7.31, -4.12), 0.55);
        a *= 0.5;
    }
    return v;
}
float WrappedAngDist(float a, float b) {
    float wrapped = atan2(sin(a - b), cos(a - b));
    return abs(wrapped);
}
float ArcSoftMask(float ang, float startAng, float sweepAng, float feather01) {
    float u = frac((ang - startAng) / TAU + 1.0);
    float lim = saturate(sweepAng / TAU);
    float edge = saturate((lim - u) / max(feather01, 1e-5));
    return smoothstep(0.0, 1.0, edge);
}
float RingGauss(float d, float r, float w) {
    float x = (d - r) / max(w, 0.001);
    return exp(-x * x);
}
float4 main(VSOut input) : SV_TARGET {
    float2 p = input.uv * Resolution - Center;
    float2 pn = p / max(min(Resolution.x, Resolution.y), 1.0);
    float d = length(p);
    float ang = atan2(p.y, p.x);

    float startAng = -PI * 0.5;
    float sweep = saturate(Progress) * TAU;
    float arcMask = ArcSoftMask(ang, startAng, sweep, 0.0075);
    float arcBody = ArcSoftMask(ang, startAng, sweep, 0.0600);

    float radius = BaseRadius;
    float thickness = max(StrokeWidth * 1.35, 2.2);
    float shell = RingGauss(d, radius, thickness * 1.30);
    float rim = RingGauss(d, radius, thickness * 0.52);
    float bloom = RingGauss(d, radius, thickness * 3.80);

    // Arc head and trailing capsule.
    float headAng = startAng + sweep;
    float2 headPos = float2(cos(headAng), sin(headAng)) * radius;
    float2 toHead = p - headPos;
    float headCore = exp(-dot(toHead, toHead) / max(thickness * thickness * 2.4, 1.0));
    float headTrail = exp(-WrappedAngDist(ang, headAng) * 8.0) * RingGauss(d, radius, thickness * 1.9) * smoothstep(0.02, 1.0, Progress);

    // Heavy layered field (GPU-friendly, CPU-expensive): dual rotating FBM volumes.
    float2 swirlUvA = Rotate2(pn * (8.0 + Progress * 10.0), TimeSec * 0.35);
    float2 swirlUvB = Rotate2(pn * (13.0 + Progress * 14.0), -TimeSec * 0.27 + 1.1);
    float plasmaA = Fbm(swirlUvA + float2(TimeSec * 0.24, -TimeSec * 0.16));
    float plasmaB = Fbm(swirlUvB + float2(-TimeSec * 0.18, TimeSec * 0.21));
    float plasma = saturate(plasmaA * 0.62 + plasmaB * 0.52);

    float ringShellMask = exp(-pow((d - radius) / max(radius * 0.28, 1.0), 2.0));
    float innerMask = exp(-pow(d / max(radius * 0.84, 1.0), 2.0));

    float caustic = pow(saturate(0.5 + 0.5 * cos(ang * 64.0 + TimeSec * 2.7 + plasma * 5.0)), 8.0);
    caustic *= ringShellMask;

    float spokes = pow(saturate(0.5 + 0.5 * cos(ang * 42.0 - TimeSec * 3.1 + d * 0.015)), 10.0);
    spokes *= exp(-pow((d - radius * 0.56) / max(radius * 0.34, 1.0), 2.0));

    float2 sparkUv = float2(ang * 7.5, d / max(radius, 1.0) * 9.0);
    float sparkNoise = Fbm(sparkUv + float2(TimeSec * 0.85, -TimeSec * 0.63));
    float sparkles = smoothstep(0.86, 0.985, sparkNoise) * ringShellMask * arcBody;
    sparkles *= (0.45 + 0.55 * sin(TimeSec * 18.0 + d * 0.03));

    float reactor = Fbm(Rotate2(pn * 16.0, TimeSec * 0.50) + float2(TimeSec * 0.11, -TimeSec * 0.14));
    float reactorCore = exp(-pow(d / max(radius * 0.33, 1.0), 2.0)) * (0.35 + 0.65 * reactor);

    float waveGate = 0.5 + 0.5 * sin(TimeSec * 2.1 + d * 0.06 + plasma * 6.0);
    float quantumWave = ringShellMask * waveGate * (0.35 + 0.65 * plasma) * arcBody;

    float3 primary = saturate(PrimaryColor);
    float3 cyan = saturate(CyanColor);
    float3 violet = saturate(float3(0.66, 0.36, 1.00) + primary * float3(0.10, 0.02, 0.14));
    float3 mint = saturate(float3(0.18, 0.98, 0.88) + cyan * float3(0.06, 0.05, 0.03));
    float3 white = float3(0.96, 0.99, 1.00);

    float alpha = 0.0;
    alpha += BaseAlpha * (rim * 0.48 + shell * 0.40 + bloom * 0.18);
    alpha += BaseAlpha * (arcMask * rim * 0.72 + arcBody * shell * 0.52 + headCore * 0.92 + headTrail * 0.72);
    alpha += BaseAlpha * (caustic * 0.20 + spokes * 0.18 + quantumWave * 0.32 + sparkles * 0.35 + reactorCore * 0.18);

    float envelopeRing = exp(-pow((d - radius) / max(radius * 0.58, 1.0), 2.0));
    float envelopeCore = exp(-pow(d / max(radius * 0.52, 1.0), 2.0)) * 0.48;
    float envelope = saturate(envelopeRing + envelopeCore);
    alpha = saturate(alpha * envelope);
    if (alpha < 0.007) {
        return float4(0.0, 0.0, 0.0, 0.0);
    }

    float3 color = 0.0;
    color += cyan * (rim * 0.52 + arcMask * rim * 0.56 + caustic * 0.34 + sparkles * 0.16);
    color += violet * (shell * 0.36 + arcBody * shell * 0.42 + headTrail * 0.42 + quantumWave * 0.36);
    color += mint * (quantumWave * 0.44 + reactorCore * 0.22 + plasma * innerMask * 0.16);
    color += primary * (headCore * 0.58 + arcMask * 0.24 + sparkles * 0.24);
    color += white * (headCore * 0.78 + sparkles * 0.56 + reactorCore * 0.22);
    color = saturate(color);
    color = color / (1.0 + color * 0.65);
    return float4(color * alpha, alpha);
}
)";

// ---------------------------------------------------------------------------
// QuantumHaloGpuV2ShaderPipeline
// ---------------------------------------------------------------------------

QuantumHaloGpuV2ShaderPipeline::~QuantumHaloGpuV2ShaderPipeline() {
    Shutdown();
}

bool QuantumHaloGpuV2ShaderPipeline::Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
    if (!device || !context) {
        SetError("pipeline_missing_device_or_context");
        return false;
    }
    device_ = device;
    context_ = context;
    return EnsureShaders();
}

void QuantumHaloGpuV2ShaderPipeline::Shutdown() {
    rtv_.Reset();
    swapChain_.Reset();
    rasterizer_.Reset();
    constantBuffer_.Reset();
    pixelShader_.Reset();
    vertexShader_.Reset();
    context_.Reset();
    device_.Reset();
    targetW_ = 0;
    targetH_ = 0;
}

bool QuantumHaloGpuV2ShaderPipeline::BindSwapChain(IDXGISwapChain1* swapChain, UINT w, UINT h) {
    if (!device_ || !context_ || !swapChain || w == 0 || h == 0) {
        SetError("bind_swapchain_invalid_args");
        return false;
    }

    const bool needRecreateTarget =
        (swapChain_.Get() != swapChain) || !rtv_ || (targetW_ != w) || (targetH_ != h);
    if (!needRecreateTarget) {
        return true;
    }

    swapChain_ = swapChain;
    rtv_.Reset();

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapChain_->GetBuffer(
        0,
        __uuidof(ID3D11Texture2D),
        reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr) || !backBuffer) {
        SetError("swapchain_get_backbuffer_failed_" + HexHr(hr));
        return false;
    }

    hr = device_->CreateRenderTargetView(backBuffer.Get(), nullptr, rtv_.GetAddressOf());
    if (FAILED(hr) || !rtv_) {
        SetError("create_rtv_failed_" + HexHr(hr));
        return false;
    }

    targetW_ = w;
    targetH_ = h;
    return true;
}

bool QuantumHaloGpuV2ShaderPipeline::Render(uint64_t elapsedMs, uint32_t holdMs, int sizePx, const RippleStyle& style) {
    if (!device_ || !context_ || !rtv_) {
        SetError("render_prereq_missing");
        return false;
    }

    if (!EnsureShaders()) {
        if (lastErrorReason_.empty()) {
            SetError("ensure_shaders_failed");
        }
        return false;
    }

    const float progress = ComputeProgress(elapsedMs, holdMs, style.durationMs);
    const float timeSec = static_cast<float>(elapsedMs) / 1000.0f;
    const uint32_t stroke = style.stroke.value;
    const float baseA = ClampFloat(static_cast<float>((stroke >> 24) & 0xFFu) / 255.0f, 0.06f, 1.0f);
    const float sr = static_cast<float>((stroke >> 16) & 0xFFu) / 255.0f;
    const float sg = static_cast<float>((stroke >> 8) & 0xFFu) / 255.0f;
    const float sb = static_cast<float>(stroke & 0xFFu) / 255.0f;
    const float pr = ClampFloat(sr * 0.70f + 0.28f, 0.0f, 1.0f);
    const float pg = ClampFloat(sg * 0.22f + 0.12f, 0.0f, 1.0f);
    const float pb = ClampFloat(sb * 0.94f + 0.06f, 0.0f, 1.0f);
    const float cr = ClampFloat(sr * 0.32f + 0.02f, 0.0f, 1.0f);
    const float cg = ClampFloat(sg * 0.88f + 0.22f, 0.0f, 1.0f);
    const float cb = ClampFloat(sb * 0.95f + 0.30f, 0.0f, 1.0f);
    const float radius = style.startRadius + (style.endRadius - style.startRadius) * progress;
    const float baseRadius = (std::min)(radius + 10.0f, sizePx * 0.42f);

    ShaderConstants constants{};
    constants.resolutionX = static_cast<float>(targetW_);
    constants.resolutionY = static_cast<float>(targetH_);
    constants.centerX = constants.resolutionX * 0.5f;
    constants.centerY = constants.resolutionY * 0.5f;
    constants.timeSec = timeSec;
    constants.progress = progress;
    constants.baseAlpha = baseA;
    constants.baseRadius = baseRadius;
    constants.strokeWidth = style.strokeWidth;
    constants.primaryR = pr;
    constants.primaryG = pg;
    constants.primaryB = pb;
    constants.cyanR = cr;
    constants.cyanG = cg;
    constants.cyanB = cb;

    D3D11_MAPPED_SUBRESOURCE mapped{};
    HRESULT hr = context_->Map(constantBuffer_.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr) || !mapped.pData) {
        SetError("map_constant_buffer_failed_" + HexHr(hr));
        return false;
    }
    *reinterpret_cast<ShaderConstants*>(mapped.pData) = constants;
    context_->Unmap(constantBuffer_.Get(), 0);

    const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
    context_->OMSetRenderTargets(1, rtv_.GetAddressOf(), nullptr);
    context_->ClearRenderTargetView(rtv_.Get(), clearColor);

    D3D11_VIEWPORT viewport{};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(targetW_);
    viewport.Height = static_cast<float>(targetH_);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    context_->RSSetViewports(1, &viewport);
    context_->RSSetState(rasterizer_.Get());

    context_->IASetInputLayout(nullptr);
    context_->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    context_->VSSetShader(vertexShader_.Get(), nullptr, 0);
    context_->PSSetShader(pixelShader_.Get(), nullptr, 0);
    context_->VSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
    context_->PSSetConstantBuffers(0, 1, constantBuffer_.GetAddressOf());
    context_->Draw(3, 0);
    return true;
}

const std::string& QuantumHaloGpuV2ShaderPipeline::LastErrorReason() const {
    return lastErrorReason_;
}

void QuantumHaloGpuV2ShaderPipeline::SetError(const std::string& reason) {
    lastErrorReason_ = reason;
}

bool QuantumHaloGpuV2ShaderPipeline::EnsureShaders() {
    if (vertexShader_ && pixelShader_ && constantBuffer_ && rasterizer_) {
        return true;
    }
    if (!device_) {
        SetError("ensure_shaders_no_device");
        return false;
    }

    UINT compileFlags = D3DCOMPILE_OPTIMIZATION_LEVEL3;
    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> psBlob;
    Microsoft::WRL::ComPtr<ID3DBlob> errBlob;

    HRESULT hr = D3DCompile(
        kVsCode,
        std::strlen(kVsCode),
        nullptr, nullptr, nullptr,
        "main", "vs_4_0",
        compileFlags, 0,
        vsBlob.GetAddressOf(),
        errBlob.GetAddressOf());
    if (FAILED(hr) || !vsBlob) {
        SetError("compile_vs_failed_" + HexHr(hr));
        return false;
    }

    errBlob.Reset();
    hr = D3DCompile(
        kPsCode,
        std::strlen(kPsCode),
        nullptr, nullptr, nullptr,
        "main", "ps_4_0",
        compileFlags, 0,
        psBlob.GetAddressOf(),
        errBlob.GetAddressOf());
    if (FAILED(hr) || !psBlob) {
        SetError("compile_ps_failed_" + HexHr(hr));
        return false;
    }

    hr = device_->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        vertexShader_.GetAddressOf());
    if (FAILED(hr) || !vertexShader_) {
        SetError("create_vs_failed_" + HexHr(hr));
        return false;
    }

    hr = device_->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        pixelShader_.GetAddressOf());
    if (FAILED(hr) || !pixelShader_) {
        SetError("create_ps_failed_" + HexHr(hr));
        return false;
    }

    D3D11_BUFFER_DESC cbDesc{};
    cbDesc.ByteWidth = sizeof(ShaderConstants);
    cbDesc.Usage = D3D11_USAGE_DYNAMIC;
    cbDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    cbDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    hr = device_->CreateBuffer(&cbDesc, nullptr, constantBuffer_.GetAddressOf());
    if (FAILED(hr) || !constantBuffer_) {
        SetError("create_constant_buffer_failed_" + HexHr(hr));
        return false;
    }

    D3D11_RASTERIZER_DESC rsDesc{};
    rsDesc.FillMode = D3D11_FILL_SOLID;
    rsDesc.CullMode = D3D11_CULL_NONE;
    rsDesc.DepthClipEnable = TRUE;
    hr = device_->CreateRasterizerState(&rsDesc, rasterizer_.GetAddressOf());
    if (FAILED(hr) || !rasterizer_) {
        SetError("create_rasterizer_failed_" + HexHr(hr));
        return false;
    }

    return true;
}

} // namespace mousefx
