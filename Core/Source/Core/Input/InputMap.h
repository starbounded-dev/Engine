#pragma once

#include "Input.h"

#include <string>
#include <unordered_map>
#include <vector>

namespace Core {

	enum class InputBindingType
	{
		Key,
		MouseButton
	};

	struct InputBinding
	{
		InputBindingType Type;
		int Code; // KeyCode or MouseButton

		InputBinding(InputBindingType type, int code)
			: Type(type), Code(code) {}
	};

	class InputMap
	{
	public:
		// Action mappings (binary: pressed/not pressed)
		// Note: Using separate names because KeyCode and MouseButton are both int
		void MapActionKey(const std::string& actionName, KeyCode key);
		void MapActionMouse(const std::string& actionName, MouseButton button);
		void MapAction(const std::string& actionName, const InputBinding& binding);
		void AddActionBindingKey(const std::string& actionName, KeyCode key);
		void AddActionBindingMouse(const std::string& actionName, MouseButton button);

		bool IsActionPressed(const std::string& actionName) const;
		bool IsActionReleased(const std::string& actionName) const;

		// Axis mappings (continuous values: -1.0 to 1.0)
		void MapAxis(const std::string& axisName, KeyCode positiveKey, KeyCode negativeKey);
		float GetAxis(const std::string& axisName) const;

		// Clear all mappings
		void Clear();

		// Clear specific action/axis
		void ClearAction(const std::string& actionName);
		void ClearAxis(const std::string& axisName);

	private:
		std::unordered_map<std::string, std::vector<InputBinding>> m_ActionBindings;
		std::unordered_map<std::string, std::pair<KeyCode, KeyCode>> m_AxisBindings; // positive, negative
	};

}
