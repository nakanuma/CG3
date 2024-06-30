#include "TextureManager.h"
#include <cassert>
#include "DirectXBase.h"

void TextureManager::Initialize(ID3D12Device* device)
{
	GetInstance().srvHeap_.Create(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 128, true);
}

int TextureManager::Load(const std::string& filePath, ID3D12Device* device)
{
	// テクスチャ読み込みの最大値に達した場合、ログを出力
	if (GetInstance().index_ >= kMaxTextureValue_) {
		Log(std::format("Maximum texture loading has been reached. Texture:{}\n", filePath));
		assert(0);
	}

	// Textureを読んで転送する
	DirectX::ScratchImage mipImages = GetInstance().LoadTexture(filePath);
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

	// メタデータの配列に保存
	GetInstance().texMetadata[GetInstance().index_] = mipImages.GetMetadata();

	// リソースの配列に保存
	Microsoft::WRL::ComPtr<ID3D12Resource>& targetResource = GetInstance().texResources[GetInstance().index_];
	targetResource = TextureManager::CreateTextureResource(device, metadata);

	TextureManager::GetInstance().UploadTextureData(targetResource.Get(), mipImages);

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// SRVの生成
	device->CreateShaderResourceView(targetResource.Get(), &srvDesc, GetInstance().srvHeap_.GetCPUHandle(GetInstance().index_));

	// SRVを作成するDescriptorHeapの場所を決める
	GetInstance().index_++;

	// 実際に返すのはインクリメントする前の値なので1引いて返す
	return GetInstance().index_ - 1;
}

TextureManager& TextureManager::GetInstance()
{
	static TextureManager instance;

	return instance;
}

void TextureManager::SetDescriptorTable(UINT rootParamIndex, ID3D12GraphicsCommandList* commandList, uint32_t textureHandle)
{
	commandList->SetGraphicsRootDescriptorTable(rootParamIndex, GetInstance().srvHeap_.GetGPUHandle(textureHandle));
}

const DirectX::TexMetadata& TextureManager::GetMetaData(uint32_t textureHandle)
{
	return GetInstance().texMetadata[textureHandle];
}

int TextureManager::CreateEmptyTexture(uint32_t width, uint32_t height)
{
	// テクスチャ読み込みの最大値に達した場合、ログを出力
	if (GetInstance().index_ >= kMaxTextureValue_) {
		Log(std::format("Maximum texture loading has been reached.\n"));
		assert(0);
	}

	// メタデータを自分で作成する
	DirectX::TexMetadata metadata;
	metadata.width = width;
	metadata.height = height;
	metadata.format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	metadata.mipLevels = 1;
	metadata.arraySize = 1;
	metadata.dimension = DirectX::TEX_DIMENSION_TEXTURE2D;

	// メタデータの配列に保存
	GetInstance().texMetadata[GetInstance().index_] = metadata;

	// リソースの配列に保存
	Microsoft::WRL::ComPtr<ID3D12Resource>& targetResource = GetInstance().texResources[GetInstance().index_];
	targetResource = TextureManager::CreateTextureResource(DirectXBase::GetInstance()->GetDevice(), metadata, true);

	// metaDataを基にSRVの設定
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	srvDesc.Format = metadata.format;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D; // 2Dテクスチャ
	srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

	// SRVの生成
	DirectXBase::GetInstance()->GetDevice()->CreateShaderResourceView(targetResource.Get(), &srvDesc, GetInstance().srvHeap_.GetCPUHandle(GetInstance().index_));

	// SRVを作成するDescriptorHeapの場所を決める
	GetInstance().index_++;

	// 実際に返すのはインクリメントする前の値なので1引いて返す
	return GetInstance().index_ - 1;
}

ID3D12Resource* TextureManager::GetResource(int textureHandle)
{
	return GetInstance().texResources[textureHandle].Get();
}

DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath)
{
	HRESULT result = S_FALSE;

	// テクスチャファイルを読み込んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	result = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	assert(SUCCEEDED(result));

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	result = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	assert(SUCCEEDED(result));

	// ミップマップ付きのデータを返す
	return mipImages;
}

Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(ID3D12Device* device, const DirectX::TexMetadata& metadata, bool isRenderTarget)
{
	HRESULT result = S_FALSE;

	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数

	// 利用するHeapの設定
	D3D12_HEAP_PROPERTIES heapProperties{};
	if (!isRenderTarget) {
		heapProperties.Type = D3D12_HEAP_TYPE_CUSTOM; // 細かい設定を行う
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_WRITE_BACK; // WriteBackポリシーでCPUアクセス可能
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_L0; // プロセッサの近くに配置
	} else {
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET;
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // 細かい設定を行う
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN; // WriteBackポリシーでCPUアクセス可能
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN; // プロセッサの近くに配置
	}

	D3D12_CLEAR_VALUE clearValue = {
		DXGI_FORMAT_R8G8B8A8_UNORM_SRGB,
		{0.1f, 0.25f, 0.5f, 1.0f}
	};

	// Resourceを生成する
	Microsoft::WRL::ComPtr<ID3D12Resource> resource = nullptr;
	result = device->CreateCommittedResource(
		&heapProperties, // Heapの設定
		isRenderTarget ? D3D12_HEAP_FLAG_ALLOW_ALL_BUFFERS_AND_TEXTURES : D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定
		&resourceDesc, // Resourceの設定
		isRenderTarget ? D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE : D3D12_RESOURCE_STATE_GENERIC_READ, // 初回のResourceState
		isRenderTarget ? &clearValue : nullptr, // Clear最適値
		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(result));

	return std::move(resource);
}

void TextureManager::UploadTextureData(ID3D12Resource* texture, const DirectX::ScratchImage& mipImages)
{
	HRESULT result = S_FALSE;

	// Meta情報を取得
	const DirectX::TexMetadata& metadata = mipImages.GetMetadata();
	// 全MipMapについて
	for (size_t mipLevel = 0; mipLevel < metadata.mipLevels; ++mipLevel) {
		// MipMapLevelを指定して各Imageを取得
		const DirectX::Image* img = mipImages.GetImage(mipLevel, 0, 0);
		// Textureに転送
		result = texture->WriteToSubresource(
			UINT(mipLevel),
			nullptr, // 全領域へコピー
			img->pixels, // 元データアドレス
			UINT(img->rowPitch), // 1ラインサイズ
			UINT(img->slicePitch) // 1枚サイズ
		);
		assert(SUCCEEDED(result));
	}
}
