#pragma once

struct Float3
{
	float x;
	float y;
	float z;

	///
	/// 二項演算子のオーバーロード
	/// 
	
    // 加算
    Float3 operator+(const Float3& other) const
    {
        return { x + other.x, y + other.y, z + other.z };
    }

    // 減算
    Float3 operator-(const Float3& other) const
    {
        return { x - other.x, y - other.y, z - other.z };
    }

    // 右辺のスカラーと乗算
    Float3 operator*(float scalar) const
    {
        return { x * scalar, y * scalar, z * scalar };
    }

    // 左辺のスカラーと乗算
    friend Float3 operator*(float scalar, const Float3& vec)
    {
        return { vec.x * scalar, vec.y * scalar, vec.z * scalar };
    }

	///
	/// 複合代入演算子のオーバーロード
	/// 
    
    Float3& operator+=(const Float3& other)
    {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    Float3& operator-=(const Float3& other)
    {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }
};