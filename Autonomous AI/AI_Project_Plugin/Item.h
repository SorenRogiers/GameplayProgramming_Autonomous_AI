#ifndef ITEM_H
#define ITEM_H
#include <Box2D/Common/b2Math.h>

class Item
{
public:
	
	Item(eItemType type,int itemHash, int health,int energy) :m_ItemType(type),m_ItemHash(itemHash), m_Health(health),m_Energy(energy) {};
	Item(eItemType type,int itemHash, int ammo, float dps, float range) :m_ItemType(type),m_ItemHash(itemHash), m_GunAmmo(ammo), m_Dps(dps), m_AttackRange(range) {};
	virtual ~Item() {};

	eItemType GetItemType() const { return m_ItemType; }
	int GetHealth() const { return m_Health; }
	int GetEnergy() const { return m_Energy; }
	int GetAmmo() const { return m_GunAmmo; }
	int GetItemHash() const { return m_ItemHash; }

	float GetDps() const { return m_Dps; }
	float GetAttackRange() const { return m_AttackRange; }

	void SetAmmo(int ammo) { m_GunAmmo = ammo; }
private:
	eItemType m_ItemType;

	int m_ItemHash = 0;
	int m_Health = 0;
	int m_Energy = 0;
	int m_GunAmmo = 0;
	float m_Dps = 0.f;
	float m_AttackRange = 0.f;
};
#endif