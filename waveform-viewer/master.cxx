// 3/4/26
// Trying to connect to ringbuffers so we can get live waveform viewing

// C/C++ includes
#include <iostream>
#include <iomanip>
#include <vector>
#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>

// NSCLDAQ includes - 12.-018
#include <CDataSource.h>
#include <CDataSourceFactory.h>
#include <CRingItem.h>
#include <CPhysicsEventItem.h>
#include <DataFormat.h>
#include <Exception.h>

// ROOT includes
#include "TSystem.h"
#include "TApplication.h"
#include "TCanvas.h"
#include "TH1.h"
#include "TGraph.h"


void parseItem(CRingItem* item, TGraph** graphs){
	if (item->type() != 0x1e) { // not a physics event
		return;
	}
	
	CPhysicsEventItem event = CPhysicsEventItem(*item);
	const uint32_t* words   = (uint32_t*)event.getBodyPointer();
	const size_t bodyBytes  = (size_t)event.getBodySize();
	const size_t wordCount  = bodyBytes / sizeof(uint32_t);
	
	//std::cout << "\033[1;33mEVENT " << counter << "\033[0m\n";
	//std::cout << "wordCount: " << wordCount << '\n';
	//std::cout << "words:\n";
	
	if (wordCount == 0){
		return;
	}
	
	// Built-event body starts with a 32-bit size in bytes.
	uint32_t builtBodyBytes = words[0];
	size_t builtBodyWords = builtBodyBytes / 4;
	
	if (builtBodyWords > wordCount){
		builtBodyWords = wordCount; // safety clamp
	}
	
	//std::cout << "\nBuilt body bytes: " << builtBodyBytes << "\n";
	
	// after the size word, the remainder is a sequence of fragments
	size_t pos = 1;
	int fragIndex = 0;
	
	const size_t bins = 1024;
	while (pos + 5 <= builtBodyWords){
		const uint32_t* frag = words + pos;
		
		// fragment header (20 bytes total)
		uint64_t fragmentTimestamp 	= ((uint64_t)frag[1] << 32) | (uint64_t)frag[0];
		uint32_t fragmentSourceId    	= frag[2];
		uint32_t fragmentPayloadBytes 	= frag[3];
		uint32_t fragmentBarrier     	= frag[4];
		
		uint32_t fragmentBytes = 20 + fragmentPayloadBytes;
		if ((fragmentBytes % 4) != 0){
			std::cout << "Fragment " << fragIndex << ": non-word-aligned fragment size, stopping.\n";
			break;
		}
		
		size_t fragWords = fragmentBytes/4;
		if (pos + fragWords > builtBodyWords){
			std::cout << "Fragment " << fragIndex << ": truncated fragment, stopping.\n";
			break;
		}
		
		if (fragWords < 24){
			std::cout << "Fragment " << fragIndex << ": too small for embedded payload parse.\n";
			pos += fragWords;
			++fragIndex;
			continue;
		}
		
		uint32_t ringItemSize   = frag[5];
		uint32_t ringItemType   = frag[6];
		uint32_t bodyHeaderSize = frag[7];
		
		uint64_t eventTimestamp = ((uint64_t)frag[9] << 32) | (uint64_t)frag[8];
		uint32_t eventSourceId = frag[10];
		uint32_t eventBarrier  = frag[11];
		uint32_t moduleTagRaw = frag[13];
		uint16_t channelNumber = (uint16_t)((frag[14] >> 16) & 0xFFFF);
		char m0 = (char)( moduleTagRaw        & 0xFF);
		char m1 = (char)((moduleTagRaw >>  8) & 0xFF);
		char m2 = (char)((moduleTagRaw >> 16) & 0xFF);
		char m3 = (char)((moduleTagRaw >> 24) & 0xFF);
		

		// Check that this is correct? I don't think it is
		// Maybe just pass nSamples as a parameter
		uint32_t nSamples = frag[22];
		if (23 + nSamples > fragWords){
			nSamples = (fragWords > 23) ? (fragWords - 23) : 0;
		}
		
		graphs[channelNumber]->Set(0);
		for (uint32_t i = 0; i < nSamples; i++){
			uint16_t sample = (uint16_t)(frag[23 + i] & 0xFFFF);
			graphs[channelNumber]->SetPoint(i, i, sample);
		}
		
		pos += fragWords;
		++fragIndex;
	}
	
	//std::cout << "\nTotal fragments parsed: " << fragIndex << "\n";
}


int main(int argc, char** argv){
	if (argc < 2){
		std::cerr << "\033[1;31mError - Usage: " << argv[0] << " tcp://hostname/ringname\033[0m\n";
		return 1;
	}

	std::vector<std::uint16_t> sample;
	std::vector<std::uint16_t> exclude;
	
	CDataSource* pDataSource = 0;
	
	try{ // check if the ring is alive
		pDataSource = CDataSourceFactory::makeSource(argv[1], sample, exclude);
	}
	catch (CException& e){
		std::cerr << "\033[1;31mError - Failed to open ring source\033[0m\n";
		return 1;
	}
	catch (...){
		std::cerr << "\033[1;31mError - Failed to open ring source\033[0m\n";
		return 1;
	}

	
	int channelCount = 16;
	TApplication app("app", &argc, argv);
	TCanvas can("can", "", 1000, 800);
	can.Divide(4,4);
	TGraph** graphs = new TGraph*[channelCount]; 
	TH1F* frames[channelCount];
	for (int i=0; i < channelCount; i++){
		graphs[i] = new TGraph();
		graphs[i]->GetXaxis()->SetTitle("Time buckets");
		graphs[i]->GetXaxis()->CenterTitle(true);
		graphs[i]->GetYaxis()->SetTitle("ADC units");
		graphs[i]->GetYaxis()->CenterTitle(true);
		std::string title = "Channel: " + std::to_string(i);
		graphs[i]->SetPoint(0, 0, 0);
		graphs[i]->SetTitle(title.c_str());

		can.cd(i+1);
		gPad->SetGrid();
		frames[i] = gPad->DrawFrame(0.1, 0.1, 512, 4096, 
				Form("Channel %d;Time Buckets; ADC Units", i));
		graphs[i]->Draw("L SAME");
	}
	can.Modified();
	can.Update();

	while (true){
		CRingItem* pItem = 0;
		
		try{ // get the blocks for the live ring 
			pItem = pDataSource->getItem();
		}
		catch (CException& e){
			std::cerr << "\033[1;31mError reading ring item!\033[0m\n";
			break;
		}
		catch (...){
			std::cerr << "\033[1;31mError reading ring item!\033[0m\n";
			break;
		}
		
		std::unique_ptr<CRingItem> item(pItem);

		parseItem(pItem, graphs);
		for (int ch=0; ch < channelCount; ch++){
			can.cd(ch+1);
			if (graphs[ch]->GetN() != 0){
				can.Modified();
				can.Update();
				gPad->Modified();
				gPad->Update();
			}
		}
		gSystem->ProcessEvents();
	}
	app.Run();

	delete pDataSource; // clean up memory holding data source 
	for (int i=0; i < channelCount; i++){
		delete graphs[i];
	}
	delete[] graphs;
	
	
	return 0;
}
