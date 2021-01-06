#pragma once

#define NOMINMAX

#include <windows.h>
#include <string>
#include <xnamath.h>
#include <algorithm>

class Utilities
{
public:

	static void DebugLog(float f);
	static void DebugLog(float f, const char* varName);
	static void DebugLog(const char* text);
	static float RandValue(float min, float max);
	static float ClampValue(float val, float min, float max);
	static float GetDistance(XMFLOAT3 objA, XMFLOAT3 objB);
	static bool CheckDistance(XMFLOAT3 objA, XMFLOAT3 objB, float tolerance);
	static float MaxValue(float a, float b);
	static float MinValue(float a, float b);
	static float LerpValue(float a, float b, float time);
	static XMFLOAT3 LerpVectors(XMFLOAT3 objA, XMFLOAT3 objB, float time);
};