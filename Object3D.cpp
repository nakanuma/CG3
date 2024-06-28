#include "Object3D.h"
#include "Camera.h"

Object3D::Object3D()
{
	transform_.translate = { 0.0f, 0.0f, 0.0f };
	transform_.rotate = { 0.0f, 0.0f, 0.0f };
	transform_.scale = { 1.0f, 1.0f, 1.0f };

	// 白を書き込む
	materialCB_.data_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	// ライティング有効化
	materialCB_.data_->enableLighting = true;
	// 単位行列で初期化
	materialCB_.data_->uvTransform = Matrix::Identity();
}

void Object3D::UpdateMatrix()
{
	Matrix worldMatrix = transform_.MakeAffineMatrix();
	Matrix viewMatrix = Camera::GetCurrent()->MakeViewMatrix();
	Matrix projectionMatrix = Camera::GetCurrent()->MakePerspectiveFovMatrix();
	Matrix worldViewProjectionMatrix = worldMatrix * viewMatrix * projectionMatrix;
	wvpCB_.data_->WVP = worldViewProjectionMatrix;
	wvpCB_.data_->World = worldMatrix;
}

void Object3D::Draw()
{
	DirectXBase* dxBase = DirectXBase::GetInstance();

	// commandListにVBVを設定
	dxBase->GetCommandList()->IASetVertexBuffers(0, 1, &model_->vertexBufferView);
	// マテリアルCBufferの場所を設定
	dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialCB_.resource_->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpCB_.resource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定（Textureの設定）
	TextureManager::SetDescriptorTable(2, dxBase->GetCommandList(), model_->material.textureHandle); // モデルデータに格納されたテクスチャを使用する
	// 描画を行う（DrawCall/ドローコール）
	dxBase->GetCommandList()->DrawInstanced(UINT(model_->vertices.size()), 1, 0, 0);
}

void Object3D::Draw(const int TextureHandle)
{
	DirectXBase* dxBase = DirectXBase::GetInstance();

	// commandListにVBVを設定
	dxBase->GetCommandList()->IASetVertexBuffers(0, 1, &model_->vertexBufferView);
	// マテリアルCBufferの場所を設定
	dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialCB_.resource_->GetGPUVirtualAddress());
	// wvp用のCBufferの場所を設定
	dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(1, wvpCB_.resource_->GetGPUVirtualAddress());
	// SRVのDescriptorTableの先頭を設定（Textureの設定）
	TextureManager::SetDescriptorTable(2, dxBase->GetCommandList(), TextureHandle); // 指定したテクスチャを使用する
	// 描画を行う（DrawCall/ドローコール）
	dxBase->GetCommandList()->DrawInstanced(UINT(model_->vertices.size()), 1, 0, 0);
}

void Object3D::DrawInstancing(StructuredBuffer<ParticleForGPU>& structuredBuffer, uint32_t numInstance)
{
	DirectXBase* dxBase = DirectXBase::GetInstance();

	// commandListにVBVを設定
	dxBase->GetCommandList()->IASetVertexBuffers(0, 1, &model_->vertexBufferView);
	// マテリアルCBufferの場所を設定
	dxBase->GetCommandList()->SetGraphicsRootConstantBufferView(0, materialCB_.resource_->GetGPUVirtualAddress());
	// instancing用のDataを読むためにStructuredBufferのSRVを設定する
	dxBase->GetCommandList()->SetGraphicsRootDescriptorTable(1, TextureManager::GetInstance().srvHeap_.GetGPUHandle(structuredBuffer.heapIndex_));
	// SRVのDescriptorTableの先頭を設定（Textureの設定）
	TextureManager::SetDescriptorTable(2, dxBase->GetCommandList(), model_->material.textureHandle); // モデルデータに格納されたテクスチャを使用する
	// 描画を行う（DrawCall/ドローコール）
	dxBase->GetCommandList()->DrawInstanced(UINT(model_->vertices.size()), numInstance, 0, 0);
}
