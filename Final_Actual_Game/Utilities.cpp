#include "Utilities.h"

// Show float in console
void Utilities::DebugLog(float f)
{
	OutputDebugString(std::to_string(f).c_str());
	OutputDebugString("\n");
}

// Show float with name in console
void Utilities::DebugLog(float f, const char* varName)
{
	OutputDebugString(varName);
	OutputDebugString(std::to_string(f).c_str());
	OutputDebugString("\n");
}

// Show text in console
void Utilities::DebugLog(const char* text)
{
	OutputDebugString(text);
	OutputDebugString("\n");
}

// Returns a random value in specific range
float Utilities::RandValue(float min, float max)
{
	return min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
}

// Clamp between two values
float Utilities::ClampValue(float val, float min, float max)
{
	return std::max(max, std::min(val, min));
}

// Returns the distance between two objects
float Utilities::GetDistance(XMFLOAT3 objA, XMFLOAT3 objB) 
{
	return sqrtf(powf(objA.x - objB.x, 2) + powf(objA.y - objB.y, 2) + powf(objA.z - objB.z, 2));
}

// Check for a given distance between two objects
bool Utilities::CheckDistance(XMFLOAT3 objA, XMFLOAT3 objB, float tolerance) 
{
	if (GetDistance(objA, objB) <= tolerance) return true;

	return false;
}

// Return greatest value
float Utilities::MaxValue(float a, float b)
{
	return std::max(a, b);
}

// Return smallest value
float Utilities::MinValue(float a, float b)
{
	return std::min(a, b);
}

// Lerp between two values
float Utilities::LerpValue(float a, float b, float time)
{
	return (a + time * (b - a));
}

// Lerp between two vectors
XMFLOAT3 Utilities::LerpVectors(XMFLOAT3 objA, XMFLOAT3 objB, float time) 
{
	float x = LerpValue(objA.x, objB.x, time);
	float y = LerpValue(objA.y, objB.y, time);
	float z = LerpValue(objA.z, objB.z, time);
	return XMFLOAT3(x, y, z);
}
