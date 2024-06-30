#include "RTVManager.h"
#include "TextureManager.h"
#include "DirectXBase.h"

RTVManager& RTVManager::GetInstance()
{
	static RTVManager instance;

	return instance;
}

int32_t RTVManager::CreateRenderTargetTexture(uint32_t width, uint32_t height)
{
	// 空のテクスチャを作成
	int32_t emptyTexture = TextureManager::CreateEmptyTexture(width, height);
	// TextureHandleとRTVHandleを対応させる
	GetInstance().rtvHandleMap[emptyTexture] = GetInstance().rtvIndex;

	// テクスチャに対してレンダーターゲットを作成
	DirectXBase::GetInstance()->GetDevice()->CreateRenderTargetView(
		TextureManager::GetResource(emptyTexture), nullptr,
		DirectXBase::GetInstance()->GetRTVHeap()->GetCPUHandle(GetInstance().rtvIndex)
	);

	// 深度テクスチャの作成(あとで)


	GetInstance().rtvIndex++;

	return emptyTexture;
}

void RTVManager::SetRenderTarget(int32_t textureHandle)
{
	DirectXBase* dxBase = DirectXBase::GetInstance();

	// 元のレンダーターゲットのリソースバリアを戻す
	ResetResourceBarrier();

	// リソースバリアを書き込み可能な状態にする
	dxBase->barrier_.Transition.pResource = TextureManager::GetResource(textureHandle);
	dxBase->barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	dxBase->barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	dxBase->GetCommandList()->ResourceBarrier(1, &dxBase->barrier_);

	// レンダーターゲットをセットする
	auto cpuHandle = dxBase->GetRTVHeap()->GetCPUHandle(GetInstance().rtvHandleMap[textureHandle]);
	auto dsvHandle = dxBase->dsvDescriptorHeap_.GetCPUHandle(0);

	dxBase->GetCommandList()->OMSetRenderTargets(1, &cpuHandle, false, &dsvHandle);

	// 現在のレンダーターゲットを保存する
	GetInstance().currentRenderTarget = textureHandle;
}

void RTVManager::SetRTtoBB()
{
	DirectXBase* dxBase = DirectXBase::GetInstance();

	// 元のレンダーターゲットのリソースバリアを戻す
	ResetResourceBarrier();

	// リソースバリアを書き込み可能な状態にする

	UINT backBufferIndex = dxBase->swapChain_->GetCurrentBackBufferIndex();

	// バリアを張る対象のリソース。現在のバックバッファに対して行う
	dxBase->barrier_.Transition.pResource = dxBase->swapChainResources_[backBufferIndex].Get();
	// 遷移前（現在）のResourceState
	dxBase->barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	// 遷移後のResourceState
	dxBase->barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	// TransitionBarrierを張る
	dxBase->GetCommandList()->ResourceBarrier(1, &dxBase->barrier_);

	// 描画先のRTVをとDSVを設定する
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dxBase->dsvDescriptorHeap_.GetCPUHandle(0);

	// レンダーターゲットをセットする
	dxBase->GetCommandList()->OMSetRenderTargets(1, &dxBase->rtvHandles_[backBufferIndex], false, &dsvHandle);

	// 現在のレンダーターゲットを保存する
	GetInstance().currentRenderTarget = -1;
}

void RTVManager::ResetResourceBarrier()
{
	int32_t rt = GetInstance().currentRenderTarget;
	DirectXBase* dxBase = DirectXBase::GetInstance();

	if (rt < 0) {
		// バックバッファのリソースバリアを戻す
		UINT backBufferIndex = dxBase->swapChain_->GetCurrentBackBufferIndex();

		dxBase->barrier_.Transition.pResource = dxBase->swapChainResources_[backBufferIndex].Get();
		dxBase->barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		dxBase->barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		dxBase->GetCommandList()->ResourceBarrier(1, &dxBase->barrier_);
	} else {
		// レンダーテクスチャのリソースバリアを戻す
		dxBase->barrier_.Transition.pResource = TextureManager::GetResource(rt);
		dxBase->barrier_.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		dxBase->barrier_.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		dxBase->GetCommandList()->ResourceBarrier(1, &dxBase->barrier_);
	}
}

void RTVManager::ClearRTV(int32_t textureHandle)
{
	DirectXBase* dxBase = DirectXBase::GetInstance();

	// 指定した色で画面全体をクリアする
	float clearColor[] = { 0.1f, 0.25f, 0.5f, 1.0f };
	dxBase->GetCommandList()->ClearRenderTargetView(
		dxBase->GetRTVHeap()->GetCPUHandle(GetInstance().rtvHandleMap[textureHandle]), clearColor, 0, nullptr);
}
