#ifndef BLACKBOARD_H
#define BLACKBOARD_H
#include <unordered_map>
#include "Helpers.h"

class IBlackboardField
{
public:
	virtual ~IBlackboardField() {};
};

template<typename T>
class BlackboardField : public IBlackboardField
{
public:
	explicit BlackboardField(T data) :m_Data(data) {};

	T GetData() { return m_Data; }
	void SetData(T data) { m_Data = data; }
private:
	T m_Data;
};

class Blackboard final
{
public:
	Blackboard() {}
	~Blackboard()
	{
		for (auto e : m_BlackboardData)
			SafeDelete(e.second);

		m_BlackboardData.clear();
	}

	template<typename T> bool AddData(const std::string& name, T data)
	{
		auto it = m_BlackboardData.find(name);
		if (it == m_BlackboardData.end())
		{
			m_BlackboardData[name] = new BlackboardField<T>(data);
			return true;
		}
		printf("WARNING: Data '%s' of type '%s' already in Blackboard \n", name.c_str(), typeid(T).name());
		return false;
	}

	template<typename T> bool ChangeData(const std::string& name, T data)
	{
		auto it = m_BlackboardData.find(name);
		if(it != m_BlackboardData.end())
		{
			BlackboardField<T>* p = dynamic_cast<BlackboardField<T>*>(m_BlackboardData[name]);
			if(p)
			{
				p->SetData(data);
				return true;
			}
		}
		printf("WARNING: Data '%s' of type '%s' not found in Blackboard \n", name.c_str(), typeid(T).name());
		return false;
	}

	template<typename T> bool GetData(const std::string& name,T& data)
	{
		BlackboardField<T>* p = dynamic_cast<BlackboardField<T>*>(m_BlackboardData[name]);
		if (p != nullptr)
		{
			data = p->GetData();
			return true;
		}
		printf("WARNING: Data '%s' of type '%s' not found in Blackboard \n", name.c_str(), typeid(T).name());
		return false;
	}
private:
	std::unordered_map<std::string, IBlackboardField*> m_BlackboardData;
};
#endif
