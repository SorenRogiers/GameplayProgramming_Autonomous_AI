#ifndef BEHAVIOURS_H
#define BEHAVIOURS_H
#include "BehaviourTree.h"
#include "Grid.h"
#include "House.h"
#include "Item.h"
//ENEMIES
//*******
inline bool IsNotRunning(Blackboard* pBlackboard)
{
	bool isRunning = false;
	auto dataAvailable = pBlackboard->GetData("RunMode", isRunning);

	if (!dataAvailable)
		return false;

	if (!isRunning)
		return true;

	return false;
}
inline bool HasWeapon(Blackboard* pBlackboard)
{
	std::vector<Item*> inventory = {};

	auto dataAvailable = pBlackboard->GetData("Inventory", inventory);

	if (!dataAvailable)
		return false;

	for (size_t i =0;i<inventory.size();++i)
	{
		if (inventory[i] != nullptr)
		{
			if (inventory[i]->GetItemType() == eItemType::PISTOL)
			{
				return true;
			}
		}
	}
	return false;
}
inline bool EnemyInFov(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> enemiesInFov = {};

	auto dataAvailable = pBlackboard->GetData("EnemiesInFov", enemiesInFov);

	if (enemiesInFov.size() > 0)
		return true;

	return false;
}

//USE ITEMS
//*********
inline bool IsHealthLow(Blackboard* pBlackboard)
{
	AgentInfo agent = {};
	auto dataAvailable = pBlackboard->GetData("AgentInfo", agent);

	if (!dataAvailable)
		return false;

	if (agent.Health <= 5)
		return true;

	return false;
}
inline bool IsHealthInInventory(Blackboard* pBlackboard)
{
	std::vector<Item*> inventory = {};

	auto dataAvailable = pBlackboard->GetData("Inventory", inventory);

	if (!dataAvailable)
		return false;

	for (size_t i =0;i<inventory.size();++i)
	{
		if(inventory[i] != nullptr)
		{
			if (inventory[i]->GetItemType() == eItemType::HEALTH)
			{
				pBlackboard->ChangeData("HealthIndex", int(i));
				return true;
			}
		}
		
	}
	return false;
}
inline bool IsEnergyInInventory(Blackboard* pBlackboard)
{
	std::vector<Item*> inventory = {};

	auto dataAvailable = pBlackboard->GetData("Inventory", inventory);

	if (!dataAvailable)
		return false;

	for (size_t i = 0; i<inventory.size(); ++i)
	{
		if (inventory[i] != nullptr)
		{
			if (inventory[i]->GetItemType() == eItemType::FOOD)
			{
				pBlackboard->ChangeData("FoodIndex", int(i));
				return true;
			}
		}
	}
		
	return false;
}
inline bool IsEnergyLow(Blackboard* pBlackboard)
{
	AgentInfo agent = {};
	auto dataAvailable = pBlackboard->GetData("AgentInfo", agent);

	if (!dataAvailable)
		return false;

	if (agent.Energy <= 11)
		return true;

	return false;
}

//INVENTORY-PICKUP
//****************
inline bool HasItemTarget(Blackboard* pBlackboard)
{
	bool itemTarget = false;
	auto dataAvailable = pBlackboard->GetData("ItemTargetSet", itemTarget);

	if (!dataAvailable)
		return false;

	return itemTarget;
}
inline bool HasInventoryFreeslot(Blackboard* pBlackboard)
{
	int hasFreeInvSlot = -1;
	auto dataAvailable = pBlackboard->GetData("HasFreeInvSlot", hasFreeInvSlot);

	if (!dataAvailable)
		return false;

	if (hasFreeInvSlot != -1)
		return true;

	return false;
}
inline bool HasItemsToPickup(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> items;
	auto dataAvailable = pBlackboard->GetData("Items", items);

	if (!dataAvailable)
		return false;

	if (items.size() > 0)
		return true;

	return false;
}
inline bool IsItemInFov(Blackboard* pBlackboard)
{
	TargetInformation target = {};
	AgentInfo agent = {};
	std::vector<EntityInfo> entitiesInFov = {};

	auto dataAvailable = pBlackboard->GetData("AgentInfo", agent) &&
		pBlackboard->GetData("ItemTarget", target) &&
		pBlackboard->GetData("EntitiesInFov",entitiesInFov);

	if (!dataAvailable)
		return false;

	for (auto e : entitiesInFov)
		if (e.Position == target.position)
			return true;

	return false;
}
inline bool IsItemInGrabRange(Blackboard* pBlackboard)
{
	TargetInformation target = {};
	AgentInfo agent ={};

	auto dataAvailable = pBlackboard->GetData("AgentInfo", agent) &&
		pBlackboard->GetData("ItemTarget", target);

	if (!dataAvailable)
		return false;

	float grabRange = agent.GrabRange;
	float distanceSquared = (target.position - agent.Position).LengthSquared();

	if (distanceSquared < grabRange * grabRange)
		return true;

	return false;
}

