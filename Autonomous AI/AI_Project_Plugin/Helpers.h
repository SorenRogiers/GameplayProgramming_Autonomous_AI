#ifndef HELPERS_H
#define HELPERS_H
#include <Box2D/Common/b2Math.h>

//OBSTACLES
//*********
struct Obstacle
{
	b2Vec2 position ={};
	float radius = 0.f;

	bool IsEmpty() const
	{
		return (position == b2Vec2_zero && radius == 0.f);
	}
};

//HIDINGSPOT
//*********
struct HidingSpot
{
	b2Vec2 center ={};
	float size = 0.f;
};

//TARGETINFORMATION
//*****************
struct TargetInformation
{
	b2Vec2 position = { 0.0f,0.0f };
	float Orientation = 0.0f;

	b2Vec2 linearVelocity = { 0.f,0.f };
	float angularVelocity = 0.0f;

	TargetInformation(b2Vec2 position = b2Vec2_zero, float orientation = 0.f, b2Vec2 linearVel = b2Vec2_zero, float angularVel = 0.f) :
		position(position),
		Orientation(orientation),
		linearVelocity(linearVel),
		angularVelocity(angularVel) {}
};

//SAFEDELETE
//**********
template< class T > void SafeDelete(T*& pVal)
{
	if (pVal != nullptr)
	{
		delete pVal;
		pVal = nullptr;
	}
}
#endif
