#include "stdafx.h"
#include "TestBoxPlugin.h"
#include "Grid.h"
#include "SteeringBehaviours.h"
#include "SteeringPipeline.h"
#include "Behaviours.h"
#include "Helpers.h"
#include "House.h"
#include "Item.h"
#include <iostream>

TestBoxPlugin::TestBoxPlugin():IBehaviourPlugin(GameDebugParams(20,false,false,false,false,.5f))
{
}

TestBoxPlugin::~TestBoxPlugin()
{
}

void TestBoxPlugin::Start()
{
	//CREATE GRID
	//***********
	m_pGrid = new Grid(WORLD_GetInfo());
	m_pGrid->CreateGrid(4);

	//BEHAVIOURS
	//**********
	m_pSeekBehaviour = new SeekBehaviour();
	m_pObstacleAvoidance = new ObstacleAvoidanceBehaviour();
	m_pWanderBehaviour = new WanderBehaviour();

	BehaviourAndWeight seekWeight = BehaviourAndWeight(m_pSeekBehaviour, 1.f);
	BehaviourAndWeight avoidWeight = BehaviourAndWeight(m_pObstacleAvoidance, 0.f);

	m_pBlendedSteeringBehaviour = new BlendedSteeringBehaviour({ seekWeight,avoidWeight });

	//STEERING PIPELINE
	//*****************
	m_pTargeter = new Targeter();
	m_pActuator = new Actuator(m_pBlendedSteeringBehaviour);
	m_pDecomposer = new Decomposer(this);

	m_pSteeringpipeline = new SteeringPipeline();
	m_pSteeringpipeline->SetTargeters({ m_pTargeter });
	m_pSteeringpipeline->SetActuators(m_pActuator);
	m_pSteeringpipeline->SetDecomposers({ m_pDecomposer });
	m_pSteeringpipeline->SetFallBackBehaviour(m_pWanderBehaviour);

	//Agent
	m_pAgent = AGENT_GetInfo();
	m_Health = m_pAgent.Health;

	//Fill inventory with 5 empty items
	m_Inventory.resize(m_InventorySize);

	//BLACKBOARD
	//**********
	auto pBlackboard = new Blackboard();
	pBlackboard->AddData("AgentInfo", AGENT_GetInfo());
	pBlackboard->AddData("Target", m_Target);
	pBlackboard->AddData("HouseTarget", m_Target);
	pBlackboard->AddData("ItemTarget", m_Target);
	pBlackboard->AddData("EnemyTarget", m_Target);
	pBlackboard->AddData("TargetSet", false);
	pBlackboard->AddData("HouseTargetSet", false);
	pBlackboard->AddData("ItemTargetSet", false);
	pBlackboard->AddData("EnemyTargetSet", false);
	pBlackboard->AddData("Checkpoint", 0);
	pBlackboard->AddData("Grid", m_pGrid);
	pBlackboard->AddData("Cells", m_pGrid->GetCells());
	pBlackboard->AddData("StartCellID", m_pGrid->FindCellID(m_pAgent.Position));
	pBlackboard->AddData("TargetCellID", m_pGrid->FindCellID(m_pAgent.Position));
	pBlackboard->AddData("Houses", m_HousesList);
	pBlackboard->AddData("Items", m_Items);
	pBlackboard->AddData("ItemToGetIndex", -1);
	pBlackboard->AddData("HasFreeInvSlot", -1);
	pBlackboard->AddData("IsGrabAction", false);
	pBlackboard->AddData("Inventory", m_Inventory);
	pBlackboard->AddData("EnemiesInFov", m_EnemiesInFov);
	pBlackboard->AddData("EntitiesInFov", FOV_GetEntities());
	pBlackboard->AddData("FillHealth", false);
	pBlackboard->AddData("FillEnergy", false);
	pBlackboard->AddData("HealthIndex", -1);
	pBlackboard->AddData("FoodIndex", -1);
	pBlackboard->AddData("RunMode", m_IsRunModeOn);

	//BEHAVIOURTREE
	m_pBehaviourTree = new BehaviourTree(pBlackboard,
		new BehaviourSelector
		({
			//Fill health
			new BehaviourSequence
			({
				new BehaviourConditional(IsHealthLow),
				new BehaviourConditional(IsHealthInInventory),
				new BehaviourAction(FillHealth)
			}),
			//Fill Energy
			new BehaviourSequence
			({
				new BehaviourConditional(IsEnergyLow),
				new BehaviourConditional(IsEnergyInInventory),
				new BehaviourAction(FillEnergy)
			}),
			//Deal with enemies
			new BehaviourSelector
			({
				new BehaviourSequence
				({
					new BehaviourConditional(EnemyInFov),
					new BehaviourConditional(HasWeapon),
					new BehaviourConditional(IsNotRunning),
					new BehaviourAction(SetEnemyTarget),
				})
			}),
			//Pickup items
			new BehaviourSelector
			({
				new BehaviourSequence
				({
					new BehaviourConditional(HasItemsToPickup),
					new BehaviourConditional(HasInventoryFreeslot),
					new BehaviourSelector
					({
						new BehaviourSequence
						({
							new BehaviourConditional(HasItemTarget),
							new BehaviourConditional(IsItemInGrabRange),
							new BehaviourConditional(IsItemInFov),
							new BehaviourAction(GrabItem)
						}),
						new BehaviourAction(SetItemTarget),
					})
				}),
			}),
			//Search houses part
			new BehaviourSelector
			({
				new BehaviourSequence
				({
					new BehaviourConditional(HasHouses),
					new BehaviourSelector
					({
						new BehaviourSequence({
							new BehaviourConditional(IsInsideHouse),
							new BehaviourConditional(IsHouseChecked),
							new BehaviourAction(Traverse)
						}),
						new BehaviourSequence({
							new BehaviourConditional(HasHousesToCheck),
							new BehaviourAction(SetHouseTarget)
						})
					})
				})
			}),
			//Traverse around the world
			new BehaviourSelector
			({
				new BehaviourSequence
				({
					new BehaviourAction(Traverse)
				})
			})
		})
	);
}

