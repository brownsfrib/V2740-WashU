#ifndef INPUTACTION_H
#define INPUTACTION_H

// C++ includes
#include <iostream>
#include <vector>
#include <tcl.h>

// Unpacker specific 
#include "IOCommand.h"


class InputAction : public IOCommand{
	public:
		explicit InputAction(UnpackerContext* context);
		int Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]); // Don't touch this!!!


		const std::string& getStagearea() const;
		const std::vector<std::string>& getRuns() const;

	private:
		int setError(Tcl_Interp* interp, const char* message);

};
#endif

