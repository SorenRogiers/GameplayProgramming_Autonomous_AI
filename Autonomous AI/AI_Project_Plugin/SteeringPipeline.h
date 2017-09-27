#ifndef STEERING_PIPELINE_H
#define STEERING_PIPELINE_H
#include <Box2D/Common/b2Math.h>
#include "IBehaviourPlugin.h"
#include "SteeringBehaviours.h"
#include <bemapiset.h>
#include "CombinedBehaviours.h"

//****
//GOAL
struct Goal
{
	b2Vec2 position = { 0.f,0.f };
	b2Vec2 velocity = { 0.f,0.f };
	float orientation = 0.f;
	float rotation = 0.f;

	bool isPositionSet = false;
	bool isOrientationSet = false;
	bool isVelocitySet = false;
	bool isRotationSet = false;

	Goal() {};

	void Clear()
	{
		position.SetZero();
		velocity.SetZero();
		orientation = 0.f;
		rotation = 0.f;

		isOrientationSet = false;
		isVelocitySet = false;
		isOrientationSet = false;
		isRotationSet = false;
	}

	void UpdateGoal(const Goal& goal)
	{
		if (goal.isPositionSet)
		{
			position = goal.position;
			isPositionSet = true;
		}
		if (goal.isOrientationSet)
		{
			orientation = goal.orientation;
			isOrientationSet = true;
		}
		if (goal.isVelocitySet)
		{
			velocity = goal.velocity;
			isVelocitySet = true;
		}
		if (goal.isRotationSet)
		{
			rotation = goal.rotation;
			isRotationSet = true;
		}
	}

	bool CanMergeGoal(const Goal& goal) const
	{
		return !((isPositionSet && goal.isPositionSet) || (isOrientationSet && goal.isOrientationSet) || (isVelocitySet && goal.isVelocitySet) || (isRotationSet && goal.isRotationSet));
	}
};

//****
//PATH
class Path
{
public:
	Path() {};
	virtual ~Path() {};

	void SetGoal(Goal goal) { m_Goal = goal; }
	Goal GetGoal() const { return m_Goal; }

	void SetAgent(AgentInfo* pAgent) { m_pAgent = pAgent; }

	float GetMaxPriority()const;
private:
	float m_MaxPriority = 50.f;
	AgentInfo* m_pAgent = nullptr;
	Goal m_Goal = {};

};

//********
//TARGETER
class Targeter
{
public:
	Targeter() {};
	virtual ~Targeter() {};

	Goal GetGoal() const { return m_Goal; };
	Goal& GetGoalRef() { return m_Goal; };

private:
	Goal m_Goal = {};
};

//**********
//DECOMPOSER
class Decomposer
{
public:
	Decomposer(IBehaviourPlugin* pBehaviourPlugin) :m_pBehaviourPlugin(pBehaviourPlugin) {};
	virtual ~Decomposer() {};

	virtual Goal DecomposeGoal(Goal& goal);
private:
	IBehaviourPlugin* m_pBehaviourPlugin = nullptr;

};

//**********
//CONSTRAINT
class Constraint
{
public:
	Constraint() {};
	virtual ~Constraint() {};

	virtual float WillViolate(const Path* pPath, AgentInfo* pAgent, float maxPriority) = 0;
	virtual Goal SuggestGoal(const Path* pPath) = 0;
	void SetSuggestionUsed(bool value) { m_SuggestionUsed = value; }
private:
	bool m_SuggestionUsed = false;
};

//********
//AcTUATOR
class Actuator
{
public:
	Actuator(BlendedSteeringBehaviour* pBlendedSteering):m_pBlendedSteeringBehaviour(pBlendedSteering) {};
	virtual ~Actuator() { SafeDelete(m_pPath); };

	virtual Path* CreatePath();
	virtual void UpdatePath(Path* pPath, AgentInfo* pAgent, const Goal& goal);
	virtual PluginOutput CalculateSteering(const Path* pPath, float deltaT, AgentInfo* pAgent);
private:
	Path* m_pPath = nullptr;
	std::vector<SteeringBehaviour*> m_pBehaviours = {};
	BlendedSteeringBehaviour* m_pBlendedSteeringBehaviour = nullptr;
};

//****************
//STEERINGPIPELINE
class SteeringPipeline : public SteeringBehaviour
{
public:
	SteeringPipeline() {};
	virtual ~SteeringPipeline() {};

	void SetActuators(Actuator* actuators) { m_pActuator = actuators; SafeDelete(m_pPath); }
	void SetTargeters(std::vector<Targeter*> targeters) { m_pTargeters = targeters; }
	void SetDecomposers(std::vector<Decomposer*> decomposers) { m_pDecomposers = decomposers; }
	void SetConstraints(std::vector<Constraint*> constraints) { m_pConstraints = constraints; }
	void SetFallBackBehaviour(SteeringBehaviour* pFallback) { m_pFallbackBehaviour = pFallback; }

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;
private:
	Path* m_pPath = nullptr;
	Actuator* m_pActuator = nullptr;

	std::vector<Targeter*> m_pTargeters = {};
	std::vector<Decomposer*> m_pDecomposers = {};
	std::vector<Constraint*> m_pConstraints = {};

	SteeringBehaviour* m_pFallbackBehaviour = nullptr;

	UINT m_MaxConstraintSteps = 10;
};
#endif