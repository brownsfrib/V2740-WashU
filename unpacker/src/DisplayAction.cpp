#include "../include/DisplayAction.h"

#include <sstream>

#include "TH1D.h"
#include "TH2D.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TROOT.h"
#include "TSystem.h"
#include "TObject.h"

DisplayAction::DisplayAction(UnpackerContext* context) :
	IOCommand(context),
	m_sessionName("display")
{}

DisplayAction::~DisplayAction(){
	endSession();
	
	for (std::map<std::string, ObjectCmdData*>::iterator it = m_cmdData.begin(); it != m_cmdData.end(); it++){
		delete it->second;
	}
	m_cmdData.clear();
}

int DisplayAction::setError(Tcl_Interp* interp, const char* message){
	Tcl_SetResult(interp, const_cast<char*>(message), TCL_STATIC);
	return TCL_ERROR;
}

int DisplayAction::Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	if (objc < 3){
		return setError(interp, "display: too few arguments");
	}

	std::string subcmd = Tcl_GetString(objv[2]);
	if (subcmd == "create"){
		return createDisplay(interp, objc, objv);
	}
	
	return setError(interp, "display: unknown subcommand");
}

int DisplayAction::createDisplay(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	if (objc != 5){
		return setError(interp, "usage: lana display create TH1D|TH2D name");
	}
	
	std::string typeName = Tcl_GetString(objv[3]);
	std::string name     = Tcl_GetString(objv[4]);
	
	if (m_displays.find(name) != m_displays.end()){
		return setError(interp, "display: name already exists");
	}
	
	Tcl_CmdInfo info;
	if (Tcl_GetCommandInfo(interp, name.c_str(), &info)){
		return setError(interp, "display: Tcl command already exists");
	}
	
	DisplaySpec spec;
	spec.name = name;
	
	if (typeName == "TH1D"){
		spec.type = HIST_TH1D;
	}
	else if (typeName == "TH2D"){
		spec.type = HIST_TH2D;
		spec.drawOption = "COLZ";
	}
	else{
		return setError(interp, "display: supported types are TH1D and TH2D");
	}
	
	m_displays[name] = spec;
	
	ObjectCmdData* data = new ObjectCmdData;
	data->self = this;
	data->name = name;
	m_cmdData[name] = data;
	
	Tcl_CreateObjCommand(interp, name.c_str(), ObjectCmd, (ClientData)data, DeleteObjectCmd);
	return TCL_OK;
}

int DisplayAction::ObjectCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	ObjectCmdData* data = (ObjectCmdData*)clientData;
	return data->self->objectCommand(interp, data->name, objc, objv);
}

void DisplayAction::DeleteObjectCmd(ClientData clientData){
	(void)clientData;
}

int DisplayAction::objectCommand(Tcl_Interp* interp, const std::string& name, int objc, Tcl_Obj* const objv[]){
	std::map<std::string, DisplaySpec>::iterator it = m_displays.find(name);
	if (it == m_displays.end()){
		return setError(interp, "display object not found");
	}
	
	if (objc < 2){
		return setError(interp, "display object: missing subcommand");
	}
	
	DisplaySpec& spec = it->second;
	std::string subcmd = Tcl_GetString(objv[1]);
	
	if (subcmd == "config"){
		return handleConfig(interp, spec, objc, objv);
	}
	if (subcmd == "add"){
		if (objc != 3){
			return setError(interp, "usage: name add {runs}");
		}
		return handleAdd(interp, spec, objv[2]);
	}
	if (subcmd == "remove"){
		if (objc != 3){
			return setError(interp, "usage: name remove {runs}");
		}
		return handleRemove(interp, spec, objv[2]);
	}
	if (subcmd == "clear"){
		return handleClear(spec);
	}
	if (subcmd == "draw"){
		return handleDraw(interp, spec, objc, objv);
	}
	if (subcmd == "write"){
		return handleWrite(interp, spec, objc, objv);
	}
	
	return setError(interp, "display object: unknown subcommand");
}

