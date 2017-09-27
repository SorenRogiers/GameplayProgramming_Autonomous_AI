#pragma once
#include "IBehaviourPlugin.h"
#include "Helpers.h"
#include "BehaviourTree.h"
#include "SteeringBehaviours.h"
#include "SteeringPipeline.h"
#include "CombinedBehaviours.h"

class Item;
class Grid;
class Cell;
class House;

class TestBoxPlugin : public IBehaviourPlugin
{
public:
	TestBoxPlugin();
	~TestBoxPlugin();

	void Start() override;
	PluginOutput Update(float dt) override;
	void ExtendUI_ImGui() override;
	void End() override;
	void ProcessEvents(const SDL_Event& e) override;

	void DrawGrid();						//Draw the grid that divides the world
	void DrawHouses();						//Draw the center of the houses color changes when they get checked / unchecked
	void DrawHidingSpots();					//Draw the hiding spots that can be used by the agents aka the corners of a house with a radius
	void RememberHouses();					//Stores the houses when we see them
	void StoreEntities();					//Stores the items & enemies when we see them in their respective vectors
	void BalanceIventory();					//Avoid having too much of everything in our inventory
	void PickupItem(bool GrabItem);			//Pickup the item and store it in our inventory
	void CreateShootFOV(float range = 20);	//I want the enemy to be in this small shape and range before we shoot our pistol
	void CreateObstaclesFromEnemies();		//Create an obstacle from an enemy in our fov to avoid it when we dont have any pistols

	int FindFreeInventorySlot();			//returns the next free space in our inventory if we dont find any return -1
	int FindGunInInventory();				//Find the a gun in the inventory and return the slotid if we don't find any return -1
	
	bool IsPointInPolygon(b2Vec2 point, std::vector<b2Vec2> polygon);	//Using this to determine if the enemy is inside or shootFOV to fire
protected:
	AgentInfo m_pAgent						= {};		//Hold info about our agent
	TargetInformation m_Target				= {};		//Hold info about our current target
	
	std::vector<House*> m_HousesList		= {};		//Collects all the houses we've seen thus far
	std::vector<EntityInfo> m_Items			= {};		//Collects all the items we've seen thus far
	std::vector<EntityInfo> m_EnemiesInFov	= {};		//Collects the enemies which are currently inside our fov
	std::vector<Item*> m_Inventory			= {};		//Stores the items we are currently holding
	std::vector<b2Vec2> m_ShootFov			= {};		//Vector that holds the positions of our shootFOV

	int m_CurrentCheckpoint		= 0;	
	int m_Fps					= 0;
	int m_TotalItems			= 0;	
	int m_TotalPistols			= 0;
	int m_TotalFood				= 0;
	int m_TotalMedKits			= 0;
	int m_TotalGarbage			= 0;

	float m_Health				= 0.f;	
	float m_FireTimer			= 0.f;
	float m_AliveTimer			= 0.f;
	float m_RunTimer			= 0.f;

	const int m_InventorySize	= 5;	
	const int m_MaxHealth		= 1;
	const int m_MaxEnergy		= 3;
	const int m_MaxPistols		= 2;

	const float m_RuntimerReset	= 4.f;
	const float m_FireDelay		= .1f;
	const float m_TargetRange	= 1.f;	
	
	bool m_IsRunModeOn			= false;

	Grid* m_pGrid							= nullptr;	//Split the world up into a grid
	BehaviourTree* m_pBehaviourTree			= nullptr;

	//BEHAVIOURS
	WanderBehaviour* m_pWanderBehaviour						= nullptr;
	SeekBehaviour* m_pSeekBehaviour							= nullptr;
	ObstacleAvoidanceBehaviour* m_pObstacleAvoidance		= nullptr;
	BlendedSteeringBehaviour* m_pBlendedSteeringBehaviour	= nullptr;

	//STEERING PIPELINE PARTS
	SteeringPipeline* m_pSteeringpipeline	= nullptr;
	Targeter* m_pTargeter					= nullptr;
	Actuator* m_pActuator					= nullptr;
	Decomposer* m_pDecomposer				= nullptr;
};

