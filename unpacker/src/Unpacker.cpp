#include "../include/Unpacker.h"
#include "../include/InputAction.h"
#include "../include/StackAction.h"
#include "../include/TreeAction.h"
#include "../include/OutputAction.h"
#include "../include/EventReader.h"
#include "../include/EventParser.h"
#include "../include/DisplayAction.h"

// Including modules
#include "../modules/include/ADCV792.h"
#include "../modules/include/TDC1190.h"

/* Debugging method 
static void printEvent(const EventData& event, const std::vector<std::string>& moduleNames){
	std::cout << "Run " << event.getRunNumber() << "\tEvent " << event.getEventIndex() << '\n';
	for (size_t i=0; i < moduleNames.size(); i++){
		const std::string& name = moduleNames[i];
		if (!event.hasModule(name)){
			continue;
		}
		const std::map<int, int>& modData = event.getModuleData(name);
		std::cout << "\033[1;33m\t" << name << "\t size: " << modData.size() << "\033[0m\n";

		//for (std::map<int, int>::const_iterator it = modData.begin(); it != modData.end(); it++){
		//	std::cout << "Channel: " << it->first << '\t';
		//	std::cout << "Data: " << it->second << '\n';
		//}
	}
}
*/

Unpacker::Unpacker() : m_name(""){

}

Unpacker::~Unpacker(){
	std::map<std::string, IOCommand*>::iterator it;
	for (it = m_actions.begin(); it != m_actions.end(); it++){
		delete it->second;
	}
}

void Unpacker::setName(const std::string& name){
	m_name = name;
}

const std::string& Unpacker::getName() const{
	return m_name;
}

UnpackerContext& Unpacker::getContext(){
	return m_context;
}

void Unpacker::registerAction(const std::string& key, IOCommand* action){
	m_actions[key] = action;
}

IOCommand* Unpacker::getAction(const std::string& key){
	std::map<std::string, IOCommand*>::iterator it = m_actions.find(key);
	if (it == m_actions.end()){
		return 0;
	}
	return it->second;
}

int Unpacker::Act(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	// Put in a objc < 2 clause here - catch error with Tcl_SetResult
	if (objc < 2){
		Tcl_SetResult(interp, (char*)("class unpacker: missing section"), TCL_STATIC);
		return TCL_ERROR;
	}

	std::string key = Tcl_GetString(objv[1]);
	std::map<std::string, IOCommand*>::iterator it = m_actions.find(key);

	if (it == m_actions.end()){
		Tcl_SetResult(interp, (char*)("class unpacker: unknown section"), TCL_STATIC);
		return TCL_ERROR;
	}

	return it->second->Handle(interp, objc, objv);
}

int UnpackerInstanceCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	Unpacker* unpacker = (Unpacker*)clientData;
	return unpacker->Act(interp, objc, objv);
}

void DeleteUnpackerInstance(ClientData clientData){
	Unpacker* unpacker = (Unpacker*)clientData;
	delete unpacker;
}

int UnpackerFactoryCmd(ClientData clientData, Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	UnpackerRegistry* registry = (UnpackerRegistry*)(clientData);
	if (objc < 3){
		Tcl_WrongNumArgs(interp, 1, objv, "create unpacker_name");
		return TCL_ERROR;
	}

	std::string subcmd = Tcl_GetString(objv[1]);

	if (subcmd != "create"){
		Tcl_SetResult(interp, (char*)("unpacker: unknown command, expected 'create'"), TCL_STATIC);
		return TCL_ERROR;
	}

	const char* unpackerName = Tcl_GetString(objv[2]);

	// Prevent duplicate command names
	Tcl_CmdInfo info;
	if (Tcl_GetCommandInfo(interp, unpackerName, &info)){
		Tcl_SetResult(interp, (char*)("unpacker: command already exists"), TCL_STATIC);
		return TCL_ERROR;
	}

	Unpacker* unpacker = new Unpacker;
	unpacker->setName(unpackerName);

	unpacker->registerAction("input",  new InputAction(&unpacker->getContext()));
	unpacker->registerAction("stack",  new StackAction(&unpacker->getContext()));
	unpacker->registerAction("tree",   new TreeAction(&unpacker->getContext()));
	unpacker->registerAction("output", new OutputAction(&unpacker->getContext()));
	unpacker->registerAction("display", new DisplayAction(&unpacker->getContext()));

	if (registry){
		registry->lastCreated = unpacker;
	}

	Tcl_CreateObjCommand(
		interp,
		unpackerName,
		UnpackerInstanceCmd,
		(ClientData)unpacker,
		DeleteUnpackerInstance
	);

	return TCL_OK;
}


