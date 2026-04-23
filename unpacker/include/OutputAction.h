#ifndef OUTPUTACTION_H
#define OUTPUTACTION_H

// C++ includes
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <tcl.h>

// Unpacker specific
#include "IOCommand.h"
#include "EventData.h"

// ROOT includes
#include "TSystem.h"
#include "TFile.h"
#include "TTree.h"

class OutputAction : public IOCommand{
	public:
		explicit OutputAction(UnpackerContext* context);
		
		int Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		
		const std::string& getStorageArea() const;
		const std::vector<std::string>& getDescriptions() const;

		int createFile(const std::string& fileName);
		// Runtime hookers (hehe) for file IO
		int beginRun(const std::string& fileName);
		int processEvent(const EventData& event);
		int endRun();

	private:
		int setError(Tcl_Interp* interp, const char* message);

		TFile* m_outputFile;
};

#endif
