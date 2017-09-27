#pragma once
#pragma region //Third-Pary Includes
#include <Box2D/Box2D.h>
#include "Box2D\Common\b2Draw.h"
#include "Box2D\Common\b2Math.h"

#include <GL/gl3w.h>
#include <ImGui/imgui.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>

//BOX2D HELPERS
//*************
#define b2_pidiv2 1.570796326795f

inline b2Vec2 operator*(const b2Vec2& a, const b2Vec2& b)
{
	return{ a.x*b.x, a.y*b.y };
}

inline b2Vec2 operator*(const b2Vec2& a, float b)
{
	return{ a.x*b, a.y*b };
}

inline b2Vec2 operator/(const b2Vec2& a, float b)
{
	return{ a.x / b, a.y / b };
}

inline b2Vec2 operator*(const b2Vec2& a, const b2Mat22& m)
{
	return b2Vec2(m.ex.x * a.x + m.ey.x * a.y,
		m.ex.y * a.x + m.ey.y * a.y);
}

//MATH HELPERS
//************
inline float ToRadians(const float angle)
{
	return angle * (b2_pi / 180.f);
}

inline float randomFloat(float max = 1.f)
{
	return max * (float(rand()) / RAND_MAX);
}

inline float randomBinomial(float max = 1.f)
{
	return randomFloat(max) - randomFloat(max);
}

inline b2Vec2 randomVector2(float max = 1.f)
{
	return{ randomBinomial(max),randomBinomial(max) };
}

inline b2Vec2 OrientationToVector(float orientation)
{
	orientation -= b2_pi;
	orientation /= 2.f;
	return b2Vec2(cos(orientation), sin(orientation));
}

inline float GetOrientationFromVelocity(b2Vec2 velocity)
{
	if (velocity.Length() == 0)
		return 0.f;

	return atan2f(velocity.x, -velocity.y);
}

inline b2Vec2 Clamp(const b2Vec2& a, float max)
{
	auto scale = max / a.Length();
	scale = scale < 1.f ? scale : 1.f;
	return a * scale;
}

inline float Distance(b2Vec2 a, b2Vec2 b)
{
	return sqrt((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
}

inline b2Vec2 abs(const b2Vec2 a)
{
	return b2Vec2(abs(a.x), abs(a.y));
}

inline bool PointInTriangleBoundingBox(const b2Vec2& point, const b2Vec2& currentTip, const b2Vec2& previous, const b2Vec2& next)
{
	auto xMin = min(currentTip.x, min(previous.x, next.x)) - FLT_EPSILON;
	auto xMax = max(currentTip.x, max(previous.x, next.x)) + FLT_EPSILON;
	auto yMin = min(currentTip.y, min(previous.y, next.y)) - FLT_EPSILON;
	auto yMax = max(currentTip.y, max(previous.y, next.y)) + FLT_EPSILON;

	if (point.x < xMin || xMax < point.x || point.y < yMin || yMax < point.y)
		return false;
	return true;
}

inline float DistanceSquarePointToLine(const b2Vec2& p1, const b2Vec2& p2, const b2Vec2& point)
{
	//http://totologic.blogspot.be/2014/01/accurate-point-in-triangle-test.html
	auto p1p2_squareLength = (p2.x - p1.x)*(p2.x - p1.x) + (p2.y - p1.y)*(p2.y - p1.y);
	auto dp = ((point.x - p1.x)*(p2.x - p1.x) + (point.y - p1.y)*(p2.y - p1.y)) / p1p2_squareLength;
	if (dp < 0)
		return (point.x - p1.x) * (point.x - p1.x) + (point.y - p1.y) *  (point.y - p1.y);
	else if (dp <= 1)
	{
		auto pp1_squareLength = (p1.x - point.x)*(p1.x - point.x) + (p1.y - point.y) * (p1.y - point.y);
		return pp1_squareLength - dp * dp * p1p2_squareLength;
	}
	else
		return (point.x - p2.x)*(point.x - p2.x) + (point.y - p2.y) * (point.y - p2.y);
}

inline bool IsPointOnLine(const b2Vec2& origin, const b2Vec2& segmentEnd, const b2Vec2& point)
{
	auto line = segmentEnd - origin;
	line.Normalize();
	//Projection
	auto w = point - origin;
	auto proj = b2Dot(w, line);
	if (proj < 0) //Not on line
		return false;

	auto vsq = b2Dot(line, line);
	if (proj > vsq) //Not on line
		return false;

	return true;
}

inline bool PointInTriangle(const b2Vec2& point, const b2Vec2& currentTip, const b2Vec2& previous, const b2Vec2& next, bool onLineAllowed = false)
{
	//Do bounding box test first
	if (!PointInTriangleBoundingBox(point, currentTip, previous, next))
		return false;

	//Reference: http://www.blackpawn.com/texts/pointinpoly/default.html
	//Compute direction vectors
	auto v0 = previous - currentTip;
	auto v1 = next - currentTip;
	auto v2 = point - currentTip;

	//Compute dot products
	auto dot00 = b2Dot(v0, v0);
	auto dot01 = b2Dot(v0, v1);
	auto dot02 = b2Dot(v0, v2);
	auto dot11 = b2Dot(v1, v1);
	auto dot12 = b2Dot(v1, v2);

	// Compute barycentric coordinates
	auto invDenom = 1 / (dot00 * dot11 - dot01 * dot01);
	auto u = (dot11 * dot02 - dot01 * dot12) * invDenom;
	auto v = (dot00 * dot12 - dot01 * dot02) * invDenom;

	// Check if point is in triangle
	if (u < 0 || v < 0 || u > 1 || v > 1 || (u + v) > 1)
	{
		if (onLineAllowed)
		{
			//Check special case where these barycentric coordinates are not enough for on line detection!
			auto epsilonSquared = (FLT_EPSILON * FLT_EPSILON) * 2;
			if (DistanceSquarePointToLine(currentTip, next, point) <= FLT_EPSILON)
				return true;
			if (DistanceSquarePointToLine(next, previous, point) <= FLT_EPSILON)
				return true;
			if (DistanceSquarePointToLine(previous, currentTip, point) <= FLT_EPSILON)
				return true;
		}

		return false;
	}
	return true;
}