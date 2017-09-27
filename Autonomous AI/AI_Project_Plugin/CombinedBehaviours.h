#ifndef COMBINED_BEHAVIOURS_H
#define COMBINED_BEHAVIOURS_H
#include "SteeringBehaviours.h"

struct BehaviourAndWeight
{
	SteeringBehaviour* pSteeringBehaviour;
	float weight = 0.0f;

	BehaviourAndWeight(SteeringBehaviour* pBehaviour, float w) : pSteeringBehaviour(pBehaviour), weight(w) {};
	void SetNewWeight(float newWeight) { weight = newWeight; };
};

//BLENDED
//*******
class BlendedSteeringBehaviour : public SteeringBehaviour
{
public:
	BlendedSteeringBehaviour(std::vector<BehaviourAndWeight> weightedBehaviours) : m_weightedBehavioursVec(weightedBehaviours) {};
	virtual ~BlendedSteeringBehaviour() {};

	void AddBehaviour(BehaviourAndWeight weightedBehaviour) { m_weightedBehavioursVec.push_back(weightedBehaviour); }

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	std::vector<BehaviourAndWeight> m_weightedBehavioursVec = {};
};

//PRIORITY
//********
class PrioritySteeringBehaviour : public SteeringBehaviour
{
public:
	PrioritySteeringBehaviour(std::vector<SteeringBehaviour*> behaviours) :m_pBehaviours(behaviours) {};
	virtual ~PrioritySteeringBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	std::vector<SteeringBehaviour*> m_pBehaviours = {};

private:
	float m_Epsilon = 0.0001f;
};

//BLENDED-PRIORITY
//****************
class BlendedPrioritySteeringBehaviour : public SteeringBehaviour
{
public:
	BlendedPrioritySteeringBehaviour(std::vector<BlendedSteeringBehaviour*> blendedPriorityGroups) :m_pBlendedPriorityGroups(blendedPriorityGroups) {};
	virtual ~BlendedPrioritySteeringBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;

private:
	std::vector<BlendedSteeringBehaviour*> m_pBlendedPriorityGroups = {};
	float m_Epsilon = 0.0001f;
};
#endif