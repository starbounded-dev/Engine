#pragma once

#include "Event.h"

#include <memory>

namespace Core {

	class Layer
	{
	public:
		Layer(const std::string& name = "Layer");
		virtual ~Layer() = default;

		virtual void OnEvent(Event& event) {}
		virtual void OnAttach() {}
		virtual void OnDetach() {}

		virtual void OnUpdate(float ts) {}
		virtual void OnImGuiRender() {}
		virtual void OnRender() {}

		template<std::derived_from<Layer> T, typename... Args>
		void TransitionTo(Args&&... args)
		{
			QueueTransition(std::move(std::make_unique<T>(std::forward<Args>(args)...)));
		}
	private:
		void QueueTransition(std::unique_ptr<Layer> layer);
	private:
		std::string m_DebugName;
	};

}