#ifndef UNPACKER_H
#define UNPACKER_H

// C++ includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <map>
#include <cstdlib>
#include <string>
#include <tcl.h>

// Custom includes
#include "IOCommand.h"
#include "UnpackerContext.h"
#include "EventData.h"
#include "EventParser.h"
#include "OutputAction.h"
#include "DisplayAction.h"

class Unpacker;

struct UnpackerRegistry{
	Unpacker* lastCreated;
	UnpackerRegistry() : lastCreated(0){}
};

class Unpacker{
	public:
		Unpacker();
		~Unpacker();

		// Getters/setters
		void setName(const std::string& name);
		const std::string& getName() const;
		UnpackerContext& getContext();

		void registerAction(const std::string& key, IOCommand* action);
		IOCommand* getAction(const std::string& key);
		int Act(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);

		// The method that actuallly runs the unpacker!
		int processAll();


	private:
		std::string m_name;
		UnpackerContext m_context;
		std::map<std::string, IOCommand*> m_actions;
};

// Used for creating unpacker instances
int UnpackerInstanceCmd(ClientData clientData, Tcl_Interp* intepr, int objc, Tcl_Obj* const objv[]);

// Used for creating unpacker objects and such
int UnpackerFactoryCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);

// Deleting unpacker instances cmon now
void DeleteUnpackerInstance(ClientData clientData);

#endif
