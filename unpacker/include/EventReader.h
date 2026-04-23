#ifndef EVENTREADER_H
#define EVENTREADER_H

// C++ includes
#include <iostream>
#include <fstream>
#include <vector>

struct RingItemInfo{
	int nbytes;
	int type;
	RingItemInfo() : nbytes(0), type(-1){}
};

struct BodyHeaderInfo{
	bool hasHeader;
	unsigned long long timeStamp;
	unsigned int sourceID;
	unsigned int barrierType;

	BodyHeaderInfo() : hasHeader(false), timeStamp(0), sourceID(0), barrierType(0){}
};

struct EventBlock{
	RingItemInfo item;
	BodyHeaderInfo body;

	std::vector<unsigned short> storage;
	const unsigned short* payload;
	int payloadWords;

	EventBlock() : payload(0), payloadWords(0){}

};

class EventReader{
	public:
		EventReader() : m_fileSize(0){}

		// for actually reading the event
		bool open(const std::string& fileName);
		//bool nextEvent(std::vector<unsigned short>& buffer, int& type);
		bool nextEvent(EventBlock& block);
		void close();

		// for timing the event by approximating the payload of a file to only contain long long
		long long currentPosition();
		long long fileSize() const;

	private:
		bool parseBodyHeader(EventBlock& block);

		std::ifstream m_file;
		long long m_fileSize;
};
#endif

