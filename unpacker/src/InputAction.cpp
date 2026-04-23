#include "../include/InputAction.h"

InputAction::InputAction(UnpackerContext* context) : IOCommand(context){
}

int InputAction::setError(Tcl_Interp* interp, const char* message){
	Tcl_SetResult(interp, const_cast<char*>(message), TCL_STATIC);
	return TCL_ERROR;
}

int InputAction::Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
		if (objc < 3){
			return setError(interp, "input: too few arguments");
		}
	
	std::string subcmd = Tcl_GetString(objv[2]);
	
	if (subcmd == "stagearea"){
		if (objc != 4){
			return setError(interp, "usage: unpacker input stagearea <path>");
		}
		
		m_context->stageArea = Tcl_GetString(objv[3]);
		return TCL_OK;
	}
	
	if (subcmd == "add"){
		if (objc != 5){
			return setError(interp, "usage: unpacker input add runs {list}");
		}
		
		std::string target = Tcl_GetString(objv[3]);
		if (target != "runs"){
			return setError(interp, "input add only supports runs");
		}
		
		int listCount = 0;
		Tcl_Obj** listV = 0;
		if (Tcl_ListObjGetElements(interp, objv[4], &listCount, &listV) != TCL_OK){
			return TCL_ERROR;
		}
		
		m_context->runs.clear();
		for (int i=0; i < listCount; i++){
			m_context->runs.push_back(Tcl_GetString(listV[i]));
		}
		
		return TCL_OK;
		}
	
	return setError(interp, "unknown input subcommand");
}

const std::string& InputAction::getStagearea() const{
	return m_context->stageArea;
}

const std::vector<std::string>& InputAction::getRuns() const{
	return m_context->runs;
}