//HOUSES
//******
inline bool HasHouseTarget(Blackboard* pBlackboard)
{
	bool hasHouseTarget = false;
	auto dataAvailable = pBlackboard->GetData("HouseTargetSet", hasHouseTarget);

	if (!dataAvailable)
		return false;

	return hasHouseTarget;
}
inline bool HasHouses(Blackboard* pBlackboard)
{
	std::vector<House*> housesList = {};

	auto dataAvailable = pBlackboard->GetData("Houses", housesList);

	if (!dataAvailable)
		return false;

	if (housesList.size() > 0)
		return true;

	return false;
}
inline bool IsInsideHouse(Blackboard* pBlackboard)
{
	AgentInfo agent = {};

	auto dataAvailable = pBlackboard->GetData("AgentInfo", agent);

	if (!dataAvailable)
		return false;

	if (agent.IsInHouse)
		return true;

	return false;
}
inline bool IsHouseChecked(Blackboard* pblackboard)
{
	AgentInfo agent = {};
	TargetInformation target = {};
	std::vector<House*> houseList = {};

	auto dataAvailable = pblackboard->GetData("AgentInfo", agent) &&
		pblackboard->GetData("HouseTarget",target) &&
		pblackboard->GetData("Houses",houseList);

	if (!dataAvailable)
		return false;

	for(auto h : houseList)
	{
		if (h->GetCenter() == target.position && h->IsChecked())
			return true;
	}

	return false;
}
inline bool HasHousesToCheck(Blackboard* pBlackboard)
{
	std::vector<House*> houseList = {};
	auto dataAvailable = pBlackboard->GetData("Houses", houseList);

	if (!dataAvailable)
		return false;

	for (auto h : houseList)
		if (!h->IsChecked())
			return true;

	return false;
}

