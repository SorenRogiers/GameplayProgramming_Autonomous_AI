#include "stdafx.h"
#include "SteeringBehaviours.h"

//SEEK CALCULATE
//**************
PluginOutput SeekBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	b2Vec2 desiredVelocity = (*m_pTargetInfo).position - pAgent->Position;
	desiredVelocity.Normalize();

	desiredVelocity *= pAgent->MaxLinearSpeed;

	PluginOutput steeringOutput = {};
	steeringOutput.LinearVelocity = desiredVelocity - pAgent->LinearVelocity;

	return steeringOutput;
}

//FLEE CALCULATE
//**************
PluginOutput FleeBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	PluginOutput steeringOutput = {};

	if (b2DistanceSquared(pAgent->Position, (*m_pTargetInfo).position) > m_StartFleeDistance * m_StartFleeDistance)
	{
		steeringOutput.LinearVelocity = { 0,0 };
		return steeringOutput;
	}

	b2Vec2 desiredVelocity = pAgent->Position - (*m_pTargetInfo).position;
	desiredVelocity.Normalize();

	desiredVelocity *= pAgent->MaxLinearSpeed;

	steeringOutput.LinearVelocity = desiredVelocity - pAgent->LinearVelocity;
	steeringOutput.RunMode = true;
	return steeringOutput;
}

//ARRIVE CALCULATE
//****************
PluginOutput ArriveBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	auto desiredVelocity = (*m_pTargetInfo).position - pAgent->Position;
	auto distance = desiredVelocity.Normalize() - m_TargetRadius;

	if (distance < m_SlowRadius)
	{
		desiredVelocity *= pAgent->MaxLinearSpeed * (distance / (m_SlowRadius + m_TargetRadius));
	}
	else
	{
		desiredVelocity *= pAgent->MaxLinearSpeed;
	}

	PluginOutput steeringOutput = {};
	steeringOutput.LinearVelocity = desiredVelocity - pAgent->LinearVelocity;

	return steeringOutput;
}

//PURSUE CALCULATE
//****************
PluginOutput PursueBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	auto distance = (pAgent->Position - (*m_pTargetInfo).position).Length();
	auto t = distance / pAgent->MaxLinearSpeed;

	auto targetVelocity = (*m_pTargetInfo).linearVelocity;
	targetVelocity.Normalize();

	auto newTarget = TargetInformation((*m_pTargetInfo).position + (targetVelocity * t));
	SeekBehaviour::m_pTargetInfo = &newTarget;

	return SeekBehaviour::CalculateSteering(deltaT, pAgent);
}

//EVADE CALCULATE
//***************
PluginOutput EvadeBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	auto distance = (pAgent->Position - (*m_pTargetInfo).position).Length();
	auto t = distance / pAgent->MaxLinearSpeed;

	auto targetVelocity = (*m_pTargetInfo).linearVelocity;
	targetVelocity.Normalize();

	auto newTarget = TargetInformation((*m_pTargetInfo).position + (targetVelocity * t));
	FleeBehaviour::m_pTargetInfo = &newTarget;

	return FleeBehaviour::CalculateSteering(deltaT, pAgent);
}

//WANDER CALCULATE
//****************
PluginOutput WanderBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	auto offset = pAgent->LinearVelocity;
	offset.Normalize();
	offset *= m_WanderOffset;

	b2Vec2 circleOffset = { cos(m_WanderAngle) * m_WanderRadius, sin(m_WanderAngle) * m_WanderRadius };

	m_WanderAngle += randomFloat() * m_AngleChange - (m_AngleChange * .5f);

	auto newTarget = TargetInformation(pAgent->Position + offset + circleOffset);
	SeekBehaviour::m_pTargetInfo = &newTarget;

	//DEBUG
	if (m_pBehaviourPlugin)
	{
		auto pos = pAgent->Position;

		m_pBehaviourPlugin->DEBUG_DrawSegment(pos, pos + offset, { 1,0,0 });
		m_pBehaviourPlugin->DEBUG_DrawCircle(pos + offset, m_WanderRadius, { 0,0,1 });
		m_pBehaviourPlugin->DEBUG_DrawSolidCircle(pos + offset + circleOffset, 0.1f, { 0,0 }, { 0, 1, 0 });
	}

	return SeekBehaviour::CalculateSteering(deltaT, pAgent);
}

