//Precompiled Header [ALWAYS ON TOP IN CPP]
#include "stdafx.h"
#include "BehaviourTree.h"

//-----------------------------------------------------------------
// Behaviour TREE COMPOSITES (IBehaviour)
//-----------------------------------------------------------------
#pragma region COMPOSITES
//SELECTOR
BehaviourState BehaviourSelector::Execute(Blackboard* pBlackBoard)
{
	for (auto child : m_ChildrenBehaviours)
	{
		m_CurrentState = child->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case Failure:
			continue; break;
		case Success:
			return m_CurrentState; break;
		case Running:
			return m_CurrentState; break;
		default:
			continue; break;
		}
	}
	return m_CurrentState = Failure;
}
//SEQUENCE
BehaviourState BehaviourSequence::Execute(Blackboard* pBlackBoard)
{
	for (auto child : m_ChildrenBehaviours)
	{
		m_CurrentState = child->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case Failure:
			return m_CurrentState; break;
		case Success:
			continue; break;
		case Running:
			return m_CurrentState; break;
		default:
			m_CurrentState = Success;
			return m_CurrentState; break;
		}
	}
	return m_CurrentState = Success;
}
//PARTIAL SEQUENCE
BehaviourState BehaviourPartialSequence::Execute(Blackboard* pBlackBoard)
{
	while (m_CurrentBehaviourIndex < m_ChildrenBehaviours.size())
	{
		m_CurrentState = m_ChildrenBehaviours[m_CurrentBehaviourIndex]->Execute(pBlackBoard);
		switch (m_CurrentState)
		{
		case Failure:
			m_CurrentBehaviourIndex = 0;
			return m_CurrentState; break;
		case Success:
			++m_CurrentBehaviourIndex;
			return m_CurrentState = Running; break;
		case Running:
			return m_CurrentState; break;
		}
	}

	m_CurrentBehaviourIndex = 0;
	return m_CurrentState = Success;
}
#pragma endregion
//-----------------------------------------------------------------
// Behaviour TREE CONDITIONAL (IBehaviour)
//-----------------------------------------------------------------
BehaviourState BehaviourConditional::Execute(Blackboard* pBlackBoard)
{
	if (m_fpConditional == nullptr)
		return Failure;

	switch (m_fpConditional(pBlackBoard))
	{
	case true:
		return m_CurrentState = Success;
	case false:
		return m_CurrentState = Failure;
	}
	return m_CurrentState = Failure;
}
//-----------------------------------------------------------------
// Behaviour TREE ACTION (IBehaviour)
//-----------------------------------------------------------------
BehaviourState BehaviourAction::Execute(Blackboard* pBlackBoard)
{
	if (m_fpAction == nullptr)
		return Failure;

	return m_CurrentState = m_fpAction(pBlackBoard);
}