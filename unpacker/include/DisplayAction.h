#ifndef DISPLAYACTION_H
#define DISPLAYACTION_H

// C++ includes
#include <map>
#include <set>
#include <string>
#include <vector>
#include <sstream>
#include <tcl.h>

// ROOT includes
#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TObject.h"

// Unpacker includes
#include "IOCommand.h"
#include "EventData.h"

class TH1D;
class TH2D;
class TCanvas;
class TFile;

class DisplayAction : public IOCommand{
	public:
		explicit DisplayAction(UnpackerContext* context);
		virtual ~DisplayAction();
		
		int Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
		
		int beginSession(const std::string& sessionName);
		int processEvent(const EventData& event);
		int updateDisplays();
		int endSession();
	
	private:
		enum HistType{
			HIST_TH1D,
			HIST_TH2D
		};
	
	enum SourceMode{
		SRC_INVALID,
		SRC_CHANNEL,       // {module NAME channel}
		SRC_VALUE_VECTOR,  // {module NAME value}
		SRC_VALUE_FIXED,   // {module NAME value CH}
		SRC_SUM,           // {module NAME sum}
		SRC_MULTIPLICITY   // {module NAME multiplicity}
	};
	
	struct SourceSpec{
		std::string _module;
		SourceMode mode;
		int channel;
		
		SourceSpec() : mode(SRC_INVALID), channel(-1) {}
	};
	
	struct DisplaySpec{
		std::string name;
		HistType type;
		
		SourceSpec source;
		SourceSpec xsource;
		SourceSpec ysource;
		
		int bins;
		double minVal;
		double maxVal;
		
		int xbins;
		double xmin;
		double xmax;
		
		int ybins;
		double ymin;
		double ymax;
		
		std::set<int> runs;
		
		TH1D* h1;
		TH2D* h2;
		TCanvas* canvas;
		std::string drawOption;
		
		DisplaySpec() :
			type(HIST_TH1D),
			bins(100), minVal(0.0), maxVal(100.0),
			xbins(100), xmin(0.0), xmax(100.0),
			ybins(100), ymin(0.0), ymax(100.0),
			h1(0), h2(0), canvas(0), drawOption("") {}
	};
	
	struct ObjectCmdData{
		DisplayAction* self;
		std::string name;
	};
	
	std::map<std::string, DisplaySpec> m_displays;
	std::map<std::string, ObjectCmdData*> m_cmdData;
	std::string m_sessionName;
	
	int setError(Tcl_Interp* interp, const char* message);
	int createDisplay(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
	int objectCommand(Tcl_Interp* interp, const std::string& name, int objc, Tcl_Obj* const objv[]);
	
	int handleConfig(Tcl_Interp* interp, DisplaySpec& spec, int objc, Tcl_Obj* const objv[]);
	int handleAdd(Tcl_Interp* interp, DisplaySpec& spec, Tcl_Obj* listObj);
	int handleRemove(Tcl_Interp* interp, DisplaySpec& spec, Tcl_Obj* listObj);
	int handleClear(DisplaySpec& spec);
	int handleDraw(Tcl_Interp* interp, DisplaySpec& spec, int objc, Tcl_Obj* const objv[]);
	int handleWrite(Tcl_Interp* interp, DisplaySpec& spec, int objc, Tcl_Obj* const objv[]);
	
	int parseSource(Tcl_Interp* interp, Tcl_Obj* obj, SourceSpec& spec);
	int parseRange(Tcl_Interp* interp, Tcl_Obj* obj, double& lo, double& hi);
	int parseRunList(Tcl_Interp* interp, Tcl_Obj* obj, std::set<int>& runs, bool addMode);
	
	void ensureHistogram(DisplaySpec& spec);
	void deleteHistogram(DisplaySpec& spec);
	void fillTH1(DisplaySpec& spec, const EventData& event);
	void fillTH2(DisplaySpec& spec, const EventData& event);
	
	bool getScalar(const EventData& event, const SourceSpec& src, double& value) const;
	bool getVector(const EventData& event, const SourceSpec& src, std::vector<double>& values) const;
	
	static int ObjectCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]);
	static void DeleteObjectCmd(ClientData clientData);
};

#endif
