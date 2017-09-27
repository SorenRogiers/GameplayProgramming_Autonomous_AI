#pragma once
#include <string>
#include <vector>

struct Context;
union CheapVariant
{
	CheapVariant() {}

	//INT
	int iVal;
	CheapVariant(int val) { iVal = val; }
	operator int() const { return iVal; }

	//UINT
	UINT uiVal;
	CheapVariant(UINT val) { uiVal = val; }
	operator UINT() const { return uiVal; }

	//FLOAT
	float fVal;
	CheapVariant(float val) { fVal = val; }	
	operator float() const { return fVal; }		

	//BOOL
	bool bVal;
	CheapVariant(bool val) { bVal = val; }
	operator bool() const { return bVal; }
};

struct PluginOutput
{
	b2Vec2 LinearVelocity = { 0.f,0.f };
	float AngularVelocity = 0.f;
	bool AutoOrientate = true;
	bool RunMode = false;

	bool IsEmpty(float epsilon = 0.0001f) const
	{
		return (LinearVelocity.LengthSquared() + (AngularVelocity*AngularVelocity)) < (epsilon*epsilon);
	}
};
enum eEntityType
{
	ENEMY,
	ITEM
	//,...
	,_LASTTYPE = ENEMY //Stores the biggest value of enum type (int)
};

enum eItemType
{
	PISTOL, //Shoot Enemies (Ammo Depletion)
	HEALTH, //Increments Health
	FOOD, //Increments Energy
	GARBAGE //Just Garbage
	//,...
	,_LASTITEM = GARBAGE //Stores the biggest value of enum type (int)
};

struct EntityInfo
{
	eEntityType Type;
	int EntityHash;

	b2Vec2 Position;
};

struct EnemyInfo
{
	int EnemyHash;
	int Health;
};

struct ItemInfo
{
	eItemType Type;
	int ItemHash;
};

struct HouseInfo
{
	b2Vec2 Center;
	b2Vec2 Size;
};

struct WorldInfo
{
	b2Vec2 Center;
	b2Vec2 Dimensions;
};

struct AgentInfo
{
	float Stamina;
	float Health;
	float Energy;
	bool RunMode;
	float GrabRange;
	bool IsInHouse;
	bool Bitten;
	bool Death;

	float FOV_Angle;
	float FOV_Range;

	b2Vec2 LinearVelocity;
	float AngularVelocity;
	float CurrentLinearSpeed;
	b2Vec2 Position;
	float Orientation;
	float MaxLinearSpeed;
	float MaxAngularSpeed;
	float AgentSize;
};

struct GameDebugParams //Debuggin Purposes only (Ignored during release build)
{
	GameDebugParams(){}
	GameDebugParams(int enemyAmount, bool autoGrab, bool ignoreEnergy, bool godMode, bool overrideDifficulty = false, float difficulty = 0.5f)
	{
		EnemySpawnAmount = enemyAmount;
		AutoGrabClosestItem = autoGrab;
		IgnoreEnergy = ignoreEnergy;
		GodMode = godMode;
		Difficulty = difficulty;
		OverrideDifficulty = overrideDifficulty;
	}

	int EnemySpawnAmount = 20; //Amount of enemies to spawn
	bool AutoGrabClosestItem = false; //ITEM_GRAB auto selects the closest item
	bool IgnoreEnergy = false; //No energy depletion
	bool GodMode = false; //Enemies can't kill you
	float Difficulty = 0.5f; //Scales from 0 > ...
	bool OverrideDifficulty = false;
};

class IBehaviourPlugin
{
public:
	IBehaviourPlugin(GameDebugParams params);
	virtual ~IBehaviourPlugin();

	void UpdateInternal(float dt);
	void RenderInternal(float dt);

	virtual void Start() = 0;
	virtual PluginOutput Update(float dt) = 0;
	virtual void End() = 0;
	virtual void ExtendUI_ImGui() {};
	virtual void ProcessEvents(const SDL_Event& e){} //Debugging Only! [Deactivated in release build ;) ]

	//Callable Functions
	//******************

	//INVENTORY
	bool INVENTORY_AddItem(int slotId, ItemInfo item);
	bool INVENTORY_UseItem(int slotId);
	bool INVENTORY_RemoveItem(int slotId);
	bool INVENTORY_GetItem(int slotId, ItemInfo& item);
	int INVENTORY_GetCapacity() const;

	//WORLD INFO
	WorldInfo WORLD_GetInfo() const;

	//FOV (Field Of View)
	std::vector<EntityInfo> FOV_GetEntities() const;
	//Get all the houses you can see. If you are in a house, you only see the house you are standing in!
	//You need to exit the current house entirely to regain full vision (see all houses outside)
	std::vector<HouseInfo> FOV_GetHouses() const;

	//ITEM
	template<typename T>
	bool ITEM_GetMetadata(ItemInfo item, std::string category, T& val)
	{
		CheapVariant varVal;
		if(GetItemMeta(item, category, varVal))
		{
			val = (T)varVal;
			return true;
		}

		return false;
	}

	bool ITEM_Grab(EntityInfo entity, ItemInfo& item);

	//ENEMY
	bool ENEMY_GetInfo(EntityInfo entity, EnemyInfo& enemy);
	
	//MISC
	b2Vec2 NAVMESH_GetClosestPathPoint(b2Vec2 goal) const;
	AgentInfo AGENT_GetInfo() const;

	//DEBUG HELPERS (Only work during Debug Build ;) )
	b2Vec2 DEBUG_ConvertScreenPosToWorldPos(b2Vec2 screenPos);
	void DEBUG_LogMessage(std::string message, ...);
	void DEBUG_DrawPoint(const b2Vec2& p, float size, const b2Color& color);
	void DEBUG_DrawCircle(const b2Vec2& center, float radius, const b2Color& color);
	void DEBUG_DrawSegment(const b2Vec2& p1, const b2Vec2& p2, const b2Color& color);
	void DEBUG_DrawSolidCircle(const b2Vec2& center, float32 radius, const b2Vec2& axis, const b2Color& color);
	void DEBUG_DrawSolidPolygon(const b2Vec2* points, int count, const b2Color& color, float depth, bool triangulate = false);
	void DEBUG_DrawString(const b2Vec2& pw, const char* string, ...);

private:
	//Private/Internal Implementation
	class Impl;
	std::unique_ptr<Impl> _impl;

	bool GetItemMeta(ItemInfo item, std::string category, CheapVariant& val) const;
};

