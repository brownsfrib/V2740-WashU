#include "../include/TDC1190.h"

// ---------------------------------------------------------------------------
// Word orientation note
// ---------------------------------------------------------------------------
// The V1190 produces 32-bit words.  In the FRIBDAQ/NSCLDAQ evt stream these
// are stored as two consecutive uint16s in BIG-ENDIAN order:
//
//   stream[0] = HIGH 16 bits  (contains the word-type tag in bits 31:27)
//   stream[1] = LOW  16 bits
//
// Every read* helper therefore reconstructs the 32-bit value as:
//   uint32_t v = ((uint32_t)word1 << 16) | word2;
// where word1 = *point++ (high) and word2 = *point++ (low).
//
// V1190 word-type tags (bits 31:27 of the 32-bit word):
//   0x08  Global header
//   0x00  TDC measurement
//   0x01  TDC header
//   0x03  TDC trailer
//   0x04  TDC error
//   0x10  Global trailer
//   0x11  Trigger time tag
//   0x18  Filler
// ---------------------------------------------------------------------------

TDC1190::TDC1190(int depth0, int referenceChannel0, int Nchannels0){
	depth            = depth0;
	referenceChannel = referenceChannel0;
	Nchannels        = Nchannels0;
	order   = new int[Nchannels];
	dataOut = new tdcStuff[Nchannels * depth];
	resetEventState();
}

TDC1190::~TDC1190(){
	delete[] order;
	delete[] dataOut;
}

void TDC1190::resetEventState(){
	eventCount     = 0;
	geographic     = 0;
	bunchID        = 0;
	eventID        = 0;
	channel        = 0;
	data           = 0;
	leading        = 0;
	wordCount      = 0;
	errorFlag      = 0;
	TDC            = 0;
	status         = 0;
	referenceTime  = 0;
	Ndata          = 0;
	
	for (int i = 0; i < Nchannels; i++){
		order[i] = 0;
	}
	for (int i = 0; i < Nchannels*depth; i++){
		dataOut[i].channel = -1;
		dataOut[i].time    = 0;
		dataOut[i].order   = 0;
	}
}

// ---------------------------------------------------------------------------
// Peek helpers
// ---------------------------------------------------------------------------

bool TDC1190::looksLikeGlobalHeader(unsigned short* point, unsigned short* end) const{
	if (point + 1 >= end){
		return false;
	}
	uint32_t v = ((uint32_t)point[0] << 16) | point[1];
	return (((v >> 27) & 0x1f) == 0x08);
}

int TDC1190::peekGeo(unsigned short* point, unsigned short* end) const{
	if (!looksLikeGlobalHeader(point, end)){
		return -1;
	}
	uint32_t v = ((uint32_t)point[0] << 16) | point[1];
	return (int)((v >> 22) & 0x1f);
}

// ---------------------------------------------------------------------------
// Word readers  (word1 = HIGH, word2 = LOW)
// ---------------------------------------------------------------------------

void TDC1190::readGlobalHeader(unsigned short word1, unsigned short word2){
	uint32_t v = ((uint32_t)word2 << 16) | word1;
	if (((v >> 27) & 0x1f) != 0x08){
		cout << "TDC1190 global header not found" << endl;
		abort();
	}
	geographic = (v >> 22) & 0x1f;
	eventCount =  v        & 0x3fffff;
	
	Ndata = 0;
	for (int i = 0; i < Nchannels; i++){
		order[i] = 0;
	}
}

void TDC1190::readTDCheader(unsigned short word1, unsigned short word2){
	uint32_t v = ((uint32_t)word2 << 16) | word1;
	if (((v >> 27) & 0x1f) != 0x01){
		cout << "TDC1190 TDC header not found" << endl;
	}
	TDC     = (v >> 24) & 0x3;
	eventID = (v >> 12) & 0xfff;
	bunchID =  v        & 0xfff;
}

void TDC1190::readTDCmeasurement(unsigned short word1, unsigned short word2){
	uint32_t v = ((uint32_t)word2 << 16) | word1;
	if (((v >> 27) & 0x1f) != 0x00){
		cout << "TDC1190: not a measurement word" << endl;
	}
	leading = (v >> 26) & 0x1;
	channel = (v >> 19) & 0x7f;
	data    =  v        & 0x7ffff;
	
	if (channel >= Nchannels){
		return;
	}
	
	if (channel == referenceChannel && order[channel] == 0){
		referenceTime = data;
	}
	
	if (order[channel] < depth){
		dataOut[Ndata].time    = data;
		dataOut[Ndata].channel = channel;
		dataOut[Ndata].order   = order[channel];
		Ndata++;
	}
	order[channel]++;
}

