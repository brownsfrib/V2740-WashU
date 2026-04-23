#ifndef V2740_H
#define V2740_H

// C++ includes
#include <iostream>
#include <iomanip>
#include <sstream>
#include <cstdint>
#include <vector>
#include <string>

// Unpacker includes
#include "Module.h"

class V2740 : public Module {
	public:
		V2740(int nSamples_=64, int nsPerSample_=8, int depth_=64) : 
			nSamples(nSamples_), nsPerSample(nsPerSample_), depth(depth_),
			FRAGMENT_UINT16(24 + (10 + nSamples_ + 9) * 2),
			EXPECTED_PAYLOAD_WORDS((10 + nSamples_ + 9) * 2){}

		virtual ~V2740() {}

		int nSamples;
		int nsPerSample;
		int depth;
		const int FRAGMENT_UINT16;
		const int EXPECTED_PAYLOAD_WORDS;
		bool readFragment(unsigned short*& point, unsigned short* end,
		uint32_t& outChannel, uint64_t& outFragTimestamp,
		uint32_t& outEnergy, std::vector<uint32_t>& outSamples);
		virtual bool decode(unsigned short*& point,
		unsigned short* end,
		EventData& event,
		const std::string& moduleName) override;

	private:
		static const int N_HEADER_WORDS = 10;
		
		inline uint32_t read32(unsigned short*& p){
			uint32_t lo = *p++;
			uint32_t hi = *p++;
			return lo | (hi << 16);
		}

		/*
		inline uint16_t read16(unsigned short*& p){
			uint16_t lo = *p++;
			uint16_t hi = *p++;
			return lo | (hi << 8);
		}
		*/

		inline void skip16(unsigned short*& p, int n){
			p += n; 
		}
};
#endif
