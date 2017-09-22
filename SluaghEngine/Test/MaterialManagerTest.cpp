#include "MaterialManagerTest.h"
#include <Core\Engine.h>


SE::Test::MaterialManagerTest::MaterialManagerTest()
{
}


SE::Test::MaterialManagerTest::~MaterialManagerTest()
{
}

bool SE::Test::MaterialManagerTest::Run(Utilz::IConsoleBackend * console)
{
	auto& engine = Core::Engine::GetInstance();
	engine.Init(Core::Engine::InitializationInfo());
	auto& em = engine.GetEntityManager();
	auto& mm = engine.GetMaterialManager();
	Core::Entity entity = em.Create();

	Core::MaterialManager::CreateInfo info;
	auto& texture = Utilz::GUID("dummytex.sei");
	auto& shader = Utilz::GUID("SimpleTexPS.hlsl");
	info.textures = &texture;
	info.textureCount = 1;
	info.shader = &shader;
	info.shaderCount = 1;
	mm.Create(entity, info);

	auto& rm = engine.GetRenderableManager();
	auto& tm = engine.GetTransformManager();

	tm.Create(entity);

	rm.CreateRenderableObject(entity, Utilz::GUID("Placeholder_levsel.obj"));
	rm.ToggleRenderableObject(entity, true);

	engine.GetWindow()->MapActionButton(0, Window::KeyEscape);
	engine.GetWindow()->MapActionButton(1, Window::KeyO);
	engine.GetWindow()->MapActionButton(2, Window::KeyK);

	bool running = true;
	while (running)
	{
		if (engine.GetWindow()->ButtonPressed(0))
			running = false;
		
		if (engine.GetWindow()->ButtonPressed(1))
			rm.ToggleRenderableObject(entity, false);
		if (engine.GetWindow()->ButtonPressed(2))
			rm.ToggleRenderableObject(entity, true);

		engine.Frame();
	}


	engine.Release();

	return true;
}