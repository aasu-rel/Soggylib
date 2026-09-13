#pragma once
#include <cstdint>
#include <string>
 
// Most of the types should be in a Sexy:: namespace
// I'm not gonna wrap them now though

// types with special properties and methods should be available in their own headers

// popcap uses std::string, but SexyString is more flexible
// either is fine
typedef std::string SexyString;

typedef uint8_t byte;
typedef unsigned short ushort;
typedef unsigned int uint;
typedef unsigned long ulong;

// idfk i found this in Board's buildSymbols (hypermodern)
// its probably just a float
typedef float pvztime_t;

struct SexyVector2
{
	float x, y;

	SexyVector2() : x(0), y(0) {}

	SexyVector2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
};

struct SexyVector3
{
	float x, y, z;

	SexyVector3() : x(0), y(0), z(0) {}

	SexyVector3(float x, float y, float z)
	{
		this->x = x;
		this->y = y;
		this->z = z;
	}
};

struct Rect
{
	int mX, mY, mWidth, mHeight;

	Rect() : mX(0), mY(0), mWidth(0), mHeight(0) {}

	Rect(int x, int y, int width, int height)
	{
		mX = x;
		mY = y;
		mWidth = width;
		mHeight = height;
	}
};

struct FRect
{
	float mX, mY, mWidth, mHeight;

	FRect() : mX(0), mY(0), mWidth(0), mHeight(0) {}

	FRect(float x, float y, float width, float height)
	{
		mX = x;
		mY = y;
		mWidth = width;
		mHeight = height;
	}
};

struct Point
{
	int mX;
	int mY;

	Point() {};

	Point(int x, int y) : mX(x), mY(y) {};
};

struct FPoint
{
	float mX;
	float mY;

	FPoint() {};

	FPoint(float x, float y) : mX(x), mY(y) {};
};

struct ValueRange
{
	float Min;
	float Max;

	ValueRange() : Min(0), Max(0) {};

	ValueRange(float Min, float Max) : Min(Min), Max(Max) {};
};