#pragma once
#include "Transform.h"
#include "ModelManager.h"
#include "TextureManager.h"
#include "ConstBuffer.h"
#include "StructuredBuffer.h"

struct Material {
	Float4 color;
	int32_t enableLighting;
	float padding[3];
	Matrix uvTransform;
};

class Object3D
{
public:
	Object3D();

	// マトリックス情報の更新
	void UpdateMatrix();

	// 描画（モデル内のテクスチャを参照 / テクスチャを指定して描画）
	void Draw();

	void Draw(const int TextureHandle);

	void DrawInstancing(StructuredBuffer<TransformationMatrix>& structuredBuffer);

	// マテリアルの定数バッファ
	ConstBuffer<Material>materialCB_;

	// トランスフォームの定数バッファ
	ConstBuffer<TransformationMatrix>wvpCB_;

	// モデル情報
	ModelData* model_ = nullptr;

	// トランスフォーム情報
	Transform transform_;
};