int DisplayAction::handleConfig(Tcl_Interp* interp, DisplaySpec& spec, int objc, Tcl_Obj* const objv[]){
	for (int i=2; i < objc; i += 2){
		if (i + 1 >= objc){
			return setError(interp, "config: missing value");
		}
		
		std::string opt = Tcl_GetString(objv[i]);
		
		if (opt == "-source"){
			if (spec.type != HIST_TH1D) return setError(interp, "TH2D uses -xsource and -ysource");
			if (parseSource(interp, objv[i+1], spec.source) != TCL_OK) return TCL_ERROR;
		}
		else if (opt == "-xsource"){
			if (spec.type != HIST_TH2D) return setError(interp, "TH1D uses -source");
			if (parseSource(interp, objv[i+1], spec.xsource) != TCL_OK) return TCL_ERROR;
		}
		else if (opt == "-ysource"){
			if (spec.type != HIST_TH2D) return setError(interp, "TH1D uses -source");
			if (parseSource(interp, objv[i+1], spec.ysource) != TCL_OK) return TCL_ERROR;
		}
		else if (opt == "-bins"){
			if (Tcl_GetIntFromObj(interp, objv[i+1], &spec.bins) != TCL_OK) return TCL_ERROR;
		}
		else if (opt == "-range"){
			if (parseRange(interp, objv[i+1], spec.minVal, spec.maxVal) != TCL_OK) return TCL_ERROR;
		}
		else if (opt == "-xbins"){
			if (Tcl_GetIntFromObj(interp, objv[i+1], &spec.xbins) != TCL_OK) return TCL_ERROR;
		}
		else if (opt == "-xrange"){
			if (parseRange(interp, objv[i+1], spec.xmin, spec.xmax) != TCL_OK) return TCL_ERROR;
		}
		else if (opt == "-ybins"){
			if (Tcl_GetIntFromObj(interp, objv[i+1], &spec.ybins) != TCL_OK) return TCL_ERROR;
		}
		else if (opt == "-yrange"){
			if (parseRange(interp, objv[i+1], spec.ymin, spec.ymax) != TCL_OK) return TCL_ERROR;
		}
		else if (opt == "-drawopt"){
			spec.drawOption = Tcl_GetString(objv[i+1]);
		}
		else{
			return setError(interp, "config: unknown option");
		}
	}
	
	deleteHistogram(spec);
	return TCL_OK;
}

int DisplayAction::handleAdd(Tcl_Interp* interp, DisplaySpec& spec, Tcl_Obj* listObj){
	return parseRunList(interp, listObj, spec.runs, true);
}

int DisplayAction::handleRemove(Tcl_Interp* interp, DisplaySpec& spec, Tcl_Obj* listObj){
	return parseRunList(interp, listObj, spec.runs, false);
}

int DisplayAction::handleClear(DisplaySpec& spec){
	spec.runs.clear();
	if (spec.h1){
		spec.h1->Reset();
	}
	if (spec.h2){
		spec.h2->Reset();
	}
	return TCL_OK;
}

int DisplayAction::handleDraw(Tcl_Interp* interp, DisplaySpec& spec, int objc, Tcl_Obj* const objv[]){
	ensureHistogram(spec);
	
	std::string opt = spec.drawOption;
	if (objc == 3){
		opt = Tcl_GetString(objv[2]);
	}
	
	if (!spec.canvas){
		std::string cname = spec.name + "_canvas";
		spec.canvas = new TCanvas(cname.c_str(), spec.name.c_str(), 900, 700);
	}
	
	spec.canvas->cd();
	if (spec.type == HIST_TH1D && spec.h1){
		spec.h1->Draw(opt.c_str());
	}
	else if (spec.type == HIST_TH2D && spec.h2){
		spec.h2->Draw(opt.c_str());
	}
	spec.canvas->Modified();
	spec.canvas->Update();
	
	if (gSystem){
		gSystem->ProcessEvents();
	}
	
	return TCL_OK;
}

int DisplayAction::handleWrite(Tcl_Interp* interp, DisplaySpec& spec, int objc, Tcl_Obj* const objv[]){
	if (objc != 3){
		return setError(interp, "usage: name write file.root");
	}
	
	ensureHistogram(spec);
	
	std::string outName = Tcl_GetString(objv[2]);
	TFile outFile(outName.c_str(), "UPDATE");
	if (outFile.IsZombie()){
		return setError(interp, "could not open ROOT output file");
	}
	
	outFile.cd();
	if (spec.h1){
		spec.h1->Write("", TObject::kOverwrite);
	}
	if (spec.h2){
		spec.h2->Write("", TObject::kOverwrite);
	}
	outFile.Close();
	
	return TCL_OK;
}

