#ifndef EVENTDATA_H
#define EVENTDATA_H

// C++ includes
#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <tcl.h>

// ROOT includes
#include "TSystem.h"
#include "TH1.h"
#include "TH2.h"

// For storing waveform data
struct DigiHit{
	int channel;
	int energy;
	double timestamp;
	std::vector<int> samples;
};

class EventData{
	public:
		EventData();
		~EventData();

		// Iterates over m_channelCounts and resets
		void clear();

		void setRunNumber(int run);
		int getRunNumber() const;
		void setEventIndex(int index);
		int getEventIndex() const;
		void setValue(const std::string& _module, int channel, int value);

		bool hasModule(const std::string& _module) const;
		bool hasValue(const std::string& _module, int channel) const;
		int getValue(const std::string& _module, int channel, int defaultValue = -10000) const;
		const std::map<int, int>& getModuleData(const std::string& _module) const;

		// multi hit storage (TDC1190 stuff)
		void addHit(const std::string& _module, int channel, int value);
		bool hasMultiHitModule(const std::string& _module) const;
		bool hasMultiHitData(const std::string& _module, int channel) const;
		const std::vector<int>& getMultiHitChannel(const std::string& _module, int channel) const;
		const std::map<int, std::vector<int>>& getMultiHitData(const std::string& _module) const;

		// Waveform storage (V2740 supported)
		void addDigiHit(const std::string& _module, const DigiHit& hit);
		bool hasDigiHits(const std::string& _module) const;
		const std::vector<DigiHit>& getDigiHits(const std::string& _module) const;

	private:
		int m_runNumber;
		int m_eventIndex;
		std::map<std::string, std::map<int, int>> m_data; // ADCV792 style
		std::map<std::string, std::map<int, std::vector<int>>> m_multiHit; //TDC1190 style
		std::map<std::string, std::vector<DigiHit>> m_digiHits;

};

#endif
