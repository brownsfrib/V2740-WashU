#include "../include/OutputAction.h"

OutputAction::OutputAction(UnpackerContext* context) : IOCommand(context), m_outputFile(0){
}

int OutputAction::setError(Tcl_Interp* interp, const char* message){
	Tcl_SetResult(interp, (char*)(message), TCL_STATIC);
	return TCL_ERROR;
}

int OutputAction::createFile(const std::string& fileName){
	std::ofstream out(fileName.c_str());
	if (!out){
		return TCL_ERROR;
	}
	out << "testing! wow! a real method!\n";
	out.close();
	return TCL_OK;
}

int OutputAction::Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
	if (objc < 4){
		return setError(interp, "output: too few arguments");
	}
	
	std::string subcmd = Tcl_GetString(objv[2]);
	
	if (subcmd == "storageArea"){
		if (objc != 4){
			return setError(interp, "usage: unpacker output storageArea <path>");
		}
		
		m_context->storageArea = Tcl_GetString(objv[3]);
		return TCL_OK;
	}

	if (subcmd == "SAMTEST"){
		std::string fileName = Tcl_GetString(objv[3]);
		if (createFile(fileName) != TCL_OK){
			std::string message = "could not create file " + fileName;
			return setError(interp, message.c_str());
		}
		return TCL_OK;
	}
	
	if (subcmd == "description"){
		int listCount = 0;
		Tcl_Obj** listV = 0;
		
		if (objc != 4){
			return setError(interp, "usage: unpacker output description {list}");
		}
		
		if (Tcl_ListObjGetElements(interp, objv[3], &listCount, &listV) != TCL_OK){
			return TCL_ERROR;
		}
		
		m_context->descriptions.clear();
		for (int i=0; i < listCount; i++){
			m_context->descriptions.push_back(Tcl_GetString(listV[i]));
		}
		
		return TCL_OK;
	}
	
	return setError(interp, "unknown output subcommand");
}

const std::string& OutputAction::getStorageArea() const{
	return m_context->storageArea;
}

const std::vector<std::string>& OutputAction::getDescriptions() const{
	return m_context->descriptions;
}


// runtime file ladies of the night
// we'll update these with actual file IO later 3/9/26
int OutputAction::beginRun(const std::string& fileName){
	(void)fileName;
	return TCL_OK;
}

int OutputAction::processEvent(const EventData& event){
	(void)event;
	return TCL_OK;
}

int OutputAction::endRun(){
	return TCL_OK;
}
