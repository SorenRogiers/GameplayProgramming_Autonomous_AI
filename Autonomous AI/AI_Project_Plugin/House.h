#ifndef HOUSE_H
#define HOUSE_H
#include <Box2D/Common/b2Math.h>
#include <vector>
#include "Helpers.h"

class House
{
public:
	House(int id, b2Vec2 center, b2Vec2 size) :m_Center(center), m_Size(size) { SetID(id); CreateHidingspots(); }
	virtual ~House() {};

	void Update(float dt);
	b2Vec2 GetCenter() const { return m_Center; }
	int GetID() const { return m_ID; }
	bool IsChecked() const { return m_IsChecked; }
	void SetIsChecked(bool value) { m_IsChecked = value; }

	std::vector<HidingSpot> GetHidingspots() const { return m_HouseCorners; }
private:
	b2Vec2 m_Center = {};
	b2Vec2 m_Size = {};

	bool m_IsChecked = false;
	
	int m_ID = 0;
	const int m_ResetDelay = 480;
	static int m_NewValidID;

	float m_CheckTimer = 0;

	std::vector<HidingSpot> m_HouseCorners = {};

private:
	void SetID(int value);
	void CreateHidingspots();
};
#endif