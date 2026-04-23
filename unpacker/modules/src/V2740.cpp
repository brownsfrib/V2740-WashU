#include "../include/V2740.h"
// ---------------------------------------------------------------------------
// Layout decode() receives (uint16 offsets from point on entry):
//
//  parse() already consumed uint16[0] (built-event size lo) with its point++.
//  decode() therefore starts at uint16[1]:
//
//  [1]         built-event size hi          ← skip 1 to finish the uint32
//  [2..5]      CEventFragment#0 timestamp   ← first fragment begins here
//  ...
// ---------------------------------------------------------------------------


// ---------------------------------------------------------------------------
// CEventFragment layout (190 uint16 = 380 bytes total):
//
//  uint16 offset | uint32 word | content
//  ------------------------------------------------
//  [+0 .. +3]    | —           | timestamp uint64 (tsLo, tsHi)
//  [+4 .. +5]    | —           | sourceID         (skip)
//  [+6 .. +7]    | —           | payloadBytes     (skip)
//  [+8 .. +9]    | —           | barrier          (skip)
//  ---- ring item header (payload starts here) ----
//  [+10.. +11]   | —           | ring item size   (skip)
//  [+12.. +13]   | —           | ring item type   (skip)
//  ---- ring body header -------------------------
//  [+14.. +15]   | —           | body hdr size    (skip)
//  [+16.. +19]   | —           | body hdr ts lo+hi(skip)
//  [+20.. +21]   | —           | body hdr srcID   (skip)  <- note only 4 uint16 for ts
//  [+22.. +23]   | —           | body hdr barrier (skip)
//  ---- V2740 payload (uint32 words [0..9] then samples) --
//  [+24.. +25]   | word[0]     | payloadWords (expect 166 for nSamples = 64)
//  [+26.. +27]   | word[1]     | nameTag "adc1"
//  [+28.. +29]   | word[2]     | high16 = channel number
//  [+30.. +31]   | word[3]     | ns timestamp      (skip)
//  [+32.. +33]   | word[4]     | raw ts lo         (skip)
//  [+34.. +35]   | word[5]     | raw ts hi         (skip)
//  [+36.. +37]   | word[6]     | fine ts / flags   (skip)
//  [+38.. +39]   | word[7]     | low16 = DPP-PHA energy
//  [+40.. +41]   | word[8]     | reserved          (skip)
//  [+42.. +43]   | word[9]     | reserved          (skip)
//  [+44..]       | word[10..10+nSamples-1] | waveform samples (low16)
//  [+44+nSamples*2 .. +189]    | tail energy + padding zeros
// ---------------------------------------------------------------------------

bool V2740::decode(
		unsigned short*& point, unsigned short* end,
		EventData& event, const std::string& moduleName){

	if (end - point < 1){
		std::cerr << "V2740: buffer too short\n"; 
		return false; 
	}
	point += 1;
	
	bool anyDecoded = false;
	
	while (point + FRAGMENT_UINT16 <= end){
		uint32_t fragChannel, fragEnergy;
		uint64_t fragTimestamp;
		std::vector<uint32_t> fragSamples;
		
		if (!readFragment(point, end, fragChannel, fragTimestamp, fragEnergy, fragSamples)){
			break;
		}

		DigiHit hit;
		hit.channel 	= static_cast<int>(fragChannel);
		hit.energy	= static_cast<int>(fragEnergy);
		hit.timestamp	= static_cast<double>(fragTimestamp); // still in ns
		hit.samples.assign(fragSamples.begin(), fragSamples.end());
		event.addDigiHit(moduleName, hit);
		anyDecoded = true;
	}
	
	return anyDecoded;
}

bool V2740::readFragment(
		unsigned short*& point, unsigned short* end,
		uint32_t& outChannel, uint64_t& outFragTimestamp,
		uint32_t& outEnergy,  std::vector<uint32_t>& outSamples){

	if (point + FRAGMENT_UINT16 > end){
		return false;
	}
	
	unsigned short* const fragStart = point;
	
	// --- CEventFragment header [+0..+11] ---
	// timestamp(4) + srcID(2) + unknown(2) + payloadBytes(2) + barrier(2) = 12 uint16
	//uint32_t tsLo    = read32(point);               // [+0..+1]
	//uint32_t tsHi    = read32(point);               // [+2..+3]
	//outFragTimestamp = ((uint64_t)tsHi << 32) | tsLo; //time stamp from the NSCLDAQ glommer
	//skip16(point, 8);                               // [+4..+11]: srcID, unknown, payloadBytes, barrier

	skip16(point, 12);

	// --- Ring item header [+12..+15] ---
	skip16(point, 4);                               // ring size, ring type
	
	// --- Ring body header [+16..+25] ---
	// size(2) + timestamp(4) + srcID(2) + barrier(2) = 10 uint16
	skip16(point, 10);
	
	// --- V2740 payload starts at [+26] ---
	uint32_t payloadWords = read32(point);          // word[0]: payload size in uint16s
	
	if ((int)payloadWords != EXPECTED_PAYLOAD_WORDS){
		std::cerr << "V2740: unexpected payloadWords=" << payloadWords
			<< " (expected " << EXPECTED_PAYLOAD_WORDS << "), skipping fragment\n";
		point = fragStart + FRAGMENT_UINT16;
		return false;
	}
	
	skip16(point, 2);                               // word[1]: nameTag "adc1" (skip)
	uint32_t word2 = read32(point);            	// word[2]: high16 = channel number
	outChannel = (word2 >> 16) & 0xFFFF;
	uint32_t rawTimeLo = read32(point);		// word[3]: timestamp (low) (ns)
	uint32_t rawTimeHi = read32(point);		// word[4]: timestamp (high) (ns)
	skip16(point, 2); 				// reserved word? always zero

	outFragTimestamp = ((((uint64_t)(rawTimeHi & 0xFFFF) << 32) | rawTimeLo));  // given in ns

	//std::cout << "combined time: " << outFragTimestamp << '\t';
	//std::cout << "lo bits: " << rawTimeLo << '\t';
	//std::cout << "hi bits: " << rawTimeHi  << '\n';

	skip16(point, 2); 				// word[6]: fine time resolution (ns)
	uint32_t energyWord = read32(point);            // word[7]: high16 = DPP-PHA energy
	outEnergy  = (energyWord >> 16) & 0xFFFF;
	skip16(point, 4);                               // word[8..9]: reserved (skip)
	//uint32_t sampleCount = read32(point); // will always be the same as nSamples :)
	skip16(point, 2);

	// --- Waveform samples word[10..10+nSamples-1] ---
	outSamples.resize(nSamples);
	for (int i = 0; i < nSamples; i++){
		outSamples[i] = read32(point) & 0xFFFF;    // ADC value in low16
	}
	
	point = fragStart + FRAGMENT_UINT16;
	return true;
}


