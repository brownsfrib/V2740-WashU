#ifndef TREEACTION_H
#define TREEACTION_H

// C++ includes
#include <iostream>
#include <algorithm>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <cstdlib>
#include <tcl.h>

// ROOT includes
#include "TFile.h"
#include "TTree.h"
#include "TNamed.h"
#include "TSystem.h"

// Unpacker includes
#include "IOCommand.h"
#include "EventData.h"

class TFile;
class TTree;


class TreeAction : public IOCommand{
	public:
		explicit TreeAction(UnpackerContext* context);
		virtual ~TreeAction();
		
		int Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		
		int beginRun(int runNumber, const std::string& evtFileName);
		int processEvent(const EventData& event);
		int endRun();
	
	private:
		struct BranchSpec{
			std::string moduleName;
			std::string branchName;
			int channels;
			int depth;
			double defaultValue;
			
			int sourceChannel;
			
			BranchSpec() : channels(32), defaultValue(-1.0), sourceChannel(-1) {}
		};
	
		// Waveform branch: writes moduleName data as branchName[nChannels][nSamples]
		struct WaveBranchSpec{
			std::string moduleName;  // EventData waveform key (e.g. "adc1")
			std::string branchName;  // ROOT branch name      (e.g. "adc")
			int depth;		     // Number of channels to be recorded (should always be total channels in digitizer)
			int nSamples;            // samples per waveform
			double defaultValue;
			
			WaveBranchSpec() : depth(64), nSamples(64), defaultValue(0.0) {}
		};
		
		// For mapping channels to a detector number
		struct MapSpec{
			std::string mapName; // cQDC, for example
			std::string detName;
			std::string _module;
			int channel;
			
			MapSpec() : channel(-1) {}
		};
		
		struct TreeSpec{
			std::string objectName;
			std::string treeName;
			std::string title;
			std::string descriptionTemplate;
			std::string outDir;
			std::set<int> runs;
			std::vector<BranchSpec> branches;
			
			TreeSpec() :
				treeName("Physics"),
				title("Physics tree"),
				descriptionTemplate(""),
				outDir(".") {}
		};
		
		struct ObjectCmdData{
			TreeAction* self;
			std::string name;
		};
		
		TreeSpec m_spec;
		
		TFile* m_file;
		TTree* m_tree;
		int m_activeRun;
		std::string m_activeEvtFile;
		
		std::map<std::string, std::vector<double> > m_buffers; // buffer to store data in
		std::map<std::string, int> m_hitCounters; // for writing out/resetting buffers - not recording a ton of zeros
		
		
		// Waveform stuff
		std::vector<WaveBranchSpec> m_waveBranches;
		std::map<std::string, int> m_waveCounters; // nHits counter per branch
		std::map<std::string, std::vector<int>> m_waveChannelBufs; // which channel each hit is in
		std::map<std::string, std::vector<double>> m_waveEnergyBufs; // energy per hit
		std::map<std::string, std::vector<double>> m_waveTimestampBufs; // timestamp per hit
		std::map<std::string, std::vector<double>> m_waveWaveBufs; // horrible name, waveform[nHits*nSamples]
		
		
		std::map<std::string, ObjectCmdData*> m_cmdData;
		std::vector<MapSpec> m_maps;
		
		int setError(Tcl_Interp* interp, const char* message);
		int createTree(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		int objectCommand(Tcl_Interp* interp, const std::string& name, int objc, Tcl_Obj* const objv[]);
		
		int handleConfig(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		int handleModule(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		int handleWaveBranch(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		int handleMap(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		int handleAdd(Tcl_Interp* interp, Tcl_Obj* listObj);
		int handleRemove(Tcl_Interp* interp, Tcl_Obj* listObj);
		int handleClear();
		
		int parseRunList(Tcl_Interp* interp, Tcl_Obj* obj, std::set<int>& runs, bool addMode);
		int ensureOutputDirectory(const std::string& path);
		int writeMappings();
		
		std::string buildOutputFileName(int runNumber) const;
		std::string resolveDescription(int runNumber) const;
		void resetBuffers();
		void buildBranches();
		
		static int ObjectCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		static void DeleteObjectCmd(ClientData clientData);
};

#endif
