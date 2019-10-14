#include "common/common.h"

#include "core/resource_loader.h"
#include "core/renderer/window.h"
#include "core/resource_manager.h"
#include "os/os.h"
#include "script/interpreter.h"

int main()
{
	printLine("Rootex Engine is starting: Build(" + OS::getSingleton().getBuildDate() + "|" + OS::getSingleton().getBuildTime() + ")");
    
	// Engine starts from build/game/.
	TextFile* r = ResourceLoader::createFileResource(DirectoryShortcut::ENGINE, "test\\abc.txt"); // So this loads build/game/abc.txt (However the binary exists in build/game/Debug/)
	printLine(r->getContents());

	Interpreter inter;
	
	Script* windowSettings = ResourceLoader::createScriptResource(DirectoryShortcut::GAME, "assets\\config\\window.lua");
	inter.loadExecuteScript(windowSettings);
	LuaVariable window = inter.getGlobal("window");
	GameWindow* gameWindow = new GameWindow(
		window["x"], 
		window["y"], 
		window["deltaX"], 
		window["deltaY"], 
		window["title"]);
	int ret = gameWindow->gameLoop();
	delete gameWindow;

	ResourceManager::emptyAll();
    return ret;
}
