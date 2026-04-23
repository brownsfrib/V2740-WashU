#include "../include/ADCV862.h"
using namespace std;

ADCV862::ADCV862(){ 
	resetEventState();
}

void ADCV862::resetEventState(){
	convertedChannels = 0;
	crate    = 0;
	geoAddr  = 0;
	for (int i=0; i < CHANNELS; i++){
		channel[i]   = 0;
		data[i]      = 0;
		underflow[i] = 0;
		overflow[i]  = 0;
	}
}

bool ADCV862::looksLikeHeader(unsigned short* point, unsigned short* end) const{
	if (point + 1 >= end){
		return false;
	}
	return (point[1] & 0x0200) != 0;
}

int ADCV862::peekSlot(unsigned short* point, unsigned short* end) const{
	if (point + 1 >= end){
		return -1;
	}
	return (point[1] >> 11);
}

unsigned short* ADCV862::parseEvent(unsigned short* point, unsigned short*){
	// Each 32-bit V862 hardware word is stored as two uint16: LO first, HI second.
	// Header:
	//   LO (word0): bits[15:8] = number of converted channels
	//   HI (word1): bits[15:11] = geoAddr, bits[9:3] = crate, bit[9] = header marker
	//
	// Data words (per channel):
	//   LO (word0): bits[11:0] = ADC value, bit[13] = underflow, bit[12] = overflow
	//   HI (word1): bits[4:0]  = channel number
	//               bit[10]    = EOB marker
	//               bit[9]     = invalid/out-of-range (skip)
	//
	// EOB: LO ignored, HI bit[10] = 1
	
	unsigned short header_lo = *point++;
	unsigned short header_hi = *point++;
	
	convertedChannels = header_lo >> 8;
	geoAddr           = header_hi >> 11;
	crate             = (header_hi >> 3) & 0x7F;
	
	int j = 0;
	for (int i=0; i < convertedChannels + 2; i++){
		unsigned short data_lo = *point++;
		unsigned short data_hi = *point++;
		
		if ((data_hi >> 10) & 0x1){
			point -= 2;
			convertedChannels = j;
			break;
		}
		if ((data_hi >> 9) & 0x1){
			continue;
		}
		
		channel[j]   = data_hi & 0x1F;
		data[j]      = data_lo & 0xFFF;
		underflow[j] = (data_lo >> 13) & 0x1;
		overflow[j]  = (data_lo >> 12) & 0x1;
		j++;
	}
	convertedChannels = j;
	
	// EOB: 2 uint16, bit10 of HI must be set
	unsigned short eob_lo = *point++;
	unsigned short eob_hi = *point++;
	(void)eob_lo;
	(void)eob_hi;
	return point;
}

bool ADCV862::readPhysicsEvent(unsigned short*& point, unsigned short* end){
	resetEventState();
	// 4-word no-blocking block
	if (point[0] == 0xffff && point[1] == 0xffff && 
	    point[2] == 0xffff && point[3] == 0xffff){
		point += 4;
		return true;
	}

	if (point[0] == 0xffff && point[1] == 0xffff){
		point += 2;
	}
	
	point = parseEvent(point, nullptr);
	
	// Trailing 2x 0xffff sentinels
	if (point[0] != 0xffff || point[1] != 0xffff){
		return false;
	}
	return true;
}

bool ADCV862::decode(unsigned short*& point, unsigned short* end, EventData& event, const std::string& moduleName){
	if (!readPhysicsEvent(point, end)){
		return false;
	}
	for (int j = 0; j < convertedChannels; j++){
		event.setValue(moduleName, channel[j], data[j]);
	}
	return true;
}