void TDC1190::readTDCtrailer(unsigned short word1, unsigned short word2){
	uint32_t v = ((uint32_t)word2 << 16) | word1;
	if (((v >> 27) & 0x1f) != 0x03){
		cout << "TDC1190: not a TDC trailer" << endl;
	}
	TDC       = (v >> 24) & 0x3;
	eventID   = (v >> 12) & 0xfff;
	wordCount =  v        & 0xfff;
}

void TDC1190::readTDCerror(unsigned short word1, unsigned short word2){
	uint32_t v = ((uint32_t)word2 << 16) | word1;
	if (((v >> 27) & 0x1f) != 0x04){
		cout << "TDC1190: not a TDC error word" << endl;
	}
	errorFlag = v & 0x7fff;
}

void TDC1190::readTrailer(unsigned short word1, unsigned short word2){
	uint32_t v = ((uint32_t)word2 << 16) | word1;
	if (((v >> 27) & 0x1f) != 0x10){
		cout << "TDC1190: not a global trailer" << endl;
	}
	
	int geo = (v >> 22) & 0x1f;
	if (geo != geographic){
		cout << "TDC1190: geographic address in trailer doesn't match header" << endl;
	}
	
	status    = (v >> 24) & 0x7;
	wordCount =  v        & 0x7ff;
	
	for (int i = 0; i < Ndata; i++){
		dataOut[i].time -= referenceTime;
	}
}

unsigned short* TDC1190::parseEvent(unsigned short* point, unsigned short* end){
	unsigned short word_lo = *point++;
	unsigned short word_hi = *point++;
	readGlobalHeader(word_lo, word_hi);
	
	for (;;){
		word_lo = *point++;
		word_hi = *point++;
		uint32_t v     = ((uint32_t)word_hi << 16) | word_lo;
		unsigned short wt = (v >> 27) & 0x1f;
		
		// Did I just write a goto line? yep
		// this is the first time in my life I've done this and I gotta say
		// I'm kinda converted it looks sick as shit
		switch (wt){
			case 0x10:  readTrailer(word_lo, word_hi);        goto done;
			case 0x00:  readTDCmeasurement(word_lo, word_hi); break;
			case 0x01:  readTDCheader(word_lo, word_hi);      break;
			case 0x03:  readTDCtrailer(word_lo, word_hi);     break;
			case 0x04:  readTDCerror(word_lo, word_hi);       break;
			case 0x11:  break;
			case 0x18:  break;
			default:
			cout << "TDC1190: unknown word type 0x" << hex << wt << dec << endl;
			break;
		}
	}
	done:
	return point;
}

bool TDC1190::readPhysicsEvent(unsigned short*& point, unsigned short* end){
	resetEventState();
	// 4x ffff = no TDC data this event
	if (
		(point[0] == 0xffff) && 
		(point[1] == 0xffff) &&
		(point[2] == 0xffff) && 
		(point[3] == 0xffff)
	){
		point += 4;
		return true;
	}
	
	// 2x ffff = leading sentinel left by previous module trailing check
	if ((point[0] == 0xffff) && (point[1] == 0xffff)){
		point += 2;
	}
	
	point = parseEvent(point, end);
	
	unsigned short e0 = *point++;
	unsigned short e1 = *point++;
	if ((e0 != 0xffff) || (e1 != 0xffff)){
		//cout << "TDC1190: expected trailing sentinels, got 0x"
		//     << hex << e0 << " 0x" << e1 << dec << endl;
		return false;
	}
	
	return true;
}

int TDC1190::getNdata() const{
	return Ndata;
}

tdcStuff TDC1190::getHit(int i) const{
	return dataOut[i];
}

// ---------------------------------------------------------------------------
// decode  (called by EventParser)
// ---------------------------------------------------------------------------
bool TDC1190::decode(unsigned short*& point, unsigned short* end,
                     EventData& event, const std::string& moduleName){
	if (!readPhysicsEvent(point, end)){
		return false;
	}
	
	for (int j = 0; j < getNdata(); j++){
		tdcStuff hit = getHit(j);
		//event.setValue(moduleName, hit.channel, hit.time);
		event.addHit(moduleName, hit.channel, hit.time); // multi event recording!!!
	}
	
	return true;
}
