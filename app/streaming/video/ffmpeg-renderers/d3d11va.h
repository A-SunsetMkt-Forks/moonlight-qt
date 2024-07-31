#pragma once

#include "renderer.h"

#include <d3d11_1.h>
#include <dxgi1_5.h>

extern "C" {
#include <libavutil/hwcontext_d3d11va.h>
}

class D3D11VARenderer : public IFFmpegRenderer
{
public:
    D3D11VARenderer(int decoderSelectionPass);
    virtual ~D3D11VARenderer() override;
    virtual bool initialize(PDECODER_PARAMETERS params) override;
    virtual bool prepareDecoderContext(AVCodecContext* context, AVDictionary**) override;
    virtual bool prepareDecoderContextInGetFormat(AVCodecContext* context, AVPixelFormat pixelFormat) override;
    virtual void renderFrame(AVFrame* frame) override;
    virtual void notifyOverlayUpdated(Overlay::OverlayType) override;
    virtual int getRendererAttributes() override;
    virtual int getDecoderCapabilities() override;
    virtual bool needsTestFrame() override;
    virtual InitFailureReason getInitFailureReason() override;

    enum PixelShaders {
        GENERIC_YUV_420,
        BT_601_LIMITED_YUV_420,
        BT_2020_LIMITED_YUV_420,
        GENERIC_AYUV,
        GENERIC_Y410,
        _COUNT
    };

private:
    static void lockContext(void* lock_ctx);
    static void unlockContext(void* lock_ctx);

    bool setupRenderingResources();
    std::vector<DXGI_FORMAT> getVideoTextureSRVFormats();
    bool setupVideoTexture(); // for !m_BindDecoderOutputTextures
    bool setupTexturePoolViews(AVD3D11VAFramesContext* frameContext); // for m_BindDecoderOutputTextures
    void renderOverlay(Overlay::OverlayType type);
    void bindColorConversion(AVFrame* frame);
    void renderVideo(AVFrame* frame);
    bool checkDecoderSupport(IDXGIAdapter* adapter);
    bool createDeviceByAdapterIndex(int adapterIndex, bool* adapterNotFound = nullptr);

    int m_DecoderSelectionPass;
    int m_DevicesWithFL11Support;
    int m_DevicesWithCodecSupport;

    IDXGIFactory5* m_Factory;
    ID3D11Device* m_Device;
    IDXGISwapChain4* m_SwapChain;
    ID3D11DeviceContext* m_DeviceContext;
    ID3D11RenderTargetView* m_RenderTargetView;
    SDL_mutex* m_ContextLock;
    bool m_BindDecoderOutputTextures;

    DECODER_PARAMETERS m_DecoderParams;
    int m_TextureAlignment;
    DXGI_FORMAT m_TextureFormat;
    int m_DisplayWidth;
    int m_DisplayHeight;
    int m_LastColorSpace;
    bool m_LastFullRange;
    AVColorTransferCharacteristic m_LastColorTrc;

    bool m_AllowTearing;

    std::array<ID3D11PixelShader*, PixelShaders::_COUNT> m_VideoPixelShaders;
    ID3D11Buffer* m_VideoVertexBuffer;

    // Only valid if !m_BindDecoderOutputTextures
    ID3D11Texture2D* m_VideoTexture;

    // Only index 0 is valid if !m_BindDecoderOutputTextures
#define DECODER_BUFFER_POOL_SIZE 17
    ID3D11ShaderResourceView* m_VideoTextureResourceViews[DECODER_BUFFER_POOL_SIZE][2];

    SDL_SpinLock m_OverlayLock;
    ID3D11Buffer* m_OverlayVertexBuffers[Overlay::OverlayMax];
    ID3D11Texture2D* m_OverlayTextures[Overlay::OverlayMax];
    ID3D11ShaderResourceView* m_OverlayTextureResourceViews[Overlay::OverlayMax];
    ID3D11PixelShader* m_OverlayPixelShader;

    AVBufferRef* m_HwDeviceContext;
    AVBufferRef* m_HwFramesContext;
};

