#include "stdafx.h"
#include "Grid.h"

//GRID METHODS
//************
void Grid::CreateGrid(float size)
{
	float cellWidth = m_WorldInfo.Dimensions.x / size;
	float cellHeight = m_WorldInfo.Dimensions.y / size;

	float startX = m_WorldInfo.Center.x - ((size / 2) * cellWidth) + (cellWidth / 2);
	float startY = m_WorldInfo.Center.y + ((size / 2) * cellHeight) - (cellHeight / 2);

	b2Vec2 startCenter = { startX,startY };
	int id = 0;

	//Rearranged the order of the table abit to avoid setting targets that are far away
	//So when the agent reaches the end of a row it will go to the cell below instead of all the way to the left
	//1  2  3  4
	//8  7  6  5
	//9 10 11  12
	//16 15 14 13

	for(int i =0;i< size;++i)
	{
		if(i % 2 ==0)
		{
			if (i > 0)
				id += int(size)-1;
		}
		else if(i % 2 !=0)
		{
			id += int(size)+1;
		}

		for(int j=0;j< size;j++)
		{
			if (i % 2 == 0)
				id++;
			else
				id--;

			auto pCell = new Cell(id,startCenter,cellWidth,cellHeight);
			pCell->CreateCheckpoints(20.f);
			m_Cells.push_back(pCell);
			startCenter.x += cellWidth;
		}
		startCenter.x = startX;
		startCenter.y -= cellHeight;
	}
}

//CELL METHODS
//************
Cell* Grid::FindCell(b2Vec2 currentPosition)
{
	float distance = (std::numeric_limits<float>::max)();
	Cell* currentCell = nullptr;

	for(auto c : m_Cells)
	{
		auto currDistance = b2DistanceSquared(currentPosition, c->GetCenter());
		if(currDistance < distance)
		{
			distance = currDistance;
			currentCell = c;
		}
	}
	return currentCell;

	//This works fine too.
	/*for(auto c : m_Cells)
	{
	auto halfWidth = c->GetCellWidth() / 2;
	auto halfHeight = c->GetCellHeight() / 2;
	auto cellPos = c->GetCenter();

	if (currentPosition.x > cellPos.x - halfWidth && currentPosition.x < cellPos.x + halfWidth &&
	currentPosition.y > cellPos.y - halfHeight && currentPosition.y < cellPos.y + halfHeight)
	return c;
	}
	return nullptr;*/

}

Cell* Grid::FindCell(int id)
{
	for (auto c : m_Cells)
	{
		if (c->GetCellID() == id)
			return c;
	}
	return nullptr;
}

int Grid::FindCellID(b2Vec2 currentPosition)
{
	float distance = (std::numeric_limits<float>::max)();
	int currentCellID = -1;

	for (auto c : m_Cells)
	{
		auto currDistance = b2DistanceSquared(currentPosition, c->GetCenter());
		if (currDistance < distance)
		{
			distance = currDistance;
			currentCellID = c->GetCellID();
		}
	}
	return currentCellID;
}

void Cell::CreateCheckpoints(float offset)
{
	b2Vec2 checkpoint01 = { m_Center.x + offset,m_Center.y + offset };
	b2Vec2 checkpoint02 = { m_Center.x - offset,m_Center.y - offset };

	m_Checkpoints.push_back(checkpoint01);
	m_Checkpoints.push_back(checkpoint02);	
}

void Cell::Update(float dt)
{
	if (m_IsChecked)
	{
		m_CheckTimer += dt;

		if (m_CheckTimer > m_ResetDelay)
		{
			m_ThreatLevel = ThreatLevel::LOW;
			m_IsChecked = false;
			m_CheckTimer = 0;
		}
	}
}

