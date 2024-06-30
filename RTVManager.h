#pragma once
#include <stdint.h>
#include <unordered_map>

class RTVManager final
{
public:
	static RTVManager& GetInstance();

	static int32_t CreateRenderTargetTexture(uint32_t width, uint32_t height);

	static void SetRenderTarget(int32_t textureHandle);

	static void SetRTtoBB();

	static void ResetResourceBarrier();

	static void ClearRTV(int32_t textureHandle);

private:
	std::unordered_map<int32_t, int32_t> rtvHandleMap;
	int32_t rtvIndex = 2;

	int32_t currentRenderTarget = -1;
};

