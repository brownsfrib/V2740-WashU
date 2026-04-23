// Listen
// I understand that making an exact copy of a class
// and just changing the name feels redundant.
// I hear you
// What you don't understand, probably becuase you haven't met me,
// is that I am stupid.
// I am an idiot.
// This is my attempt at idiot proofing this unpacker

// You call the ADC module that you have, requiring some sort of thought
// on the user side, and then you forget about it.
// Sue me. it works and im moving on with my life


#ifndef ADCV785_H
#define ADCV785_H

// C++ includes
#include <iostream>
#include <cstdlib>
#include <iomanip>

// Unpacker includes
#include "Module.h"

#define CHANNELS 32
class ADCV785 : public Module{
	public:
		ADCV785();
		virtual ~ADCV785(){}
		
		unsigned short convertedChannels;
		unsigned short crate;
		unsigned short geoAddr;
		unsigned short channel[CHANNELS];
		unsigned short data[CHANNELS];
		unsigned short underflow[CHANNELS];
		unsigned short overflow[CHANNELS];
		
		unsigned short* parseEvent(unsigned short* point, unsigned short* end);
		bool readPhysicsEvent(unsigned short*& point, unsigned short* end);
		virtual bool decode(unsigned short*& point,
		unsigned short* end,
		EventData& event,
		const std::string& moduleName);
	
	private:
		void resetEventState();
		bool looksLikeHeader(unsigned short* point, unsigned short* end) const;
		int  peekSlot(unsigned short* point, unsigned short* end) const;
};

#endif
