#include "GameManager.h"

int main()
{
	GameManager* Game = new GameManager();
	Game->Run();

	delete Game;
	return 0;
}