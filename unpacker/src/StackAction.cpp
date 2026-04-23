#include "../include/StackAction.h"
#include "../modules/include/ADCV792.h"
#include "../modules/include/TDC1190.h"
#include "../modules/include/V2740.h"

static Module* makeModule(const ModuleEntry& entry){
    if (entry.classType == "ADCV792"){
        return new ADCV792;
    }

    if (entry.classType == "TDC1190"){
        int depth = 1;
        int referenceChannel = 2;
        int nchannels = 32;

        std::map<std::string, std::string>::const_iterator it;

        it = entry.params.find("depth");
        if (it != entry.params.end()){
            depth = std::atoi(it->second.c_str());
        }

        it = entry.params.find("referenceChannel");
        if (it != entry.params.end()){
            referenceChannel = std::atoi(it->second.c_str());
        }

        it = entry.params.find("nchannels");
        if (it != entry.params.end()){
            nchannels = std::atoi(it->second.c_str());
        }

        return new TDC1190(depth, referenceChannel, nchannels);
    }

    if (entry.classType == "V2740"){
	    int nSamples 	= 64;
	    int nsPerSample 	= 8;
	    int depth 		= 64;

	    std::map<std::string, std::string>::const_iterator it;
	    it = entry.params.find("recordsamples");
	    if (it != entry.params.end()){
		    nSamples = std::atoi(it->second.c_str());
	    }
	    it = entry.params.find("nsPerSample");
	    if (it != entry.params.end()){
		    nsPerSample = std::atoi(it->second.c_str());
	    }
	    it = entry.params.find("depth");
	    if (it != entry.params.end()){
		    depth = std::atoi(it->second.c_str());
	    }
	    return new V2740(nSamples, nsPerSample, depth);
    }

    return 0;
}

StackAction::StackAction(UnpackerContext* context) : IOCommand(context){
}

int StackAction::setError(Tcl_Interp* interp, const char* message){
	Tcl_SetResult(interp, (char*)(message), TCL_STATIC);
	return TCL_ERROR;
}

int StackAction::handleModule(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
        if (objc < 5){
                return setError(interp, "usage: unpacker stack module name classType ?-key value ...?");
        }

        ModuleEntry entry;
        entry.name = Tcl_GetString(objv[3]);
        entry.classType = Tcl_GetString(objv[4]);

        for (int i=5; i < objc; i+=2){
                if ((i + 1) >= objc){
                        return setError(interp, "stack module: missing value");
                }

                std::string key = Tcl_GetString(objv[i]);
                std::string value = Tcl_GetString(objv[i+1]);

                if ((!key.empty()) && (key[0] == '-')){
                        key = key.substr(1);
                }

                entry.params[key] = value;
        }

        entry._module = makeModule(entry);
        if (!entry._module){
                return setError(interp, "stack module: unsupported module class");
        }

        m_context->moduleStack.push_back(entry);
        return TCL_OK;
}

int StackAction::Handle(Tcl_Interp* interp, int objc, Tcl_Obj* const objv[]){
        if (objc < 3){
                return setError(interp, "stack: too few arguments");
        }

        std::string subcmd = Tcl_GetString(objv[2]);

        if (subcmd == "module"){
                return handleModule(interp, objc, objv);
        }

        return setError(interp, "stack: unknown subcommand");
}