//ACTIONS
//*******
inline BehaviourState SetEnemyTarget(Blackboard* pBlackboard)
{
	std::vector<EntityInfo> enemiesInFov = {};
	AgentInfo agent = {};
	TargetInformation enemyTarget = {};
	float closestdistance = (std::numeric_limits<float>::max)();
	int enemyIndex = 0;

	auto dataAvailable = pBlackboard->GetData("EnemiesInFov", enemiesInFov) &&
		pBlackboard->GetData("AgentInfo", agent);

	if (!dataAvailable)
		return Failure;

	//Find the closest enemy and set it as target
	if(enemiesInFov.size() > 0)
	{
		for(size_t i =0;i<enemiesInFov.size();++i)
		{
			auto distanceSquared = (enemiesInFov[i].Position - agent.Position).LengthSquared();
			if(distanceSquared < closestdistance * closestdistance)
			{
				closestdistance = distanceSquared;
				enemyIndex = i;
			}
		}
	}
	enemyTarget.position = enemiesInFov[enemyIndex].Position;

	if (pBlackboard->ChangeData("EnemyTarget", enemyTarget) && pBlackboard->ChangeData("EnemyTargetSet", true))
		return Success;
	
	return Failure;
}
inline BehaviourState FillHealth(Blackboard* pBlackboard)
{
	if (pBlackboard->ChangeData("FillHealth", true))
		return Success;

	return Failure;
}
inline BehaviourState FillEnergy(Blackboard* pBlackboard)
{
	if (pBlackboard->ChangeData("FillEnergy", true))
		return Success;

	return Failure;
}
inline BehaviourState GrabItem(Blackboard* pBlackboard)
{
	if (pBlackboard->ChangeData("IsGrabAction", true))
		return Success;

	return Failure;
}
inline BehaviourState SetItemTarget(Blackboard* pBlackboard)
{
	int closestItem = -1;
	float closestDistance = (std::numeric_limits<float>::max)();
	AgentInfo agent = {};
	std::vector<EntityInfo> entities = {};

	auto dataAvailable = pBlackboard->GetData("AgentInfo", agent) &&
		pBlackboard->GetData("Items", entities);

	if (!dataAvailable)
		return Failure;

	TargetInformation itemTarget = {};

	//Find the closest item and set it as a target
	for (size_t i = 0; i < entities.size(); ++i)
	{
		float distanceSquared = b2DistanceSquared(entities[i].Position, agent.Position);

		if (distanceSquared < closestDistance)
		{
			closestDistance = distanceSquared;
			itemTarget.position = entities[i].Position;
			closestItem = i;
		}
	}

	if (pBlackboard->ChangeData("ItemTarget", itemTarget) && pBlackboard->ChangeData("ItemTargetSet", true) && pBlackboard->ChangeData("ItemToGetIndex", closestItem))
		return Success;

	return Failure;
}
inline BehaviourState SetHouseTarget(Blackboard* pBlackboard)
{
	std::vector<House*> housesList = {};
	bool houseTargetSet = false;

	auto dataAvailable = pBlackboard->GetData("Houses", housesList) &&
		pBlackboard->GetData("HouseTargetSet",houseTargetSet);

	if (!dataAvailable)
		return Failure;

	TargetInformation houseTarget = {};

	for(auto h : housesList)
	{
		if (!h->IsChecked() && !houseTargetSet)
		{
			houseTarget.position = h->GetCenter();
			pBlackboard->ChangeData("HouseTarget", houseTarget);
			pBlackboard->ChangeData("HouseTargetSet", true);
		}
	}

	pBlackboard->ChangeData("TargetSet", false);

	return Success;
}
inline BehaviourState ContinueTraverse(Blackboard* pBlackboard)
{
	if (pBlackboard->ChangeData("TargetSet", true) && pBlackboard->ChangeData("HouseTargetSet", false))
		return Success;

	return Failure;
}
inline BehaviourState Traverse(Blackboard* pBlackboard)
{
	//Get blackboard info
	Grid* pGrid = nullptr;
	AgentInfo agent;
	int checkpoint = 0;
	int targetCellID = -1;
	bool targetSet = false;
	int startCellID = -1;

	auto dataAvailable = pBlackboard->GetData("AgentInfo", agent) &&
		pBlackboard->GetData("Checkpoint", checkpoint) &&
		pBlackboard->GetData("Grid", pGrid) &&
		pBlackboard->GetData("TargetSet", targetSet) &&
		pBlackboard->GetData("TargetCellID", targetCellID) &&
		pBlackboard->GetData("StartCellID", startCellID);

	if (!dataAvailable)
		return Failure;

	if (!pGrid)
		return Failure;

	//Traverse around the world
	TargetInformation target = {};

	auto pTargetCell = pGrid->FindCell(targetCellID);
	if (!pTargetCell->IsChecked() && !(pTargetCell->GetThreatLevel() == Cell::ThreatLevel::SEVERE))
	{
		target = pTargetCell->GetCheckpoints()[checkpoint];
		pBlackboard->ChangeData("Target", target);
		pBlackboard->ChangeData("TargetSet", true);

		float distance = b2DistanceSquared(target.position, agent.Position);

		if (b2DistanceSquared(target.position, agent.Position) < 10.f)
		{
			if (checkpoint < 1)
			{
				checkpoint++;
				pBlackboard->ChangeData("Checkpoint", checkpoint);
			}
			else
			{
				pTargetCell->SetChecked(true);
				pBlackboard->ChangeData("Checkpoint", 0);
				pBlackboard->ChangeData("TargetSet", false);
			}
		}
	}
	else
	{
		//Update the next cell to check
		if(targetCellID < int(pGrid->GetCells().size()))
		{
			targetCellID++;
			pBlackboard->ChangeData("TargetCellID", targetCellID);
		} else
		{
			targetCellID = 1;
			pBlackboard->ChangeData("TargetCellID", targetCellID);
		}

		//Reset IsChecked to start anew
		if(targetCellID == startCellID-1)
		{
			for (auto c : pGrid->GetCells())
				c->SetChecked(false);
		}
	}

	return Success;
}
#endif
