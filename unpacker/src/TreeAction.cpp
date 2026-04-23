#include "../include/TreeAction.h"

TreeAction::TreeAction(UnpackerContext* context) :
	IOCommand(context),
	m_file(0),
	m_tree(0),
	m_activeRun(-1),
	m_activeEvtFile(""){
}

TreeAction::~TreeAction(){
	endRun();
	
	for (std::map<std::string, ObjectCmdData*>::iterator it = m_cmdData.begin(); it != m_cmdData.end(); it++){
		delete it->second;
	}
	m_cmdData.clear();
}

int TreeAction::setError(Tcl_Interp* interp, const char* message){
	Tcl_SetResult(interp, const_cast<char*>(message), TCL_STATIC);
	return TCL_ERROR;
}

int TreeAction::Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	if (objc < 3){
		return setError(interp, "tree: too few arguments");
	}
	
	std::string subcmd = Tcl_GetString(objv[2]);
	if (subcmd == "create"){
		return createTree(interp, objc, objv);
	}
	
	return setError(interp, "tree: unknown subcommand");
}

int TreeAction::createTree(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	if (objc != 4){
		return setError(interp, "usage: {unpacker_name} tree create name");
	}
	
	std::string name = Tcl_GetString(objv[3]);
	
	if (!m_spec.objectName.empty()){
		return setError(interp, "tree: only one tree object supported in this version");
	}
	
	Tcl_CmdInfo info;
	if (Tcl_GetCommandInfo(interp, name.c_str(), &info)){
		return setError(interp, "tree: Tcl command already exists");
	}
	
	m_spec.objectName = name;
	m_spec.treeName   = name;
	
	ObjectCmdData* data = new ObjectCmdData;
	data->self = this;
	data->name = name;
	m_cmdData[name] = data;
	
	Tcl_CreateObjCommand(interp, name.c_str(), ObjectCmd, (ClientData)data, DeleteObjectCmd);
	return TCL_OK;
}

int TreeAction::ObjectCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	ObjectCmdData* data = (ObjectCmdData*)clientData;
	return data->self->objectCommand(interp, data->name, objc, objv);
}

void TreeAction::DeleteObjectCmd(ClientData clientData){
	(void)clientData;
}

int TreeAction::objectCommand(Tcl_Interp* interp, const std::string& name, int objc, Tcl_Obj* const objv[]){
	if (name != m_spec.objectName){
		return setError(interp, "tree object not found");
	}
	
	if (objc < 2){
		return setError(interp, "tree object: missing subcommand");
	}
	
	std::string subcmd = Tcl_GetString(objv[1]);
	
	if (subcmd == "config"){
		return handleConfig(interp, objc, objv);
	}
	if (subcmd == "branch"){
		return handleModule(interp, objc, objv);
	}
	if (subcmd == "wavebranch"){
		return handleWaveBranch(interp, objc, objv);
	}
	if (subcmd == "map"){
		return handleMap(interp, objc, objv);
	}
	if (subcmd == "add"){
		if (objc != 3){
			return setError(interp, "usage: name add {runs}");
		}
		return handleAdd(interp, objv[2]);
	}
	if (subcmd == "remove"){
		if (objc != 3){
			return setError(interp, "usage: name remove {runs}");
		}
		return handleRemove(interp, objv[2]);
	}
	if (subcmd == "clear"){
		return handleClear();
	}
	
	return setError(interp, "tree object: unknown subcommand");
}

int TreeAction::handleConfig(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	for (int i=2; i < objc; i+=2){
		if (i + 1 >= objc){
			return setError(interp, "config: missing value");
		}
	
		std::string opt = Tcl_GetString(objv[i]);
		
		if (opt == "-title"){
			m_spec.title = Tcl_GetString(objv[i+1]);
		}
		else if (opt == "-description"){
			m_spec.descriptionTemplate = Tcl_GetString(objv[i+1]);
		}
		else if (opt == "-outdir"){
			m_spec.outDir = Tcl_GetString(objv[i+1]);
		}
		else{
			return setError(interp, "config: unknown option");
		}
	}
	
	return TCL_OK;
}