PluginOutput TestBoxPlugin::Update(float dt)
{
	//FPS & TIME ALIVE
	m_Fps = int((1.0f / dt));
	m_AliveTimer += dt;

	//DRAWING
	//*******
	DrawGrid();
	DrawHouses();

	//UPDATE AGENTINFO
	//****************
	m_pAgent = AGENT_GetInfo();

	//RUN FOR A COUPLE OF SECONDS AFTER WE'VE BEEN BITTEN
	float tempHealth = m_pAgent.Health;
	if(m_Health != tempHealth)
	{
		m_Health = tempHealth;
		m_IsRunModeOn = true;
	}

	if(m_IsRunModeOn)
	{
		m_RunTimer += dt;
		if (m_RunTimer > m_RuntimerReset)
		{
			m_RunTimer = 0.f;
			m_IsRunModeOn = false;
		}
	}

	//UPDATE BLACKBOARD AND BEHAVIOUR TREE
	//************************************
	auto pBlackboard = m_pBehaviourTree->GetBlackboard();
	pBlackboard->ChangeData("AgentInfo", m_pAgent);
	pBlackboard->ChangeData("Houses", m_HousesList);
	pBlackboard->ChangeData("HasFreeInvSlot", FindFreeInventorySlot());
	pBlackboard->ChangeData("Items", m_Items);
	pBlackboard->ChangeData("EnemiesInFov", m_EnemiesInFov);
	pBlackboard->ChangeData("EntitiesInFov", FOV_GetEntities());
	pBlackboard->ChangeData("RunMode", m_IsRunModeOn);

	if (m_pBehaviourTree)
		m_pBehaviourTree->Update();

	//STORE AND UPDATE HOUSES / ITEMS
	//*******************************
	StoreEntities();

	RememberHouses();
	for (auto h : m_HousesList)
		h->Update(dt);

	for (auto c : m_pGrid->GetCells())
		c->Update(dt);

	if (m_EnemiesInFov.size() > 1)
	{
		auto pCurrentCell = m_pGrid->FindCell(m_pAgent.Position);
		pCurrentCell->SetThreatLevel(Cell::ThreatLevel::MODERATE);
	}
	else if (m_EnemiesInFov.size() > 2)
	{
		auto pCurrentCell = m_pGrid->FindCell(m_pAgent.Position);
		pCurrentCell->SetThreatLevel(Cell::ThreatLevel::SEVERE);
	}

	//TARGETING & REASSIGNING TARGETS
	//*******************************
	bool enemyTargetSet = false;
	bool itemTargetSet = false;
	bool houseTargetSet = false;
	bool targetSet = false;

	pBlackboard->GetData("TargetSet", targetSet);
	pBlackboard->GetData("HouseTargetSet", houseTargetSet);
	pBlackboard->GetData("ItemTargetSet", itemTargetSet);
	pBlackboard->GetData("EnemyTargetSet", enemyTargetSet);

	if (enemyTargetSet)
	{
		pBlackboard->GetData("EnemyTarget", m_Target);

		//Find the gun slot id in the inventory
		int gunIndex = FindGunInInventory();

		//GunIndex == -1 probably means we are fleeing
		if (gunIndex != -1)
		{
			auto range = m_Inventory[gunIndex]->GetAttackRange();
			auto currentAmmo = m_Inventory[gunIndex]->GetAmmo();

			CreateShootFOV(range);
			bool isEnemyInShootFov = IsPointInPolygon(m_Target.position, m_ShootFov);

			if (isEnemyInShootFov)
			{
				//Provide fire delay
				m_FireTimer += dt;
				if (m_FireTimer > m_FireDelay)
				{
					//Shoot
					if (INVENTORY_UseItem(gunIndex))
					{
						m_FireTimer = 0;
						currentAmmo--;
						if (currentAmmo <= 0)
						{
							INVENTORY_RemoveItem(gunIndex);
							SafeDelete(m_Inventory[gunIndex]);
						}
						else
						{
							m_Inventory[gunIndex]->SetAmmo(currentAmmo);
						}
						pBlackboard->ChangeData("Inventory", m_Inventory);

					}
					pBlackboard->ChangeData("EnemyTargetSet", false);
				}
			}
			if(m_EnemiesInFov.size() == 0)
				pBlackboard->ChangeData("EnemyTargetSet", false);

		}
		else
		{
			auto distance = b2DistanceSquared(m_Target.position, m_pAgent.Position);
			if (distance > (2.f * 2.f))
			{
				pBlackboard->ChangeData("EnemyTargetSet", false);
			}
		}
	}
	else if (itemTargetSet) //Reached the item so we mark our itemtarget false again
	{
		pBlackboard->GetData("ItemTarget", m_Target);
	}
	else if (houseTargetSet) //If we reached our housetarget we set it to false again and mark the house checked
	{
		pBlackboard->GetData("HouseTarget", m_Target);
		auto distance = b2DistanceSquared(m_Target.position, m_pAgent.Position);

		if (distance < m_TargetRange * m_TargetRange)
		{
			for (auto h : m_HousesList)
			{
				if (h->GetCenter() == m_Target.position)
				{
					h->SetIsChecked(true);
					auto pTargetCell = m_pGrid->FindCell(m_pAgent.Position);
					pTargetCell->SetChecked(true);
				}
			}
			pBlackboard->ChangeData("HouseTargetSet", false);
		}
	}
	else if (targetSet) //If we reached our world target reset it
	{
		pBlackboard->GetData("Target", m_Target);
		auto distance = b2DistanceSquared(m_Target.position, m_pAgent.Position);
		if (distance < m_TargetRange * m_TargetRange)
			pBlackboard->ChangeData("TargetSet", false);
	}

	//PICKUP ITEMS
	//************
	bool grabAction = false;
	pBlackboard->GetData("IsGrabAction", grabAction);
	PickupItem(grabAction);


	//USE MEDKITS AND FOOD
	//********************
	bool fillHealth = false;
	bool fillEnergy = false;

	pBlackboard->GetData("FillHealth", fillHealth);
	pBlackboard->GetData("FillEnergy", fillEnergy);

	if (fillHealth)
	{
		int healthIndex = -1;
		pBlackboard->GetData("HealthIndex", healthIndex);

		if (INVENTORY_UseItem(healthIndex))
		{
			INVENTORY_RemoveItem(healthIndex);
			std::cout << "Used item: MedKit - Removed from slotID: " << healthIndex << std::endl;

			SafeDelete(m_Inventory[healthIndex]);
			pBlackboard->ChangeData("Inventory", m_Inventory);
			pBlackboard->ChangeData("FillHealth", false);
		}
	}

	if (fillEnergy)
	{
		int foodIndex = -1;
		pBlackboard->GetData("FoodIndex", foodIndex);

		if (INVENTORY_UseItem(foodIndex))
		{
			INVENTORY_RemoveItem(foodIndex);
			std::cout << "Used item: ENERGY - Removed from slotID: " << foodIndex << std::endl;

			SafeDelete(m_Inventory[foodIndex]);
			pBlackboard->ChangeData("Inventory", m_Inventory);
			pBlackboard->ChangeData("FillEnergy", false);
		}
	}

	//REMOVE / USE EXCESSIVE ITEMS - 3 FOOD - 2 PISTOLS - 1 MEDKIT
	BalanceIventory();

	//UPDATE TARGETER
	DEBUG_DrawPoint(m_Target.position, 7.5f, { 0,0,0,1 });
	m_pTargeter->GetGoalRef().isPositionSet = true;
	m_pTargeter->GetGoalRef().position = m_Target.position;

	//CREATE OBSTACLES AND ADJUST OUR BLENDED BEHAVIOUR
	CreateObstaclesFromEnemies();

	//Output
	PluginOutput output = {};
	output = m_pSteeringpipeline->CalculateSteering(dt, &m_pAgent);
	output.RunMode = m_IsRunModeOn;
	return output;
}

