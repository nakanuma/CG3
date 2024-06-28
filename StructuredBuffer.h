#pragma once
#include "DirectXUtil.h"
#include "DirectXBase.h"
#include "DescriptorHeap.h"
#include "TextureManager.h"

template<class Type>class StructuredBuffer {
public:
	// trueを指定した場合には空で生成される
	StructuredBuffer(uint32_t numInstance, bool isEmpty = false) : numMaxInstance_(numInstance) { 
		if (!isEmpty)Create(); 
	};

	void Create() {
		// リソースを作る
		resource_ = CreateBufferResource(DirectXBase::GetInstance()->GetDevice(), sizeof(Type) * numMaxInstance_);
		// データを書き込む
		data_ = nullptr;
		// 書き込むためのアドレスを取得
		resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
		// DescriptorHeapの生成
		/*heap_.Create(DirectXBase::GetInstance()->GetDevice(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, kNumInstance, true);*/

		CreateSRV();
	};

	void CreateSRV();

	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	Type* data_;
	/*DescriptorHeap heap_;*/

	// コピー不可にする
	StructuredBuffer(const StructuredBuffer&) = delete;
	StructuredBuffer(StructuredBuffer&&) = delete;
	StructuredBuffer& operator=(const StructuredBuffer&) = delete;
	StructuredBuffer& operator=(StructuredBuffer&&) = delete;


	uint32_t numMaxInstance_; // インスタンス数
	uint32_t heapIndex_;
};

template<class Type>
inline void StructuredBuffer<Type>::CreateSRV()
{
	D3D12_SHADER_RESOURCE_VIEW_DESC instancingSrvDesc{};
	instancingSrvDesc.Format = DXGI_FORMAT_UNKNOWN;
	instancingSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	instancingSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	instancingSrvDesc.Buffer.FirstElement = 0;
	instancingSrvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
	instancingSrvDesc.Buffer.NumElements = numMaxInstance_;
	instancingSrvDesc.Buffer.StructureByteStride = sizeof(Type);
	D3D12_CPU_DESCRIPTOR_HANDLE instancingSrvHandleCPU = TextureManager::GetInstance().srvHeap_.GetCPUHandle(TextureManager::GetIndex());
	heapIndex_ = TextureManager::GetIndex(); // heapのIndexを記録
	TextureManager::IncrementIndex(); // indexをインクリメント
	DirectXBase::GetInstance()->GetDevice()->CreateShaderResourceView(resource_.Get(), &instancingSrvDesc, instancingSrvHandleCPU);
}