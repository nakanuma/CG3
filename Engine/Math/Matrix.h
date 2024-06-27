#pragma once
#include "Float2.h"
#include "Float3.h"
#include "Float4.h"

class Matrix
{
public:
	float r[4][4];

	///
	/// Constructors
	///

	// 単位行列で初期化
	Matrix();

	// floatを16個で初期化
	Matrix(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33);

	///
	/// Operators
	/// 

	Matrix operator - () const;

	Matrix operator + (const Matrix& m) const;
	Matrix operator - (const Matrix& m) const;
	Matrix operator * (const Matrix& m) const;

	Matrix& operator += (const Matrix& m);
	Matrix& operator -= (const Matrix& m);
	Matrix& operator *= (const Matrix& m);

	/// 
	/// Functions
	/// 

	static Matrix Identity();

	static Matrix Inverse(Matrix m);

	static Matrix PerspectiveFovLH(float fov, float aspectRatio, float nearZ, float farZ);

	static Matrix Orthographic(float width, float height, float nearClip, float farClip);

	static Matrix Scaling(Float3 scale);

	static Matrix Translation(Float3 translation);

	static Matrix RotationX(float rad);
	static Matrix RotationY(float rad);
	static Matrix RotationZ(float rad);

	static Matrix Pitch(float rad);
	static Matrix Yaw(float rad);
	static Matrix Roll(float rad);

	static Matrix RotationRollPitchYaw(float roll, float pitch, float yaw);
};

struct TransformationMatrix {
	Matrix WVP;
	Matrix World;
};