//Extend the UI [ImGui call only!]
void TestBoxPlugin::ExtendUI_ImGui()
{
	ImGui::Text("FPS: %i", m_Fps);
	ImGui::Text("Time alive (sec): %.0lf", m_AliveTimer);
	ImGui::Text("Total items: %i", m_TotalItems);
	ImGui::Text("Total Pistols: %i", m_TotalPistols);
	ImGui::Text("Total MedKits: %i", m_TotalMedKits);
	ImGui::Text("Total Food: %i", m_TotalFood);
	ImGui::Text("Total Garbage: %i", m_TotalGarbage);
}
void TestBoxPlugin::End()
{
	delete m_pBehaviourTree;
	delete m_pGrid;

	delete m_pSeekBehaviour;
	delete m_pObstacleAvoidance;
	delete m_pBlendedSteeringBehaviour;
	delete m_pWanderBehaviour;

	delete m_pSteeringpipeline;
	delete m_pTargeter;
	delete m_pDecomposer;
	delete m_pActuator;

	for (auto h : m_HousesList)
		SafeDelete(h);

	m_HousesList.clear();

	for (auto i : m_Inventory)
		SafeDelete(i);

	m_Inventory.clear();
}
//[Optional]> For Debugging
void TestBoxPlugin::ProcessEvents(const SDL_Event& e)
{
}