int TreeAction::handleModule(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	if (objc < 3){
		return setError(interp, "usage: treeName branch moduleName ?-name branchName? ?-channels N? ?-default X?");
	}
	
	BranchSpec spec;
	spec.moduleName = Tcl_GetString(objv[2]);
	spec.branchName = spec.moduleName;   // default: branch name == EventData key
	spec.channels = 32;
	spec.defaultValue = -1.0;
	spec.depth = 1.0;
	
	for (int i=3; i < objc; i += 2){
		if ((i + 1)>= objc){
			return setError(interp, "branch: missing value");
		}
		
		std::string opt = Tcl_GetString(objv[i]);
		
		if (opt == "-channels"){
			if (Tcl_GetIntFromObj(interp, objv[i+1], &spec.channels) != TCL_OK){
				return TCL_ERROR;
			}
		}
		else if (opt == "-default"){
			if (Tcl_GetDoubleFromObj(interp, objv[i+1], &spec.defaultValue) != TCL_OK){
				return TCL_ERROR;
			}
		}
		else if (opt == "-name"){
			// Override the ROOT branch name independently of the EventData lookup key
			spec.branchName = Tcl_GetString(objv[i+1]);
		}
		else if (opt == "-channel"){
			// For waveform modules: which physical channel to pull from.
			// The EventData lookup key becomes moduleName + "_ch" + N.
			if (Tcl_GetIntFromObj(interp, objv[i+1], &spec.sourceChannel) != TCL_OK){
				return TCL_ERROR;
			}
		}
		else if (opt == "-depth"){
			if (Tcl_GetIntFromObj(interp, objv[i+1], &spec.depth) != TCL_OK){
				return TCL_ERROR;
			}
		}
		else{
			return setError(interp, "branch: unknown option");
		}
	}
	
	for (size_t i=0; i < m_spec.branches.size(); i++){
		if (m_spec.branches[i].branchName == spec.branchName){
			m_spec.branches[i] = spec;
			return TCL_OK;
		}
	}
	
	m_spec.branches.push_back(spec);
	return TCL_OK;
}

int TreeAction::handleWaveBranch(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	if (objc < 3){
		return setError(interp, "usage: treeName wavebranch moduleName -name branchName -nchannels N -nsamples M ?-default X?");
	}

	WaveBranchSpec spec;
	spec.moduleName  	= Tcl_GetString(objv[2]);
	spec.branchName  	= spec.moduleName;
	spec.depth 		= 64;
	spec.nSamples    	= 64;
	spec.defaultValue 	= 0.0;

	for (int i=3; i < objc; i += 2){
		if ((i + 1) >= objc){
			return setError(interp, "wavebranch: missing value");
		}
		std::string opt = Tcl_GetString(objv[i]);
		if (opt == "-name"){
			spec.branchName = Tcl_GetString(objv[i+1]);
		}
		else if (opt == "-depth"){
			if (Tcl_GetIntFromObj(interp, objv[i+1], &spec.depth) != TCL_OK){return TCL_ERROR;}
		}
		else if (opt == "-nsamples"){
			if (Tcl_GetIntFromObj(interp, objv[i+1], &spec.nSamples) != TCL_OK){return TCL_ERROR;}
		}
		else if (opt == "-default"){
			if (Tcl_GetDoubleFromObj(interp, objv[i+1], &spec.defaultValue) != TCL_OK){return TCL_ERROR;}
		}
		else{
			return setError(interp, "wavebranch: unknown option");
		}
	}

	for (size_t i=0; i < m_waveBranches.size(); i++){
		if (m_waveBranches[i].branchName == spec.branchName){
			m_waveBranches[i] = spec;
			return TCL_OK;
		}
	}
	m_waveBranches.push_back(spec);
	return TCL_OK;
}

int TreeAction::handleMap(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	if (objc != 5){
		return setError(interp, "usage: treeName map mapName detector {module channel}");
	}
	
	MapSpec spec;
	spec.mapName = Tcl_GetString(objv[2]);
	spec.detName = Tcl_GetString(objv[3]);
	
	//if (Tcl_GetIntFromObj(interp, objv[3], &spec.detName) != TCL_OK){
	//	return TCL_ERROR;
	//}
	
	int listCount = 0;
	Tcl_Obj** listV = 0;
	
	if (Tcl_ListObjGetElements(interp, objv[4], &listCount, &listV) != TCL_OK){
		return TCL_ERROR;
	}
	
	if (listCount != 2){
		return setError(interp, "map source must be {module channel}");
	}
	
	spec._module = Tcl_GetString(listV[0]);
	if (Tcl_GetIntFromObj(interp, listV[1], &spec.channel) != TCL_OK){
		return TCL_ERROR;
	}
	
	for (size_t i=0; i < m_maps.size(); i++){
		if ((m_maps[i].mapName == spec.mapName) && (m_maps[i].detName == spec.detName)){
			m_maps[i] = spec;
			return TCL_OK;
		}
	}
	
	m_maps.push_back(spec);
	return TCL_OK;
}

