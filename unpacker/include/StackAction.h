#ifndef STACKACTION_H
#define STACKACTION_H

// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <tcl.h>

// Unpacker specific
#include "IOCommand.h"
#include "Module.h"

class StackAction : public IOCommand{
	public:
		explicit StackAction(UnpackerContext* context);
		int Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		
	private:
		int handleModule(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		int setError(Tcl_Interp* interp, const char* message);
};

#endif
