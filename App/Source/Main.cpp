#include "Core/Application.h"

#include "ImLayer.h"

int main()
{
	Core::ApplicationSpecification appSpec;
	appSpec.Name = "Architecture";
	appSpec.WindowSpec.Width = 1920;
	appSpec.WindowSpec.Height = 1080;

	Core::Application application(appSpec);
	//application.PushLayer<Sandbox::SponzaTestLayer>();
	application.PushLayer<Core::ImLayer>();
	application.Run();
}
