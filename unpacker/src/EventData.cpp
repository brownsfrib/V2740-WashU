#include "../include/EventData.h"

EventData::EventData() : m_runNumber(-1), m_eventIndex(-1){
}

EventData::~EventData(){
}

void EventData::clear(){
	m_runNumber = -1;
	m_eventIndex = -1;
	m_data.clear();
	m_multiHit.clear();
	m_digiHits.clear();
}

void EventData::setRunNumber(int run){
	m_runNumber = run;
}

void EventData::setEventIndex(int index){
	m_eventIndex = index;
}

void EventData::setValue(const std::string& _module, int channel, int value){
	m_data[_module][channel] = value;
}

int EventData::getRunNumber() const{
	return m_runNumber;
}

int EventData::getEventIndex() const{
	return m_eventIndex;
}

bool EventData::hasModule(const std::string& _module) const{
	return m_data.find(_module) != m_data.end();
}

bool EventData::hasValue(const std::string& _module, int channel) const{
	std::map<std::string, std::map<int, int> >::const_iterator modIt = m_data.find(_module);
	if (modIt == m_data.end()){
		return false;
	}

	return modIt->second.find(channel) != modIt->second.end();
}

int EventData::getValue(const std::string& _module, int channel, int defaultValue) const{
	std::map<std::string, std::map<int, int> >::const_iterator modIt = m_data.find(_module);
	if (modIt == m_data.end()){
		return defaultValue;
	}
	
	std::map<int, int>::const_iterator chIt = modIt->second.find(channel);
	if (chIt == modIt->second.end()){
		return defaultValue;
	}
	
	return chIt->second;
}

const std::map<int, int>& EventData::getModuleData(const std::string& _module) const{
	return m_data.find(_module)->second;
}


// Multi hit saving stuff
void EventData::addHit(const std::string& _module, int channel, int value){
	m_multiHit[_module][channel].push_back(value);
}

bool EventData::hasMultiHitModule(const std::string& _module) const{
	return m_multiHit.find(_module) != m_multiHit.end();
}

bool EventData::hasMultiHitData(const std::string& _module, int channel) const{
	std::map<std::string, std::map<int, std::vector<int>>>::const_iterator it = m_multiHit.find(_module);
	if (it == m_multiHit.end()){
		return false;
	}
	return it->second.find(channel) != it->second.end();
}

const std::vector<int>& EventData::getMultiHitChannel(const std::string& _module, int channel) const{
	static const std::vector<int> empty;
	std::map<std::string, std::map<int, std::vector<int>>>::const_iterator it = m_multiHit.find(_module);
	if (it == m_multiHit.end()){
		return empty;
	}
	std::map<int, std::vector<int>>::const_iterator channelIt = it->second.find(channel);
	if (channelIt == it->second.end()){
		return empty;
	}
	return channelIt->second;
}

const std::map<int, std::vector<int>>& EventData::getMultiHitData(const std::string& _module) const{
	static const std::map<int, std::vector<int>> empty;
	std::map<std::string, std::map<int, std::vector<int>>>::const_iterator it = m_multiHit.find(_module);
	if (it == m_multiHit.end()){
		return empty;
	}
	return it->second;
}


// Waveform saving stuff
void EventData::addDigiHit(const std::string& _module, const DigiHit& hit){
	m_digiHits[_module].push_back(hit);
}

bool EventData::hasDigiHits(const std::string& _module) const{
	return m_digiHits.find(_module) != m_digiHits.end();
}

const std::vector<DigiHit>& EventData::getDigiHits(const std::string& _module) const{
	static const std::vector<DigiHit> empty;
	std::map<std::string, std::vector<DigiHit>>::const_iterator it = m_digiHits.find(_module);
	if (it == m_digiHits.end()){
		return empty;
	}
	return it->second;
}