//HIDE CALCULATE
//**************
PluginOutput HideBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	float distanceToClosest = (std::numeric_limits<float>::max)();
	b2Vec2 bestHidingspot = {};

	std::vector<b2Body*>::iterator itr = m_pObstaclesArr.begin();
	while (itr != m_pObstaclesArr.end())
	{
		b2Vec2 hidingspot = GetBestHidingPosition((*itr)->GetPosition(), m_pTargetInfo);

		float distance = b2DistanceSquared(hidingspot, pAgent->Position);

		if (distance < distanceToClosest)
		{
			distanceToClosest = distance;
			bestHidingspot = hidingspot;
		}

		++itr;
	}

	//If we didnt find a suitable hiding spot flee from the hunter
	if (distanceToClosest == (std::numeric_limits<float>::max)())
	{
		//Flee behaviour
		PluginOutput steeringOutput = {};

		auto targetVelocity = pAgent->Position - (*m_pTargetInfo).position;
		targetVelocity.Normalize();
		targetVelocity *= pAgent->MaxLinearSpeed;

		steeringOutput.LinearVelocity = targetVelocity - pAgent->LinearVelocity;

		return steeringOutput;
	}

	//Hide in the best spot possible
	auto newTarget = TargetInformation(bestHidingspot);
	ArriveBehaviour::m_pTargetInfo = &newTarget;

	return ArriveBehaviour::CalculateSteering(deltaT, pAgent);
}

b2Vec2 HideBehaviour::GetBestHidingPosition(const b2Vec2& obstaclePosition, TargetInformation* pTarget)
{
	const float distanceFromBoundary = 5.f;
	float distanceAway = 1.0f + distanceFromBoundary;

	b2Vec2 directionToObstacle = obstaclePosition - (*pTarget).position;
	directionToObstacle.Normalize();

	return (directionToObstacle * distanceAway) + obstaclePosition;

}
//AVOID-ENEMY CALCULATE
//*********************
PluginOutput ObstacleAvoidanceBehaviour::CalculateSteering(float deltaT, AgentInfo* pAgent)
{
	auto velocity = pAgent->LinearVelocity;
	velocity.Normalize();

	auto maxSeeAhead = 10.f;

	auto ahead = pAgent->Position + velocity * maxSeeAhead;
	auto ahead2 = pAgent->Position + velocity * maxSeeAhead * 0.5f;

	auto mostThreatening = FindMostThreateningObstacle(ahead, ahead2, pAgent->Position);

	b2Vec2 avoidance = {};

	if (!mostThreatening.IsEmpty())
	{
		avoidance.x = ahead.x - mostThreatening.position.x;
		avoidance.y = ahead.y - mostThreatening.position.y;

		avoidance.Normalize();
		avoidance *= m_MaxAvoidanceForce;
	}
	else
	{
		avoidance.SetZero();
	}

	PluginOutput steering = {};
	steering.LinearVelocity = avoidance;

	return steering;
}

Obstacle ObstacleAvoidanceBehaviour::FindMostThreateningObstacle(const b2Vec2& ahead, const b2Vec2& ahead2, const b2Vec2& currentPos)
{
	Obstacle mostThreatening = {};

	for(auto ob : m_Obstacles)
	{
		auto collision = LineIntersectsCircle(ahead, ahead2, ob);

		auto distancePosToEnemySqrt = b2DistanceSquared(currentPos, ob.position);
		auto distancePosToMtSqrt = b2DistanceSquared(currentPos, mostThreatening.position);

		if (collision && (mostThreatening.IsEmpty() || distancePosToEnemySqrt < distancePosToMtSqrt))
		{
			mostThreatening = ob;
		}
	}
	return mostThreatening;

}

bool ObstacleAvoidanceBehaviour::LineIntersectsCircle(b2Vec2 ahead, b2Vec2 ahead2, Obstacle obstacle) const
{
	auto distanceAheadSqrt = b2DistanceSquared(obstacle.position, ahead);
	auto distanceAhead2Sqrt = b2DistanceSquared(obstacle.position, ahead2);

	return distanceAheadSqrt <= obstacle.radius * obstacle.radius || distanceAhead2Sqrt <= obstacle.radius * obstacle.radius;
}

//The avoidance force must be added to the character's velocity vector. 
//all steering forces can be combined into one, producing a force that represents all active behavior acting on the character.
//Depending on the avoidance force angle and direction it will not interrupt other steering forces, 
//such as seek or wander.The avoidance force is added to the player velocity as usual