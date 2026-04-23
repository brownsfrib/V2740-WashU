#ifndef TDC1190_H
#define TDC1190_H

// C++ includes
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <vector>
#include <map>

// Unpacker includes
#include "Module.h"

using namespace std;

struct tdcStuff{
	int channel;
	int time;
	int order;
};

class TDC1190 : public Module{
	public:
		TDC1190(int depth, int referenceChannel, int Nchannels);
		virtual ~TDC1190();
		
		unsigned short* parseEvent(unsigned short* point, unsigned short* end);
		bool readPhysicsEvent(unsigned short*& point, unsigned short* end);
		
		virtual bool decode(unsigned short*& point,
		unsigned short* end,
		EventData& event,
		const std::string& moduleName);
		
		int getNdata() const;
		tdcStuff getHit(int i) const;
	
	private:
		void readGlobalHeader(unsigned short word1, unsigned short word2);
		void readTDCheader(unsigned short word1, unsigned short word2);
		void readTDCmeasurement(unsigned short word1, unsigned short word2);
		void readTDCtrailer(unsigned short word1, unsigned short word2);
		void readTDCerror(unsigned short word1, unsigned short word2);
		void readTrailer(unsigned short word1, unsigned short word2);
		
		void resetEventState();
		bool looksLikeGlobalHeader(unsigned short* point, unsigned short* end) const;
		int  peekGeo(unsigned short* point, unsigned short* end) const;
		
		int eventCount;
		int geographic;
		int bunchID;
		int eventID;
		int channel;
		int data;
		int leading;
		int wordCount;
		int errorFlag;
		int TDC;
		int status;
		int depth;
		int referenceChannel;
		int referenceTime;
		int Nchannels;
		int* order;
		tdcStuff* dataOut;
		int Ndata;
};

#endif
