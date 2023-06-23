#include "App.h"

int main()
{
	App::Get()->Run();
	App::Destroy();
	return 0;
}