int Unpacker::processAll(){
	EventReader reader;
	EventParser parser;
	DisplayAction* display = dynamic_cast<DisplayAction*>(getAction("display"));
	if (display){
		if (display->beginSession("display.root") != TCL_OK){
			std::cerr << "Could not initialize display output\n";
			return TCL_ERROR;
		}
	}
	TreeAction* tree = dynamic_cast<TreeAction*>(getAction("tree"));
	
	for (size_t i=0; i < m_context.runs.size(); i++){
		std::time_t runStart = std::time(0);
		std::time_t lastPrint = runStart;
		int runNumber = std::atoi(m_context.runs[i].c_str());
		
		std::ostringstream fName;
		fName << m_context.stageArea << "/run-" << std::setw(4) << std::setfill('0') << runNumber << "-00.evt";
		
		std::string fileName = fName.str();
		
		if (!reader.open(fileName)){
			std::cerr << "Could not open event file: " << fileName << '\n';
			return TCL_ERROR;
		}

		if (tree){
                        if (tree->beginRun(runNumber, fileName) != TCL_OK){
                                std::cerr << "Could not initialize ROOT tree output for run " << runNumber << '\n';
                                reader.close();
                                return TCL_ERROR;
                        }
                }
		
		std::vector<unsigned short> buffer;

		EventBlock block;
		int failedEvents = 0;
		int eventIndex = 0;

		while (reader.nextEvent(block)){
			if ((!block.payload) || (block.payloadWords <= 0)){
				continue;
			}
			
			if (block.item.type == 1){
				continue;
			}
			
			if (block.item.type != 30){
				continue;
			}
			
			m_context.currentEvent.clear();
			m_context.currentEvent.setRunNumber(runNumber);

			if (parser.parse(block.payload, block.payloadWords, m_context, m_context.currentEvent)){
				m_context.physicsEvent++;
				eventIndex++;
				
				if (display){
					display->processEvent(m_context.currentEvent);
				}
				if (tree){
					tree->processEvent(m_context.currentEvent);
				}
			}
			else{
				failedEvents++;
			}
			
			if (((eventIndex % 5000) == 0) && (eventIndex > 0)){
				std::time_t now = std::time(0);
				
				if ((now - lastPrint) >= 2){
					long long pos   = reader.currentPosition();
					long long total = reader.fileSize();
					
					double frac = 0.0;
					if (total > 0){
						frac = (double)pos / (double)total;
					}
					
					double elapsed = std::difftime(now, runStart);
					double eta = 0.0;
					if (frac > 0.0){
						eta = elapsed * (1.0 - frac) / frac;
					}

					std::cout << std::fixed
					<< " Run " << std::setw(4) << runNumber << ":  "
					<< std::setw(5) << std::setprecision(1) << (100.0 * frac) << "%   "
					<< std::scientific << std::setprecision(2) << (double)eventIndex
					<< " physics events   "
					<< std::fixed << "\033[1;31m" << std::setw(4) << failedEvents
					<< " failed parser attempts\033[0m   "
					<< "\033[1;33mETA " << std::setw(6) << std::setprecision(1) << eta << " s\033[0m"
					<< "     \r" << std::flush;
					
					lastPrint = now;
				}
			}
			
			if ((display) && ((eventIndex % 5000) == 0) && (eventIndex > 0)){
				display->updateDisplays();
			}
		}
		
		if (display){
			display->updateDisplays();
		}
		if (tree){
			tree->endRun();
		}
		reader.close();

		std::cout << "\r" << std::string(120, ' ') << "\r" << std::flush;
		std::cout << std::fixed
		<< " Run " << std::setw(4) << runNumber << ":  "
		<< std::setw(5) << "100.0%   "
		<< std::scientific << std::setprecision(2) << (double)eventIndex
		<< " physics events   "
		<< std::fixed << "\033[1;31m" << std::setw(6) << failedEvents
		<< " failed parser attempts\033[0m   "
		<< "\033[1;32mComplete!\033[0m\n" << std::flush;

	}
	if (display){
		display->endSession();
	}
	return TCL_OK;
}
