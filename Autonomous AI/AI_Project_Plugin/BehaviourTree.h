#ifndef Behaviour_TREE_H
#define Behaviour_TREE_H

//Includes
#include "Blackboard.h"
#include <functional>
#include "stdafx.h"

//-----------------------------------------------------------------
// Behaviour TREE HELPERS
//-----------------------------------------------------------------
enum BehaviourState
{
	Failure,
	Success,
	Running
};

//-----------------------------------------------------------------
// Behaviour INTERFACES (BASE)
//-----------------------------------------------------------------
class IBehaviour
{
public:
	IBehaviour() {}
	virtual ~IBehaviour() {}
	virtual BehaviourState Execute(Blackboard* pBlackBoard) = 0;

protected:
	BehaviourState m_CurrentState = Failure;
};

//-----------------------------------------------------------------
// Behaviour TREE COMPOSITES (IBehaviour)
//-----------------------------------------------------------------
#pragma region COMPOSITES
class BehaviourComposite : public IBehaviour
{
public:
	explicit BehaviourComposite(std::vector<IBehaviour*> childrenBehaviours)
	{
		m_ChildrenBehaviours = childrenBehaviours;
	}
	virtual ~BehaviourComposite()
	{
		for (auto pb : m_ChildrenBehaviours)
			SafeDelete(pb);
		m_ChildrenBehaviours.clear();
	}
	virtual BehaviourState Execute(Blackboard* pBlackBoard) override = 0;

protected:
	std::vector<IBehaviour*> m_ChildrenBehaviours = {};
};

class BehaviourSelector : public BehaviourComposite
{
public:
	explicit BehaviourSelector(std::vector<IBehaviour*> childrenBehaviours) :
		BehaviourComposite(childrenBehaviours)
	{}
	virtual ~BehaviourSelector()
	{}

	virtual BehaviourState Execute(Blackboard* pBlackBoard) override;
};

class BehaviourSequence : public BehaviourComposite
{
public:
	explicit BehaviourSequence(std::vector<IBehaviour*> childrenBehaviours) :
		BehaviourComposite(childrenBehaviours)
	{}
	virtual ~BehaviourSequence()
	{}

	virtual BehaviourState Execute(Blackboard* pBlackBoard) override;
};

class BehaviourPartialSequence : public BehaviourSequence
{
public:
	explicit BehaviourPartialSequence(std::vector<IBehaviour*> childrenBehaviours)
		: BehaviourSequence(childrenBehaviours)
	{}
	virtual ~BehaviourPartialSequence() {};
	virtual BehaviourState Execute(Blackboard* pBlackBoard) override;

private:
	unsigned int m_CurrentBehaviourIndex = 0;
};
#pragma endregion

//-----------------------------------------------------------------
// Behaviour TREE CONDITIONAL (IBehaviour)
//-----------------------------------------------------------------
class BehaviourConditional : public IBehaviour
{
public:
	explicit BehaviourConditional(std::function<bool(Blackboard*)> fp) : m_fpConditional(fp)
	{}
	virtual BehaviourState Execute(Blackboard* pBlackBoard) override;

private:
	std::function<bool(Blackboard*)> m_fpConditional = nullptr;
};

//-----------------------------------------------------------------
// Behaviour TREE ACTION (IBehaviour)
//-----------------------------------------------------------------
class BehaviourAction : public IBehaviour
{
public:
	explicit BehaviourAction(std::function<BehaviourState(Blackboard*)> fp) : m_fpAction(fp)
	{}
	virtual BehaviourState Execute(Blackboard* pBlackBoard) override;

private:
	std::function<BehaviourState(Blackboard*)> m_fpAction = nullptr;
};

//-----------------------------------------------------------------
// Behaviour TREE (BASE)
//-----------------------------------------------------------------
class BehaviourTree final
{
public:
	explicit BehaviourTree(Blackboard* pBlackBoard, IBehaviour* pRootComposite)
		: m_pBlackBoard(pBlackBoard), m_pRootComposite(pRootComposite)
	{};
	~BehaviourTree()
	{
		SafeDelete(m_pRootComposite);
		SafeDelete(m_pBlackBoard);
	};

	BehaviourState Update()
	{
		if (m_pRootComposite == nullptr)
			return m_CurrentState = Failure;

		return m_CurrentState = m_pRootComposite->Execute(m_pBlackBoard);
	}
	Blackboard* GetBlackboard() const
	{
		return m_pBlackBoard;
	}

private:
	BehaviourState m_CurrentState = Failure;
	Blackboard* m_pBlackBoard = nullptr;
	IBehaviour* m_pRootComposite = nullptr;
};
#endif
