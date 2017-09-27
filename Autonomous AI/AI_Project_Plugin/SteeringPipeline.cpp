#include "stdafx.h"
#include "SteeringPipeline.h"

//****
//PATH
float Path::GetMaxPriority() const
{
	//Distance between (sub)goal and character (current path)

	if (!m_pAgent)
		return 0.f;

	return (m_pAgent->Position - m_Goal.position).Length();
}

//**********
//DECOMPOSER
Goal Decomposer::DecomposeGoal(Goal& goal)
{
	goal.position = m_pBehaviourPlugin->NAVMESH_GetClosestPathPoint(goal.position);
	return goal;
}

//********
//ACTUATOR
Path* Actuator::CreatePath()
{
	SafeDelete(m_pPath);

	m_pPath = new Path();
	return m_pPath;
}

void Actuator::UpdatePath(Path* pPath, AgentInfo* pAgent, const Goal& goal)
{
	pPath->SetAgent(pAgent);
	pPath->SetGoal(goal);
}

PluginOutput Actuator::CalculateSteering(const Path* pPath, float deltaT, AgentInfo* pAgent)
{
	auto goal = pPath->GetGoal();

	if (goal.isPositionSet && m_pBlendedSteeringBehaviour)
	{
		for(size_t i =0;i<m_pBlendedSteeringBehaviour->m_weightedBehavioursVec.size();i++)
		{
			if (dynamic_cast<SeekBehaviour*>(m_pBlendedSteeringBehaviour->m_weightedBehavioursVec[i].pSteeringBehaviour))
			{
				auto newTarget = TargetInformation(goal.position);
				m_pBlendedSteeringBehaviour->m_weightedBehavioursVec[i].pSteeringBehaviour->SetTarget(&newTarget);
			}
		}

		return m_pBlendedSteeringBehaviour->CalculateSteering(deltaT, pAgent);
	}

	return PluginOutput();
}
//*****************
//STEERING PIPELINE
PluginOutput SteeringPipeline::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	Goal currentGoal = {};

	//1.Targeter
	for (auto pTargeter : m_pTargeters)
	{
		Goal targetGoal = pTargeter->GetGoal();
		if (currentGoal.CanMergeGoal(targetGoal))
		{
			currentGoal.UpdateGoal(targetGoal);
		}
	}

	//2.Decomposer
	for (auto pDecomposer : m_pDecomposers)
	{
		currentGoal = pDecomposer->DecomposeGoal(currentGoal);
	}

	//3.Path & constraints
	if (!m_pPath)
		m_pPath = m_pActuator->CreatePath();

	//float shortestViolation, currentViolation, maxViolation = 0;

	for (UINT i = 0; i < m_MaxConstraintSteps; ++i)
	{
		//Get the path to the goal
		m_pActuator->UpdatePath(m_pPath, pAgent, currentGoal);

		return m_pActuator->CalculateSteering(m_pPath, deltaT, pAgent);
	}

	if (m_pFallbackBehaviour)
		return m_pFallbackBehaviour->CalculateSteering(deltaT, pAgent);

	return PluginOutput();
}
