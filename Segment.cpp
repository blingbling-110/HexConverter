#include"Segment.h"
#include"CrcCalculator.h"

void Segment::setStartAddress(size_t startAddr)
{
	this->startAddr = startAddr;
}

size_t Segment::getStartAddress()
{
	return this->startAddr;
}

void Segment::append(unsigned char* buffer, size_t length)
{
    while (length-- > 0) {
        this->data.push_back(*buffer++);
    }
}

size_t Segment::getLength()
{
	return this->data.size();
}

unsigned char* Segment::getFrontPointer()
{
	return &(this->data.front());
}

void Segment::calculateSegmentCrc(
	rsize_t width,
	unsigned long long poly,
	unsigned long long init,
	bool refin,
	bool refout,
	unsigned long long xorout)
{
	if (width <= 8)
	{
		this->crc = calculateCrc<unsigned char>(getFrontPointer(), getLength(), width, poly, init, refin, refout, xorout);
	}
	else if (width > 8 && width <= 16)
	{
		this->crc = calculateCrc<unsigned short>(getFrontPointer(), getLength(), width, poly, init, refin, refout, xorout);
	}
	else if (width > 16 && width <= 32)
	{
		this->crc = calculateCrc<unsigned int>(getFrontPointer(), getLength(), width, poly, init, refin, refout, xorout);
	}
	else if (width > 32 && width <= 64)
	{
		this->crc = calculateCrc<unsigned long long>(getFrontPointer(), getLength(), width, poly, init, refin, refout, xorout);
	}
}

unsigned long long Segment::getSegmentCrc()
{
	return this->crc;
}
