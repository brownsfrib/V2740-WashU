#include "../include/EventParser.h"

bool EventParser::parse(const unsigned short* data, int nWords, const UnpackerContext& context, EventData& event){
	unsigned short* point = const_cast<unsigned short*>(data);
	unsigned short* end   = point + nWords;

	if (point >= end){
		//std::cout << "Parser: empty physics payload\n";
		return false;
	}

	// keep this, we need the event level skip
	// I need to figure this out - it might bite me in the ass if we use a different event build sink
	point++;

	for (size_t i = 0; i < context.moduleStack.size(); i++){
		const ModuleEntry& entry = context.moduleStack[i];

		if (!entry._module){
			//std::cout << "Parser: null module for " << entry.name << '\n';
			return false;
		}

		if (point >= end){
			//std::cout << "Parser: ran out of words before " << entry.name << '\n';
			return false;
		}

		if (!entry._module->decode(point, end, event, entry.name)){
			//std::cout << "Parser: failed module decode for " << entry.name << '\n';
			return false;
		}
	}

	return true;
}