//Split up into methods to avoid cluttered reading
void TestBoxPlugin::RememberHouses()
{
	auto housesList = FOV_GetHouses();
	int newId = m_HousesList.size() + 1;

	if(housesList.size() > 0)
	{
		for(auto h : housesList)
		{
			auto findHouse = [&](const House* house) {return house->GetCenter() == h.Center; };
			if(!(std::find_if(m_HousesList.begin(),m_HousesList.end(),findHouse) != m_HousesList.end()))
			{
				auto newHouse = new House(newId,h.Center,h.Size);
				m_HousesList.push_back(newHouse);
			}
		}
	}
}
void TestBoxPlugin::StoreEntities()
{
	auto entityList = FOV_GetEntities();
	m_EnemiesInFov.clear();
	if(entityList.size() > 0)
	{
		for(auto e:entityList)
		{
			if(e.Type == eEntityType::ITEM)
			{
				auto findItem = [&](const EntityInfo& entity) {return e.Position == entity.Position; };
				if(!(std::find_if(m_Items.begin(), m_Items.end(),findItem) != m_Items.end()))
				{
					m_Items.push_back(e);
				}
			} 
			else if (e.Type == eEntityType::ENEMY)
			{
				auto findEnemy = [&](const EntityInfo& entity) {return e.EntityHash == entity.EntityHash; };
				if (!(std::find_if(m_EnemiesInFov.begin(), m_EnemiesInFov.end(), findEnemy) != m_EnemiesInFov.end()))
				{
					m_EnemiesInFov.push_back(e);
				}
			}
		}
	}
}
void TestBoxPlugin::PickupItem(bool GrabItem)
{
	if(GrabItem)
	{
		auto pBlackboard = m_pBehaviourTree->GetBlackboard();
		int itemIndex = -1;
		ItemInfo item = {};

		pBlackboard->GetData("ItemToGetIndex", itemIndex);

		if(ITEM_Grab(m_Items[itemIndex],item))
		{
			m_TotalItems++;
			int freeSlot = FindFreeInventorySlot();

			if (freeSlot == -1)
				return;

			switch (item.Type)
			{
			case PISTOL:
			{
				INVENTORY_AddItem(freeSlot, item);
				std::cout << "Pick up item: PISTOL" << std::endl;
				m_TotalPistols++;
				int ammo = 0;
				float dps = 0.f;
				float attackRange = 0.f;
				ITEM_GetMetadata(item, "ammo", ammo);
				ITEM_GetMetadata(item, "dps", dps);
				ITEM_GetMetadata(item, "range", attackRange);

				auto newItem = new Item(item.Type, item.ItemHash, ammo, dps, attackRange);
				m_Inventory[freeSlot] = newItem;

				break;
			}
			case HEALTH:
			{
				INVENTORY_AddItem(freeSlot, item);
				std::cout << "Pick up item: HEALTH" << std::endl;
				m_TotalMedKits++;
				int health=0;
				ITEM_GetMetadata(item, "health", health);

				auto newItem = new Item(item.Type, item.ItemHash, health,0);
				m_Inventory[freeSlot] = newItem;

				break;
			}	
			case FOOD:
			{
				INVENTORY_AddItem(freeSlot, item);
				std::cout << "Pick up item: FOOD" << std::endl;
				m_TotalFood++;
				int energy =0;
				ITEM_GetMetadata(item, "energy", energy);

				auto newItem = new Item(item.Type, item.ItemHash, 0,energy);
				m_Inventory[freeSlot] = newItem;

				break;
			}
			case GARBAGE:
			{	
				std::cout << "Did not pickup item: GARBAGE" << std::endl;
				INVENTORY_AddItem(freeSlot, item);
				INVENTORY_RemoveItem(freeSlot);
				m_TotalGarbage++;
				break;
			}
			default:
				break;
			}

			pBlackboard->ChangeData("Inventory", m_Inventory);

			//Remove the item we picked up from our explored item list
			if (m_Items.size() > 0)
				m_Items.erase(m_Items.begin() + itemIndex);
		}
		pBlackboard->ChangeData("IsGrabAction", false);
		pBlackboard->ChangeData("ItemTargetSet", false);
	}
}
void TestBoxPlugin::BalanceIventory()
{
	//If one of the items in the inventory reaches the max allowance limit remove it
	//Keep the one with the most value though
	if(FindFreeInventorySlot() == -1)
	{
		int foodAmount = 0;
		int healthAmount = 0;
		int pistolAmount = 0;

		std::vector<std::pair<int, int>> pistolInfo = {};	//Vector of pairs - the first int is the index of the pistol & the second int is the amount of ammo the pistol has
		std::vector<std::pair<int, int>> foodInfo = {};		//Vector of pairs - the first int is the index of the food & the second int is the amount of energy the food gives
		std::vector<std::pair<int, int>> healthInfo = {};	//Vector of pairs - the first int is the index of the medkit & the second int is the amount it heals

		int lowestAmmo = (std::numeric_limits<int>::max)();
		int lowestEnergy = (std::numeric_limits<int>::max)();
		int lowestHealth = (std::numeric_limits<int>::max)();

		int pistolIndex = -1;
		int foodIndex = -1;
		int healthIndex = -1;

		for (size_t i = 0; i<m_Inventory.size(); ++i)
		{
			if (m_Inventory[i] != nullptr)
			{
				if (m_Inventory[i]->GetItemType() == eItemType::PISTOL)
				{
					pistolAmount++;
					pistolInfo.push_back(std::make_pair(i, m_Inventory[i]->GetAmmo()));
				}
				else if (m_Inventory[i]->GetItemType() == eItemType::FOOD)
				{
					foodAmount++;
					foodInfo.push_back(std::make_pair(i, m_Inventory[i]->GetEnergy()));
				}
				else if (m_Inventory[i]->GetItemType() == eItemType::HEALTH)
				{
					healthAmount++;
					healthInfo.push_back(std::make_pair(i, m_Inventory[i]->GetHealth()));
				}
			}

		}

		if (pistolAmount>m_MaxPistols)
		{
			//Compare ammo's and remove lowest ammo
			for (size_t i = 0; i<pistolInfo.size(); ++i)
			{
				if (pistolInfo[i].second < lowestAmmo)
				{
					lowestAmmo = pistolInfo[i].second;
					pistolIndex = pistolInfo[i].first;
				}
			}
			std::cout << "Removed excessive item: PISTOL - Ammo:" << m_Inventory[pistolIndex]->GetAmmo() << std::endl;
			INVENTORY_RemoveItem(pistolIndex);
			SafeDelete(m_Inventory[pistolIndex]);
		}
		else if (foodAmount > m_MaxEnergy)
		{
			//Compoare energy and remove the lowest one
			for (size_t i = 0; i<foodInfo.size(); ++i)
			{
				if (foodInfo[i].second < lowestEnergy)
				{
					lowestEnergy = foodInfo[i].second;
					foodIndex = foodInfo[i].first;
				}
			}
			std::cout << "used excessive item: FOOD - Energy:" << m_Inventory[foodIndex]->GetEnergy() << std::endl;
			INVENTORY_UseItem(foodIndex);
			INVENTORY_RemoveItem(foodIndex);
			SafeDelete(m_Inventory[foodIndex]);
		}
		else if (healthAmount > m_MaxHealth)
		{
			//Compoare energy and remove the lowest one
			for (size_t i = 0; i<healthInfo.size(); ++i)
			{
				if (healthInfo[i].second < lowestHealth)
				{
					lowestHealth = healthInfo[i].second;
					healthIndex = healthInfo[i].first;
				}
			}

			std::cout << "Used excessive item: MedKit - health:" << m_Inventory[healthIndex]->GetHealth() << std::endl;
			INVENTORY_UseItem(healthIndex);
			INVENTORY_RemoveItem(healthIndex);
			SafeDelete(m_Inventory[healthIndex]);
		}
	}
}
void TestBoxPlugin::CreateShootFOV(float range)
{
	m_ShootFov.clear();

	float topOffset = 0.07f;
	float botOffset = 0.6f;
	float bottomRange = 1.f;

	float orientation = m_pAgent.Orientation;

	b2Vec2 topRight = { m_pAgent.Position.x + (sin(topOffset + orientation)*range), m_pAgent.Position.y - (cos(topOffset + orientation)*range) };
	b2Vec2 topLeft = { m_pAgent.Position.x + (sin((-topOffset)+orientation)*range), m_pAgent.Position.y - (cos((-topOffset)+orientation)*range) };

	b2Vec2 botRight = { m_pAgent.Position.x + (sin(botOffset + orientation)*bottomRange), m_pAgent.Position.y - (cos(botOffset + orientation)*bottomRange) };
	b2Vec2 botLeft = { m_pAgent.Position.x + (sin(-botOffset + orientation)*bottomRange), m_pAgent.Position.y - (cos(-botOffset + orientation)*bottomRange) };

	m_ShootFov.push_back(m_pAgent.Position);
	m_ShootFov.push_back(botLeft);
	m_ShootFov.push_back(botRight);
	m_ShootFov.push_back(topLeft);
	m_ShootFov.push_back(topRight);

	//DRAW THE ShootFOV
	DEBUG_DrawPoint(topLeft, 5.f, { 1,1,0,1 });
	DEBUG_DrawPoint(topRight, 5.f, { 1,1,0,1 });
	DEBUG_DrawPoint(botLeft, 5.f, { 1,1,0,1 });
	DEBUG_DrawPoint(botRight, 5.f, { 1,1,0,1 });

	DEBUG_DrawSegment(m_pAgent.Position, botLeft, { 1,1,0,1 });
	DEBUG_DrawSegment(botLeft, topLeft, { 1,1,0,1 });
	DEBUG_DrawSegment(topLeft, topRight, { 1,1,0,1 });
	DEBUG_DrawSegment(topRight, botRight, { 1,1,0,1 });
	DEBUG_DrawSegment(botRight, m_pAgent.Position, { 1,1,0,1 });
}
void TestBoxPlugin::CreateObstaclesFromEnemies()
{
	std::vector<Obstacle> obstacles;
	for (auto e : m_EnemiesInFov)
	{
		Obstacle obstacle = {};
		obstacle.position = e.Position;
		obstacle.radius = 3.5f;
		obstacles.push_back(obstacle);
	}

	if (obstacles.size() >0)
	{
		if (FindGunInInventory() == -1)
		{
			m_pObstacleAvoidance->UpdateObstacles(obstacles);
			m_pObstacleAvoidance->SetMaxAvoidanceForce(50.f);

			m_pBlendedSteeringBehaviour->m_weightedBehavioursVec[0].SetNewWeight(.85f);
			m_pBlendedSteeringBehaviour->m_weightedBehavioursVec[1].SetNewWeight(.15f);
			m_IsRunModeOn = true;
		}
	}
}
int TestBoxPlugin::FindFreeInventorySlot()
{
	for(int i = 0;i<m_InventorySize;++i)
	{
		if(m_Inventory[i] == nullptr)
			return i;
	}
	return -1;
}
int TestBoxPlugin::FindGunInInventory()
{
	int index = -1;
	for (size_t i = 0; i<m_Inventory.size(); ++i)
	{
		if (m_Inventory[i] != nullptr)
		{
			if (m_Inventory[i]->GetItemType() == eItemType::PISTOL)
			{
				index = i;
				break;
			}
		}
	}
	return index;
}
bool TestBoxPlugin::IsPointInPolygon(b2Vec2 point, std::vector<b2Vec2> polygon)
{
	if (polygon.size() < 3)
		return false;

	int i, j, nvert = polygon.size();

	bool inPoly = false;

	for(i = 0,j=nvert-1;i<nvert;j=i++)
	{
		if(((polygon[i].y >= point.y) != (polygon[j].y >= point.y)) &&
			(point.x <= (polygon[j].x - polygon[i].x) * (point.y - polygon[i].y) / (polygon[j].y-polygon[i].y) + polygon[i].x))
		{
			inPoly = !inPoly;
		}
	}
	return inPoly;
}

