#ifndef MODULE_H
#define MODULE_H

// C++ includes
#include <iostream>
#include <iomanip>
#include <string>

// Unpacker includes
#include "EventData.h"

class Module{
	public:
		virtual ~Module(){}

		// defined by your modules, think of this as a place holder
		virtual bool decode(unsigned short*& point,
				    unsigned short* end,
				    EventData& event, 
				    const std::string& moduleName) = 0;

};
#endif
