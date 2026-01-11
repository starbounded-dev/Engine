#include "InputMap.h"

#include "Core/Debug/Profiler.h"

namespace Core {

	void InputMap::MapActionKey(const std::string& actionName, KeyCode key)
	{
		m_ActionBindings[actionName].clear();
		m_ActionBindings[actionName].emplace_back(InputBindingType::Key, key);
	}

	void InputMap::MapActionMouse(const std::string& actionName, MouseButton button)
	{
		m_ActionBindings[actionName].clear();
		m_ActionBindings[actionName].emplace_back(InputBindingType::MouseButton, button);
	}

	void InputMap::MapAction(const std::string& actionName, const InputBinding& binding)
	{
		m_ActionBindings[actionName].clear();
		m_ActionBindings[actionName].push_back(binding);
	}

	void InputMap::AddActionBindingKey(const std::string& actionName, KeyCode key)
	{
		m_ActionBindings[actionName].emplace_back(InputBindingType::Key, key);
	}

	void InputMap::AddActionBindingMouse(const std::string& actionName, MouseButton button)
	{
		m_ActionBindings[actionName].emplace_back(InputBindingType::MouseButton, button);
	}

	bool InputMap::IsActionPressed(const std::string& actionName) const
	{
		PROFILE_FUNC();

		auto it = m_ActionBindings.find(actionName);
		if (it == m_ActionBindings.end())
			return false;

		for (const auto& binding : it->second)
		{
			if (binding.Type == InputBindingType::Key)
			{
				if (Input::IsKeyPressed(binding.Code))
					return true;
			}
			else if (binding.Type == InputBindingType::MouseButton)
			{
				if (Input::IsMouseButtonPressed(binding.Code))
					return true;
			}
		}

		return false;
	}

	bool InputMap::IsActionReleased(const std::string& actionName) const
	{
		PROFILE_FUNC();

		auto it = m_ActionBindings.find(actionName);
		if (it == m_ActionBindings.end())
			return false;

		for (const auto& binding : it->second)
		{
			if (binding.Type == InputBindingType::Key)
			{
				if (Input::IsKeyReleased(binding.Code))
					return true;
			}
			else if (binding.Type == InputBindingType::MouseButton)
			{
				if (Input::IsMouseButtonReleased(binding.Code))
					return true;
			}
		}

		return false;
	}

	void InputMap::MapAxis(const std::string& axisName, KeyCode positiveKey, KeyCode negativeKey)
	{
		m_AxisBindings[axisName] = { positiveKey, negativeKey };
	}

	float InputMap::GetAxis(const std::string& axisName) const
	{
		PROFILE_FUNC();

		auto it = m_AxisBindings.find(axisName);
		if (it == m_AxisBindings.end())
			return 0.0f;

		float value = 0.0f;

		if (Input::IsKeyPressed(it->second.first))
			value += 1.0f;

		if (Input::IsKeyPressed(it->second.second))
			value -= 1.0f;

		return value;
	}

	void InputMap::Clear()
	{
		m_ActionBindings.clear();
		m_AxisBindings.clear();
	}

	void InputMap::ClearAction(const std::string& actionName)
	{
		m_ActionBindings.erase(actionName);
	}

	void InputMap::ClearAxis(const std::string& axisName)
	{
		m_AxisBindings.erase(axisName);
	}

}
