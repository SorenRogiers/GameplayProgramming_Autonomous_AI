#include "stdafx.h"
#include "House.h"
#include <cassert>

int House::m_NewValidID = 1;

void House::SetID(int value)
{
	assert((value >= m_NewValidID) && "<House::SetID>: invalid ID");
	m_ID = m_NewValidID;
	m_NewValidID = m_ID + 1;
}

void House::CreateHidingspots()
{
	float size = 5.f;
	//Create a hidingspot out of every house corner

	//hidingspot 01
	b2Vec2 hs1_TopLeft = { (m_Center.x - m_Size.x/2) , (m_Center.y + m_Size.y/2) };
	HidingSpot hs1 = {};
	hs1.size = size;
	hs1.center = { hs1_TopLeft.x + size / 2 , hs1_TopLeft.y - size / 2 };

	//hidingspot 02
	b2Vec2 hs2_TopLeft = { (m_Center.x + m_Size.x / 2) - size , (m_Center.y + m_Size.y / 2) };
	HidingSpot hs2 = {};
	hs2.size = size;
	hs2.center = { hs2_TopLeft.x + size / 2 , hs2_TopLeft.y - size / 2 };

	//hidingspot 03
	b2Vec2 hs3_TopLeft = { (m_Center.x - m_Size.x / 2) , (m_Center.y - m_Size.y / 2) + size };
	HidingSpot hs3 = {};
	hs3.size = size;
	hs3.center = { hs3_TopLeft.x + size / 2 , hs3_TopLeft.y - size / 2 };

	//hidingspot 04
	b2Vec2 hs4_TopLeft = { (m_Center.x + m_Size.x / 2) - size , (m_Center.y - m_Size.y / 2) + size };
	HidingSpot hs4 = {};
	hs4.size = size;
	hs4.center = { hs4_TopLeft.x + size / 2 , hs4_TopLeft.y - size / 2 };

	m_HouseCorners.push_back(hs1);
	m_HouseCorners.push_back(hs2);
	m_HouseCorners.push_back(hs3);
	m_HouseCorners.push_back(hs4);
}

void House::Update(float dt)
{
	if(m_IsChecked)
	{
		m_CheckTimer += dt;

		if (m_CheckTimer > m_ResetDelay)
		{
			m_IsChecked = false;
			m_CheckTimer = 0;
		}
	}
	
}