int DisplayAction::parseRange(Tcl_Interp* interp, Tcl_Obj* obj, double& lo, double& hi){
	int count = 0;
	Tcl_Obj** elems = 0;
	
	if (Tcl_ListObjGetElements(interp, obj, &count, &elems) != TCL_OK){
		return TCL_ERROR;
	}
	if (count != 2){
		return setError(interp, "range must be {min max}");
	}
	
	if (Tcl_GetDoubleFromObj(interp, elems[0], &lo) != TCL_OK){
		return TCL_ERROR;
	}
	if (Tcl_GetDoubleFromObj(interp, elems[1], &hi) != TCL_OK){
		return TCL_ERROR;
	}
	
	return TCL_OK;
}

int DisplayAction::parseSource(Tcl_Interp* interp, Tcl_Obj* obj, SourceSpec& spec){
	int count = 0;
	Tcl_Obj** elems = 0;
	
	if (Tcl_ListObjGetElements(interp, obj, &count, &elems) != TCL_OK){
		return TCL_ERROR;
	}
	if (count < 3){
		return setError(interp, "source must be {module NAME ...}");
	}
	
	std::string kind = Tcl_GetString(elems[0]);
	if (kind != "module"){
		return setError(interp, "source must start with 'module'");
	}
	
	spec = SourceSpec();
	spec._module = Tcl_GetString(elems[1]);
	
	std::string mode = Tcl_GetString(elems[2]);
	
	if (mode == "channel" && count == 3){
		spec.mode = SRC_CHANNEL;
		return TCL_OK;
	}
	if (mode == "value" && count == 3){
		spec.mode = SRC_VALUE_VECTOR;
		return TCL_OK;
	}
	if (mode == "value" && count == 4){
		spec.mode = SRC_VALUE_FIXED;
		if (Tcl_GetIntFromObj(interp, elems[3], &spec.channel) != TCL_OK) return TCL_ERROR;
		return TCL_OK;
	}
	if (mode == "sum" && count == 3){
		spec.mode = SRC_SUM;
		return TCL_OK;
	}
	if (mode == "multiplicity" && count == 3){
		spec.mode = SRC_MULTIPLICITY;
		return TCL_OK;
	}
	
	return setError(interp, "unsupported source spec");
}

int DisplayAction::parseRunList(Tcl_Interp* interp, Tcl_Obj* obj, std::set<int>& runs, bool addMode){
	int count = 0;
	Tcl_Obj** elems = 0;
	
	if (Tcl_ListObjGetElements(interp, obj, &count, &elems) != TCL_OK){
		return TCL_ERROR;
	}
	
	for (int i=0; i < count; i++){
		int run = 0;
		if (Tcl_GetIntFromObj(interp, elems[i], &run) != TCL_OK){
			return TCL_ERROR;
		}
		if (addMode){
			runs.insert(run);
		}
		else{
			runs.erase(run);
		}
	}
	
	return TCL_OK;
}

void DisplayAction::ensureHistogram(DisplaySpec& spec){
	if (spec.type == HIST_TH1D && !spec.h1){
		spec.h1 = new TH1D(spec.name.c_str(), spec.name.c_str(), spec.bins, spec.minVal, spec.maxVal);
		spec.h1->SetDirectory(0);
	}
	if (spec.type == HIST_TH2D && !spec.h2){
		spec.h2 = new TH2D(spec.name.c_str(), spec.name.c_str(),
		spec.xbins, spec.xmin, spec.xmax,
		spec.ybins, spec.ymin, spec.ymax);
		spec.h2->SetDirectory(0);
	}
}

void DisplayAction::deleteHistogram(DisplaySpec& spec){
	if (spec.h1){
		delete spec.h1;
		spec.h1 = 0;
	}
	if (spec.h2){
		delete spec.h2;
		spec.h2 = 0;
	}
}