//DEBUG DRAWING
//*************
void TestBoxPlugin::DrawGrid()
{
	if (m_pGrid)
	{
		float offset = 0.2f;
		b2Color color = { 1.f,0.6f,0.f,1.f };
		auto pBlackboard = m_pBehaviourTree->GetBlackboard();

		for (size_t i = 0; i<m_pGrid->GetCells().size(); ++i)
		{
			//Change color depending on safety
			if (m_pGrid->GetCells()[i]->GetThreatLevel() == Cell::ThreatLevel::LOW)
				color = { 0.f,1.f,0.f,1.f };
			else if(m_pGrid->GetCells()[i]->GetThreatLevel() == Cell::ThreatLevel::MODERATE)
				color = { 1.f,0.6f,0.f,1.f };
			else if(m_pGrid->GetCells()[i]->GetThreatLevel() == Cell::ThreatLevel::SEVERE)
				color = { 1.f,0.f,0.f,1.f };

			//Draw Border
			float top = m_pGrid->GetCells()[i]->GetCenter().y + m_pGrid->GetCells()[i]->GetCellHeight() / 2 - offset;
			float bottom = m_pGrid->GetCells()[i]->GetCenter().y - m_pGrid->GetCells()[i]->GetCellHeight() / 2 + offset;
			float left = m_pGrid->GetCells()[i]->GetCenter().x - m_pGrid->GetCells()[i]->GetCellWidth() / 2 + offset;
			float right = m_pGrid->GetCells()[i]->GetCenter().x + m_pGrid->GetCells()[i]->GetCellWidth() / 2 - offset;

			DEBUG_DrawSegment({ left,top }, { right,top }, color);
			DEBUG_DrawSegment({ left,bottom }, { right,bottom }, color);
			DEBUG_DrawSegment({ left,top }, { left,bottom }, color);
			DEBUG_DrawSegment({ right,top }, { right,bottom }, color);

			//Draw Checkpoints
			for (size_t j = 0; j<m_pGrid->GetCells()[i]->GetCheckpoints().size(); ++j)
			{
				DEBUG_DrawSolidCircle(m_pGrid->GetCells()[i]->GetCheckpoints()[j], 2.f, { 0,0 }, { 1,1,1,1 });
			}
		}
	}
}
void TestBoxPlugin::DrawHouses()
{
	b2Color color = { .0f,.6f,0.f,1.f };

	for (size_t i = 0; i < m_HousesList.size(); ++i)
	{
		if (m_HousesList[i]->IsChecked())
			color = { .0f,.6f,0.f,1.f };
		else
			color = { .4f,0.f,0.f,1.f };

		DEBUG_DrawSolidCircle(m_HousesList[i]->GetCenter(), 5.f, { 0,0 }, color);
	}
}
void TestBoxPlugin::DrawHidingSpots()
{
	b2Color color = { 0.f,0.0f,1.f,1.f };
	float offset = 0.1f; //for a clearer view of the spots

	for(size_t i = 0;i<m_HousesList.size();++i)
	{
		std::vector<HidingSpot> hidingspots = m_HousesList[i]->GetHidingspots();

		for(auto hs : hidingspots)
		{
			//draw the center point
			DEBUG_DrawPoint(hs.center, 5.f, color);
			//draw the borders
			float top = { hs.center.y + hs.size/2 +offset };
			float bottom = { hs.center.y - hs.size/2 - offset };
			float left = { hs.center.x - hs.size/2 - offset };
			float right = { hs.center.x + hs.size/2 +offset };

			DEBUG_DrawSegment({ left,top }, { right,top }, color);
			DEBUG_DrawSegment({ left,bottom }, { right,bottom }, color);
			DEBUG_DrawSegment({ left,top }, { left,bottom }, color);
			DEBUG_DrawSegment({ right,top }, { right,bottom }, color);
		}
	}
}
