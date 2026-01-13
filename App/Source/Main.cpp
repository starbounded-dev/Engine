#include "Core/Application.h"

#include "AppLayer.h"
#include "OverlayLayer.h"
#include "ImLayer.h"
#include "ExampleLayer.h"

int main()
{
	Core::ApplicationSpecification appSpec;
	appSpec.Name = "Architecture";
	appSpec.WindowSpec.Width = 1920;
	appSpec.WindowSpec.Height = 1080;

	Core::Application application(appSpec);
	//application.PushLayer<AppLayer>();
	//application.PushLayer<OverlayLayer>();
	//application.PushLayer<Core::ImLayer>();
	application.PushLayer<Sandbox::SponzaTestLayer>();
	application.Run();
}
