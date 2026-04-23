// C++ includes
#include <iostream>
#include <stdexcept>
#include <tcl.h>

// Unpacker specific
#include "Unpacker.h"
#include "InputAction.h"
#include "StackAction.h"
#include "TreeAction.h"
#include "OutputAction.h"

// ROOT includes
#include "TSystem.h"
#include "TApplication.h"

int main(int argc, char* argv[]){
	const char* configFile = "configuration.tcl";
	if (argc > 1){
		configFile = argv[1];
	}
	

	Tcl_Interp* interp = Tcl_CreateInterp();
	if (Tcl_Init(interp) != TCL_OK){
		std::cerr << "Tcl_Init failed: " << Tcl_GetStringResult(interp) << std::endl;
		Tcl_DeleteInterp(interp);
		return 1;
	}
	
	UnpackerRegistry registry;
	// this casting shit is being very strange I don't like it anymore :(
	Tcl_CreateObjCommand(interp, "unpacker", UnpackerFactoryCmd, static_cast<ClientData>(&registry), NULL);
	
	int status = Tcl_EvalFile(interp, configFile);
	if (status != TCL_OK){
		std::cerr << "Error evaluating " << configFile << ": " << Tcl_GetStringResult(interp) << std::endl;
		Tcl_DeleteInterp(interp);
		return 1;
	}


	if (!registry.lastCreated){
		std::cerr << "No unpacker was created by the Tcl configuration." << std::endl;
		Tcl_DeleteInterp(interp);
		return 1;
	}
	
	status = registry.lastCreated->processAll();
	if (status != TCL_OK){
		std::cerr << "Automatic processing failed." << std::endl;
		Tcl_DeleteInterp(interp);
		return 1;
	}
	std::cout << "\n";
	
	Tcl_DeleteInterp(interp);
	

	return 0;
}
