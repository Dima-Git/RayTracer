#include "Configurer.h"
#include "AppSystem.h"
#include <iostream>

int main(int argc, char * argv[])
{
	//for (double q = -2.2; q < 2.2; q += 0.1)
	//	printf("fmod(fmod(%lf, 1.0) + 1.0, 1.0) = %lf\n", q, fmod(fmod(q, 1.0) + 1.0, 1.0));
	//return 0;

	Configurer config;

	//if (argc < 2 || !config.load(argv[1])) {
	//	std::cerr << "Configuration file not specified or invalid." << std::endl;
	//	return 1;
	//}

	config.load("default.rtconf");

	AppSystem app_system(config);
	//TODO: Read config
	AppSystem::InitPhase init_result = app_system.init();
	if (init_result == AppSystem::InitPhase::SUCCESS) {
		app_system.loop();
	}
	app_system.cleanup(init_result);
	
	return 0;
}