#ifndef IOCOMMAND_H
#define IOCOMMAND_H

// C++ includes
#include <iostream>
#include <tcl.h>

// Unpacker includes
#include "UnpackerContext.h"

class IOCommand{
	public:
		explicit IOCommand(UnpackerContext* context) : m_context(context) {}
		virtual ~IOCommand(){}
		virtual int Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]) = 0;

	protected:
		UnpackerContext* m_context;
};

#endif