int TreeAction::handleAdd(Tcl_Interp* interp, Tcl_Obj* listObj){
	return parseRunList(interp, listObj, m_spec.runs, true);
}

int TreeAction::handleRemove(Tcl_Interp* interp, Tcl_Obj* listObj){
	return parseRunList(interp, listObj, m_spec.runs, false);
}

int TreeAction::handleClear(){
	m_spec.runs.clear();
	m_spec.branches.clear();
	m_buffers.clear();
	m_waveBranches.clear();
	m_waveCounters.clear();
	m_waveChannelBufs.clear();
	m_waveEnergyBufs.clear();
	m_waveTimestampBufs.clear();
	m_waveWaveBufs.clear();

	m_maps.clear();
	return TCL_OK;
}

int TreeAction::parseRunList(Tcl_Interp* interp, Tcl_Obj* obj, std::set<int>& runs, bool addMode){
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

int TreeAction::ensureOutputDirectory(const std::string& path){
	if (path.empty()){
		return TCL_OK;
	}
	
	void* dirPtr = gSystem->OpenDirectory(path.c_str());
	if (dirPtr){
		gSystem->FreeDirectory(dirPtr);
		return TCL_OK;
	}
	
	int rc = gSystem->mkdir(path.c_str(), true);
	return (rc == 0) ? TCL_OK : TCL_ERROR; // damn these look sick
}

std::string TreeAction::buildOutputFileName(int runNumber) const{
	std::ostringstream out;
	out << m_spec.outDir
		<< "/run-"
		<< std::setw(4) << std::setfill('0') << runNumber
		<< "-"
		<< m_spec.objectName
		<< ".root";
	return out.str();
}

std::string TreeAction::resolveDescription(int runNumber) const{
	std::string desc = m_spec.descriptionTemplate;
	
	std::ostringstream runText;
	runText << runNumber;
	
	size_t pos = std::string::npos;
	while ((pos = desc.find("%RUN%")) != std::string::npos){
		desc.replace(pos, 5, runText.str());
	}
	
	return desc;
}

void TreeAction::resetBuffers(){
	for (size_t i=0; i < m_spec.branches.size(); i++){
		const BranchSpec& spec = m_spec.branches[i];
		std::vector<double>& buf = m_buffers[spec.branchName];
		std::fill(buf.begin(), buf.end(), spec.defaultValue);
		if (spec.depth > 1){
			m_hitCounters[spec.branchName] = 0;
		}
	}

	// Waveform recording branches
	for (size_t i=0; i < m_waveBranches.size(); i++){
		const WaveBranchSpec& wspec = m_waveBranches[i];
		std::string name = wspec.branchName;
		m_waveCounters[name] = 0;
		std::fill(m_waveChannelBufs[name].begin(), m_waveChannelBufs[name].end(), -1);
		std::fill(m_waveEnergyBufs[name].begin(), m_waveEnergyBufs[name].end(), wspec.defaultValue);
		std::fill(m_waveTimestampBufs[name].begin(), m_waveTimestampBufs[name].end(), wspec.defaultValue);
		std::fill(m_waveWaveBufs[name].begin(), m_waveWaveBufs[name].end(), wspec.defaultValue);
	}
}

void TreeAction::buildBranches(){
	if (!m_tree){
		return;
	}
	
	m_buffers.clear();
	
	for (size_t i=0; i < m_spec.branches.size(); i++){
		const BranchSpec& spec = m_spec.branches[i];
		std::vector<double>& buf = m_buffers[spec.branchName];
		buf.assign(spec.channels, spec.defaultValue);
		
		std::ostringstream leaf;
		if (spec.depth > 1){
			std::string counterName = "n" + spec.branchName + "Hits";
			int& counter = m_hitCounters[spec.branchName];
			counter = 0;
			m_tree->Branch(counterName.c_str(), &counter, (counterName + "/I").c_str());

			buf.assign(spec.channels*spec.depth, spec.defaultValue);
			leaf << spec.branchName << "[" << counterName << "][" << spec.channels << "]/D";
		}
		else{
			buf.assign(spec.channels, spec.defaultValue);
			leaf << spec.branchName << "[" << spec.channels << "]/D";
		}
		
		m_tree->Branch(spec.branchName.c_str(), &buf[0], leaf.str().c_str());
	}


	for (size_t i = 0; i < m_waveBranches.size(); i++){
		const WaveBranchSpec& wspec = m_waveBranches[i];
		std::string name = wspec.branchName;
		std::string counterName = "n" + name + "Hits";
		
		int& counter = m_waveCounters[name];
		counter = 0;
		m_tree->Branch(counterName.c_str(), &counter, (counterName + "/I").c_str());
		
		std::vector<int>& chBuf = m_waveChannelBufs[name];
		chBuf.assign(wspec.depth, -1);
		std::ostringstream chLeaf;
		chLeaf << name << "Channel[" << counterName << "]/I";
		m_tree->Branch((name + "Channel").c_str(), &chBuf[0], chLeaf.str().c_str());
		
		std::vector<double>& enBuf = m_waveEnergyBufs[name];
		enBuf.assign(wspec.depth, wspec.defaultValue);
		std::ostringstream enLeaf;
		enLeaf << name << "Energy[" << counterName << "]/D";
		m_tree->Branch((name + "Energy").c_str(), &enBuf[0], enLeaf.str().c_str());
		
		std::vector<double>& tsBuf = m_waveTimestampBufs[name];
		tsBuf.assign(wspec.depth, wspec.defaultValue);
		std::ostringstream tsLeaf;
		tsLeaf << name << "Timestamp[" << counterName << "]/D";
		m_tree->Branch((name + "Timestamp").c_str(), &tsBuf[0], tsLeaf.str().c_str());
		
		std::vector<double>& waveBuf = m_waveWaveBufs[name];
		waveBuf.assign(wspec.depth * wspec.nSamples, wspec.defaultValue);
		std::ostringstream waveLeaf;
		waveLeaf << name << "[" << counterName << "][" << wspec.nSamples << "]/D";
		m_tree->Branch(name.c_str(), &waveBuf[0], waveLeaf.str().c_str());
	}
}

int TreeAction::writeMappings(){
	if (!m_file){
		return TCL_OK;
	}
	
	if (m_maps.empty()){
		return TCL_OK;
	}
	
	m_file->cd();
	
	TTree mapTree("DetMap", "Detector mapping to module channels");
	
	char moduleName[128];
	char detectorName[128];
	int channel = -1;
	
	mapTree.Branch("detector", detectorName , "detector/C");
	mapTree.Branch("module", moduleName, "module/C");
	mapTree.Branch("channel", &channel, "channel/I");
	
	for (size_t i=0; i < m_maps.size(); i++){
		std::snprintf(moduleName, sizeof(moduleName), "%s", m_maps[i].mapName.c_str());
		std::snprintf(detectorName, sizeof(detectorName), "%s", m_maps[i].detName.c_str());
		channel = m_maps[i].channel;
		mapTree.Fill();
	}
	
	mapTree.Write("", TObject::kOverwrite);
	return TCL_OK;
}

int TreeAction::beginRun(int runNumber, const std::string& evtFileName){
	if (!m_spec.runs.empty() && m_spec.runs.find(runNumber) == m_spec.runs.end()){
		return TCL_OK;
	}
	
	if (ensureOutputDirectory(m_spec.outDir) != TCL_OK){
		return TCL_ERROR;
	}
	
	m_activeRun = runNumber;
	m_activeEvtFile = evtFileName;
	
	std::string outName = buildOutputFileName(runNumber);
	m_file = new TFile(outName.c_str(), "RECREATE");
	if (!m_file || m_file->IsZombie()){
		return TCL_ERROR;
	}
	
	m_tree = new TTree(m_spec.treeName.c_str(), m_spec.title.c_str());
	buildBranches();
	
	std::string desc = resolveDescription(runNumber);
	TNamed description("description", desc.c_str());
	description.Write();
	
	TNamed sourceFile("source_evt_file", evtFileName.c_str());
	sourceFile.Write();
	
	if (writeMappings() != TCL_OK){
		return TCL_ERROR;
	}
	
	return TCL_OK;
}

int TreeAction::processEvent(const EventData& event){
	if (!m_tree){
		return TCL_OK;
	}
	
	resetBuffers();
	
	for (size_t i=0; i < m_spec.branches.size(); i++){
		const BranchSpec& spec = m_spec.branches[i];

		std::string lookupKey = spec.moduleName;
		if (spec.sourceChannel >= 0){
			std::ostringstream oss;
			oss << spec.moduleName << "_ch" << spec.sourceChannel;
			lookupKey = oss.str();
		}

		std::vector<double>& buf = m_buffers[spec.branchName];

		if (event.hasMultiHitModule(lookupKey)){
			const std::map<int, std::vector<int>>& mhData = event.getMultiHitData(lookupKey);

			if (spec.depth > 1){ // multi valued events
				int maxHits = 0;
				for (std::map<int, std::vector<int>>::const_iterator it = mhData.begin(); it != mhData.end(); it++){
					int channel = it->first;
					if ((channel < 0) || (channel >= spec.channels)){
						continue;
					}
					int n = std::min((int)it->second.size(), spec.depth);
					if (n > maxHits){
						maxHits = n;
					}
				}
				m_hitCounters[spec.branchName] = maxHits;

				for (std::map<int, std::vector<int>>::const_iterator it = mhData.begin(); it != mhData.end(); it++){
					int channel = it->first;
					if ((channel < 0) || (channel >= spec.channels)){
						continue;
					}
					const std::vector<int>& hits = it->second;
					int nHits = std::min((int)hits.size(), spec.depth);
					if (spec.depth > 1){
						// 2D layout: buf[ch * depth + hitIndex]
						for (int h=0; h < nHits; h++){
							buf[h*spec.channels + channel] = (double)hits[h];
						}
					}
				}
			}
			else{ // single valued readout (like an ADC)
				for (std::map<int, std::vector<int>>::const_iterator it = mhData.begin(); it != mhData.end(); it++){
					int channel = it->first;
					if ((channel < 0) || (channel >= spec.channels)){
						continue;
					}
					buf[channel] = (double)it->second[0];
				}
			}
			continue;
		}
		
		const std::map<int, int>& data = event.getModuleData(lookupKey);
		for (std::map<int, int>::const_iterator it = data.begin(); it != data.end(); it++){
			int ch = it->first;
			int val = it->second;
			
			if ((ch < 0) || (ch >= spec.channels)){
				continue;
			}
			
			buf[ch] = (double)val;
		}
	}
	
	// Fill waveform branches
	for (size_t i=0; i < m_waveBranches.size(); i++){
		const WaveBranchSpec& wspec = m_waveBranches[i];
		std::string name = wspec.branchName;
		
		if (!event.hasDigiHits(wspec.moduleName)){
			continue;
		}
		
		const std::vector<DigiHit>& hits = event.getDigiHits(wspec.moduleName);
		int nHits = std::min((int)hits.size(), wspec.depth);
		m_waveCounters[name] = nHits;
		
		std::vector<int>& chBuf       = m_waveChannelBufs[name];
		std::vector<double>& enBuf    = m_waveEnergyBufs[name];
		std::vector<double>& tsBuf       = m_waveTimestampBufs[name];
		std::vector<double>& waveBuf  = m_waveWaveBufs[name];
		
		for (int h=0; h < nHits; h++){
			chBuf[h] = hits[h].channel;
			enBuf[h] = (double)hits[h].energy;
			tsBuf[h] = hits[h].timestamp;
			
			int n = std::min((int)hits[h].samples.size(), wspec.nSamples);
			for (int s=0; s < n; s++){
				waveBuf[h * wspec.nSamples + s] = (double)hits[h].samples[s];
			}
		}
	}

	m_tree->Fill();
	return TCL_OK;
}

int TreeAction::endRun(){
	if (m_file){
		m_file->cd();
		if (m_tree){
			m_tree->Write("", TObject::kOverwrite);
		}
		m_file->Close();
		delete m_file;
		m_file = 0;
	}
	
	m_tree = 0;
	m_buffers.clear();
	m_activeRun = -1;
	m_activeEvtFile = "";
	
	return TCL_OK;
}
