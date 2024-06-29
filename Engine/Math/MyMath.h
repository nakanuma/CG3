#pragma once
#include "Float2.h"
#include "Float3.h"
#include "Float4.h"
#include "Matrix3x3.h"
#include "Matrix.h"
#include "Transform.h"

static constexpr double PI = 3.14159265359;

static constexpr float PIf = 3.14159265359f;

// AABB
struct AABB {
	Float3 min;
	Float3 max;
};

// AABBと点の衝突判定
static bool IsCollision(const AABB& aabb, const Float3& point)
{
    // 点がAABBの範囲内にあるかどうかを判定
    if (point.x >= aabb.min.x && point.x <= aabb.max.x &&
        point.y >= aabb.min.y && point.y <= aabb.max.y &&
        point.z >= aabb.min.z && point.z <= aabb.max.z) {
        return true;
    }

    return false;
}