#include "stdafx.h"
#include "CombinedBehaviours.h"

//BLENDED
//*******
PluginOutput BlendedSteeringBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	auto totalWeight = 0.f;
	PluginOutput steeringOutput = {};

	for (auto behaviour : m_weightedBehavioursVec)
	{
		auto steering = behaviour.pSteeringBehaviour->CalculateSteering(deltaT, pAgent);
		steeringOutput.LinearVelocity += behaviour.weight * steering.LinearVelocity;
		steeringOutput.AngularVelocity += behaviour.weight * steering.AngularVelocity;

		totalWeight += behaviour.weight;
	}

	if (totalWeight>0.f)
	{
		auto scale = 1.f / totalWeight;
		steeringOutput.LinearVelocity *= scale;
		steeringOutput.AngularVelocity *= scale;
	}

	return steeringOutput;
}
//PRIORITY
//********
PluginOutput PrioritySteeringBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	PluginOutput steeringOutput = {};

	for (auto pBehaviour : m_pBehaviours)
	{
		steeringOutput = pBehaviour->CalculateSteering(deltaT, pAgent);

		if (!steeringOutput.IsEmpty(m_Epsilon))
			break;
	}

	return steeringOutput;
}

//BLENDED-PRIORITY
//****************
PluginOutput BlendedPrioritySteeringBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	PluginOutput steeringOutput = {};

	for (auto pBehaviour : m_pBlendedPriorityGroups)
	{
		steeringOutput = pBehaviour->CalculateSteering(deltaT, pAgent);

		if (!steeringOutput.IsEmpty(m_Epsilon))
			break;
	}

	return steeringOutput;
}
