#ifndef UNPACKERCONTEXT_H
#define UNPACKERCONTEXT_H

// C++ includes
#include <iostream>
#include <iomanip>
#include <vector>
#include <map>

// Unpacker includes
#include "Module.h"
#include "EventData.h"

struct TreeInfo{
	std::vector<std::string> _modules;
	std::string description;
};

struct ModuleEntry{
	std::string classType; // ADCV792, for example
	std::string name; // cQDC (common QDC for LANA), for example
	Module* _module;
	std::map<std::string, std::string> params;

	ModuleEntry() : _module(0){}
};

struct UnpackerContext{
	EventData currentEvent;

	std::string stageArea;
	std::vector<std::string> runs;

	std::vector<std::string> classOrder;
	std::vector<std::string> className;
	std::vector<ModuleEntry> moduleStack; // important!!!

	std::map<std::string, TreeInfo> trees;

	std::vector<std::string> displayModules;

	std::string storageArea;
	std::vector<std::string> descriptions;

	int physicsEvent;
	int runNumber;

	UnpackerContext() : 
		stageArea(""), 
		storageArea(""),
		physicsEvent(0), 
		runNumber(-1){}

	~UnpackerContext(){
		for (size_t i=0; i < moduleStack.size(); i++){
			delete moduleStack[i]._module;
			moduleStack[i]._module = 0;
		}
	}


};
#endif
