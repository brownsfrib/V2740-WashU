#ifndef EVENTPARSER_H
#define EVENTPARSER_H

// C++ includes
#include <iostream>
#include <map>
#include <string>

// Unpacker includes
#include "UnpackerContext.h"
#include "EventData.h"
#include "Module.h"

class EventParser{
	public:
		bool parse(const unsigned short* data, int nWords, const UnpackerContext& context, EventData& event);
};
#endif
