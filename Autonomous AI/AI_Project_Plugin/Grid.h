#ifndef GRID_H
#define GRID_H

#include <Box2D/Common/b2Math.h>
#include "IBehaviourPlugin.h"
#include "Helpers.h"

class Cell;

//**********
//GRID CLASS
//**********
class Grid
{
public:
	Grid(WorldInfo worldInfo):m_WorldInfo(worldInfo){}
	virtual ~Grid()
	{
		for (auto c : m_Cells)
			SafeDelete(c);

		m_Cells.clear();
	}

	void CreateGrid(float size);
	
	std::vector<Cell*> GetCells() const { return m_Cells; }
	
	Cell* FindCell(b2Vec2 currentPosition);
	Cell* FindCell(int id);
	int FindCellID(b2Vec2 currentPosition);
private:
	WorldInfo m_WorldInfo = {};
	std::vector<Cell*> m_Cells = {};
};

//**********
//CELL CLASS
//**********
class Cell
{
public:
	enum class ThreatLevel {LOW,MODERATE,SEVERE};

	Cell(int id,b2Vec2 center,float width,float height,ThreatLevel threatLevel = ThreatLevel::LOW): m_ID(id),m_Center(center),m_CellWidth(width),m_CellHeight(height),m_ThreatLevel(threatLevel){}
	virtual ~Cell() {}

	b2Vec2 GetCenter() const { return m_Center; };
	std::vector<b2Vec2> GetCheckpoints() const { return m_Checkpoints; }
	float GetCellWidth() const { return m_CellWidth; }
	float GetCellHeight() const { return m_CellHeight; }
	int GetCellID() const { return m_ID; }
	ThreatLevel GetThreatLevel() const { return m_ThreatLevel; }
	bool IsChecked() const { return m_IsChecked; }
	
	void SetChecked(bool value) { m_IsChecked = value; }
	void CreateCheckpoints(float offset);
	void SetThreatLevel(ThreatLevel level) { m_ThreatLevel = level; }
	void Update(float dt);
private:
	b2Vec2 m_Center = {};

	ThreatLevel m_ThreatLevel = ThreatLevel::LOW;

	int m_ID = 0;

	bool m_IsChecked = false;

	float m_CellWidth = 0.f;
	float m_CellHeight = 0.f;
	float m_CheckTimer = 0.f;

	const float m_ResetDelay = 200.f;

	std::vector<b2Vec2> m_Checkpoints = {};
};
#endif