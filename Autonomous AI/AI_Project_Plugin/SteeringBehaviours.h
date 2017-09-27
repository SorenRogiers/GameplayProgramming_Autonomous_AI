#ifndef STEERING_BEHAVIOURS_H
#define STEERING_BEHAVIOURS_H
#include "IBehaviourPlugin.h"
#include "Helpers.h"

//BASE STEERINGBEHAVIOUR
//**********************
class SteeringBehaviour
{
public:
	SteeringBehaviour() {};
	SteeringBehaviour(IBehaviourPlugin* pPlugin):m_pBehaviourPlugin(pPlugin) {};
	virtual ~SteeringBehaviour() {};

	virtual PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) = 0;
	virtual void SetTarget(TargetInformation* pTarget) { m_pTargetInfo = pTarget; }

	template <class T, typename std::enable_if<std::is_base_of<SteeringBehaviour, T>::value>::type* = nullptr>
	T* As()
	{
		return static_cast<T*>(*this);
	}

protected:
	IBehaviourPlugin* m_pBehaviourPlugin = nullptr;
	TargetInformation* m_pTargetInfo = nullptr;
};

//SEEK
//****
class SeekBehaviour : public SteeringBehaviour
{
public:
	SeekBehaviour() {};
	virtual ~SeekBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;
};

//FLEE
//****
class FleeBehaviour : public SteeringBehaviour
{
public:
	FleeBehaviour() {};
	virtual ~FleeBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	virtual void SetFleeWithinDistance(float startFleeDistance) { m_StartFleeDistance = startFleeDistance; }
protected:
	float m_StartFleeDistance = 21.0f;
};

//ARRIVE
//******
class ArriveBehaviour : public SteeringBehaviour
{
public:
	ArriveBehaviour() {};
	virtual ~ArriveBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	virtual void SetSlowRadius(float slowRadius) { m_SlowRadius = slowRadius; }

protected:
	float m_SlowRadius = 10.f;
	float m_TargetRadius = 1.f;
};

//PURSUE
//******
class PursueBehaviour : public SeekBehaviour
{
public:
	PursueBehaviour() {};
	virtual ~PursueBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;

protected:
	TargetInformation* m_pTargetInfo = nullptr;
};

//EVADE
//*****
class EvadeBehaviour : public FleeBehaviour
{
public:
	EvadeBehaviour() {};
	virtual ~EvadeBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent)override;
};

//WANDER
//******
class WanderBehaviour : public SeekBehaviour
{
public:
	WanderBehaviour() {};
	WanderBehaviour(IBehaviourPlugin* pPlugin) { m_pBehaviourPlugin = pPlugin; };

	virtual ~WanderBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	virtual void SetWanderOffset(float offset) { m_WanderOffset = offset; }
	virtual void SetWanderRadius(float radius) { m_WanderRadius = radius; }
	virtual void SetAngleChange(float angle) { m_AngleChange = angle; }

protected:
	float m_WanderOffset = 5.0f;
	float m_WanderRadius = 5.0f;
	float m_WanderAngle = 0.f;
	float m_AngleChange = ToRadians(45);

	TargetInformation* m_pTargetInfo = nullptr;
};

//HIDE
//****
class HideBehaviour : public ArriveBehaviour
{
public:
	HideBehaviour() {};
	HideBehaviour(std::vector<b2Body*> obstacles) :m_pObstaclesArr(obstacles) {};
	virtual ~HideBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;
	b2Vec2 GetBestHidingPosition(const b2Vec2& obstaclePosition, TargetInformation* pTarget);
protected:
	std::vector<b2Body*> m_pObstaclesArr;
	float m_StartFleeDistance = 10.0f;

};

//OBSTACLE-AVOIDANCE
//******************
class ObstacleAvoidanceBehaviour : public SteeringBehaviour
{
public:
	ObstacleAvoidanceBehaviour(){}
	virtual ~ObstacleAvoidanceBehaviour() {};

	PluginOutput CalculateSteering(float deltaT, AgentInfo* pAgent) override;

	void UpdateObstacles(std::vector<Obstacle> obstacles) { m_Obstacles = obstacles; }
	void SetMaxAvoidanceForce(float maxForce) { m_MaxAvoidanceForce = maxForce; }

private:
	std::vector<Obstacle> m_Obstacles = {};
	float m_MaxAvoidanceForce = 30.f;

private:
	bool LineIntersectsCircle(b2Vec2 ahead, b2Vec2 ahead2, Obstacle obstacle) const;
	Obstacle FindMostThreateningObstacle(const b2Vec2& ahead, const b2Vec2& ahead2, const b2Vec2& currentPos);

};
#endif