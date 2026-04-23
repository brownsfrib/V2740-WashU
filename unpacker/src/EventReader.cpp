#include "../include/EventReader.h"

bool EventReader::open(const std::string& fileName){
	close();
	m_file.open(fileName.c_str(), std::ios::binary);
	if (!m_file.is_open()){
		return false;
	}

	m_file.seekg(0, std::ios::end);
	m_fileSize = (long long)m_file.tellg();
	m_file.seekg(0, std::ios::beg);

	return true;
}

void EventReader::close(){
	if (m_file.is_open()){
		m_file.close();
	}
}

long long EventReader::currentPosition(){
	if (!m_file.is_open()){
		return 0;
	}
	return (long long)m_file.tellg();
}

long long EventReader::fileSize() const{
	return m_fileSize;
}

bool EventReader::parseBodyHeader(EventBlock& block){
	block.payload = 0;
	block.payloadWords = 0;

	if (block.storage.empty()){
		return true;
	}
	const unsigned short* point 	= &block.storage[0];
	const unsigned short* end	= point + block.storage.size();

	if ((point + 1) > end){
		return false;
	}

	unsigned short bodyHeaderSize 	= *point++;
	unsigned short bodyHeaderSize2 	= *point++;
	(void)bodyHeaderSize2;

	if (bodyHeaderSize == 0x14){ // type 20
		if ((point + 8) > end){
			return false;
		}
		block.body.hasHeader = true;
		
		unsigned long long t0 = (unsigned long long)(*point++);
		unsigned long long t1 = (unsigned long long)(*point++);
		unsigned long long t2 = (unsigned long long)(*point++);
		unsigned long long t3 = (unsigned long long)(*point++);
		
		block.body.timeStamp   = t0 | (t1 << 16) | (t2 << 32) | (t3 << 48);
		block.body.sourceID    = (unsigned int)(*point++);
		block.body.barrierType = (unsigned int)(*point++);
		
		block.payload = point;
		block.payloadWords = (int)(end - point);
		return true;
	}
	
	// no body header
	block.body.hasHeader = false;
	block.payload = point;
	block.payloadWords = (int)(end - point);

return true;

}

bool EventReader::nextEvent(EventBlock& block){
	unsigned short* point;
	const int hBufferWords = 4;
	const int hBufferBytes = 8;
	unsigned short hBuffer[hBufferWords];

	block = EventBlock();

	if (!m_file.is_open()){
		return false;
	}

	std::streampos headerPos = m_file.tellg();

	m_file.read((char*)hBuffer, hBufferBytes);
	point = hBuffer;

	if (m_file.gcount() != hBufferBytes){ // something's fucked up with the evt file
		return false;
	}

	int nbytes 	= *point++;
	int nbytes2 	= *point++;
	int itemType	= *point++;
	int type2	= *point++;
	(void)nbytes2;
	(void)type2;

	if (nbytes < 8){
		std::cout << "EventReader: bad ring item size " << nbytes << " at " << headerPos << '\n';
		return false;
	}

	int dBufferBytes = nbytes - 8;
	if (dBufferBytes < 0){
		return false;
	}
	int dBufferWords = (dBufferBytes+1)/2;
	block.storage.assign(dBufferWords, 0);

	if (dBufferBytes > 0){
		m_file.read((char*)(&block.storage[0]), dBufferBytes);
		if (m_file.gcount() != dBufferBytes){
			block.storage.clear();
			return false;
		}
	}

	block.item.nbytes 	= nbytes;
	block.item.type 	= itemType;

	if (!parseBodyHeader(block)){
		std::cout << "EventReader: bad body header for type " << itemType << '\n';
		return false;
	}
	

	return true;
}
