#pragma once
#include <d3d12.h>
#include "externals/DirectXTex/DirectXTex.h"
#include "StringUtil.h"
#include "Logger.h"
#include "DescriptorHeap.h"
#include <array>

class TextureManager final
{
public:
	static void Initialize(ID3D12Device* device);

	static int Load(const std::string& filePath, ID3D12Device* device);

	static TextureManager& GetInstance();

	static void SetDescriptorTable(UINT rootParamIndex, ID3D12GraphicsCommandList* commandList, uint32_t textureHandle);

	static const DirectX::TexMetadata& GetMetaData(uint32_t textureHandle);

	static int CreateEmptyTexture(uint32_t width, uint32_t height);

	static ID3D12Resource* GetResource(int textureHandle);

	DescriptorHeap srvHeap_;
private:
	// TextureデータをCPUで読む
	static DirectX::ScratchImage LoadTexture(const std::string& filePath);
	// DirectX12のTextureResourceを作る
	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata, bool isRenderTarget = false);
	// TextureResourceにデータを転送する
	static void UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages);

	static const uint32_t kMaxTextureValue_ = 128;
	uint32_t index_ = 1;

	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kMaxTextureValue_> texResources;

	std::array<DirectX::TexMetadata, kMaxTextureValue_> texMetadata;
};