bool DisplayAction::getScalar(const EventData& event, const SourceSpec& src, double& value) const{
	if (!event.hasModule(src._module)){
		return false;
	}
	
	const std::map<int, int>& data = event.getModuleData(src._module);
	
	if (src.mode == SRC_VALUE_FIXED){
		std::map<int, int>::const_iterator it = data.find(src.channel);
		if (it == data.end()){
			return false;
		}
		value = (double)it->second;
		return true;
	}
	
	if (src.mode == SRC_SUM){
		double sum = 0.0;
		for (std::map<int, int>::const_iterator it = data.begin(); it != data.end(); it++){
			sum += (double)it->second;
		}
		value = sum;
		return true;
	}
	
	if (src.mode == SRC_MULTIPLICITY){
		value = (double)data.size();
		return true;
	}
	return false;
}

bool DisplayAction::getVector(const EventData& event, const SourceSpec& src, std::vector<double>& values) const{
	values.clear();
	if (!event.hasModule(src._module)){
		return false;
	}
	
	const std::map<int, int>& data = event.getModuleData(src._module);
	
	if (src.mode == SRC_CHANNEL){
		for (std::map<int, int>::const_iterator it = data.begin(); it != data.end(); it++){
			values.push_back((double)it->first);
		}
		return !values.empty();
	}
	
	if (src.mode == SRC_VALUE_VECTOR){
		for (std::map<int, int>::const_iterator it = data.begin(); it != data.end(); it++){
			values.push_back((double)it->second);
		}
		return !values.empty();
	}
	
	return false;
}

void DisplayAction::fillTH1(DisplaySpec& spec, const EventData& event){
	if (!spec.h1){
		return;
	}
	
	double scalar = 0.0;
	if (getScalar(event, spec.source, scalar)){
		spec.h1->Fill(scalar);
		return;
	}
	
	std::vector<double> values;
	if (getVector(event, spec.source, values)){
		for (size_t i=0; i < values.size(); i++){
			spec.h1->Fill(values[i]);
		}
	}
}

void DisplayAction::fillTH2(DisplaySpec& spec, const EventData& event){
	if (!spec.h2){
		return;
	}
	
	double x = 0.0;
	double y = 0.0;
	bool xScalar = getScalar(event, spec.xsource, x);
	bool yScalar = getScalar(event, spec.ysource, y);
	
	if (xScalar && yScalar){
		spec.h2->Fill(x, y);
		return;
	}
	
	std::vector<double> xv;
	std::vector<double> yv;
	bool xVector = getVector(event, spec.xsource, xv);
	bool yVector = getVector(event, spec.ysource, yv);
	
	if (xVector && yVector){
		size_t n = (xv.size() < yv.size()) ? xv.size() : yv.size();
		for (size_t i=0; i < n; i++){
			spec.h2->Fill(xv[i], yv[i]);
		}
	}
}

int DisplayAction::beginSession(const std::string& sessionName){
	m_sessionName = sessionName;
	
	for (std::map<std::string, DisplaySpec>::iterator it = m_displays.begin(); it != m_displays.end(); it++){
		ensureHistogram(it->second);
	}
	
	return TCL_OK;
}

int DisplayAction::processEvent(const EventData& event){
	for (std::map<std::string, DisplaySpec>::iterator it = m_displays.begin(); it != m_displays.end(); it++){
		DisplaySpec& spec = it->second;
		
		if (!spec.runs.empty() && spec.runs.find(event.getRunNumber()) == spec.runs.end()){
			continue;
		}
		
		ensureHistogram(spec);
		
		if (spec.type == HIST_TH1D){
			fillTH1(spec, event);
		}
		else{
			fillTH2(spec, event);
		}
	}
	
	return TCL_OK;
}

int DisplayAction::updateDisplays(){
	for (std::map<std::string, DisplaySpec>::iterator it = m_displays.begin(); it != m_displays.end(); it++){
	DisplaySpec& spec = it->second;
	
	if (!spec.canvas){
		continue;
	}
	
	spec.canvas->cd();
	
	if (spec.type == HIST_TH1D && spec.h1){
		spec.h1->Draw(spec.drawOption.c_str());
	}
	else if (spec.type == HIST_TH2D && spec.h2){
		spec.h2->Draw(spec.drawOption.c_str());
	}
	
	spec.canvas->Modified();
	spec.canvas->Update();
	}
	
	if (gSystem){
		gSystem->ProcessEvents();
	}
	
	return TCL_OK;
}

int DisplayAction::endSession(){
	return updateDisplays();
}
