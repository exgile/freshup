#include "packet.h"
#include <fstream> 
#include <Poco/LocalDateTime.h>

Packet::Packet() : length_(0), buffer_size_(kInitialSize), buffer_(new unsigned char[kInitialSize]()){}

Packet::~Packet() {
	delete[] buffer_;
}

void Packet::write_hex(const unsigned char* hex, int size){
	if (length_ + size >= buffer_size_) {
		buffer_ = (unsigned char*)realloc(buffer_, buffer_size_ + size);
		buffer_size_ += size;
	}

	for (int i = 0; i < size; ++i){
		buffer_[length_] = hex[i];
		++length_;
	}
}

void Packet::write_time() {
	if (buffer_) {
		Poco::LocalDateTime now;

		if (length_ + 16 >= buffer_size_) {
			buffer_ = (unsigned char*)realloc(buffer_, buffer_size_ + 16);
			buffer_size_ += 16;
		}


		*(unsigned __int16*)(buffer_ + length_) = now.year();
		*(unsigned __int16*)(buffer_ + length_ + 2) = now.month();
		*(unsigned __int16*)(buffer_ + length_ + 4) = now.dayOfWeek();
		*(unsigned __int16*)(buffer_ + length_ + 6) = now.day();
		*(unsigned __int16*)(buffer_ + length_ + 8) = now.hour();
		*(unsigned __int16*)(buffer_ + length_ + 10) = now.minute();
		*(unsigned __int16*)(buffer_ + length_ + 12) = now.second();
		*(unsigned __int16*)(buffer_ + length_ + 14) = now.millisecond();

		length_ += 16;
	}
}

void Packet::write_null(size_t amount){
	if (length_ + amount >= buffer_size_) {
		buffer_ = (unsigned char*)realloc(buffer_, buffer_size_ + amount);
		buffer_size_ += amount;
	}

	for (; amount > 0; --amount)
	{
		buffer_[length_] = 0;
		++length_;
	}
}


void Packet::write_string(std::string str, std::size_t slen){
	if (length_ + slen >= buffer_size_)
	{
		buffer_ = (unsigned char*)realloc(buffer_, buffer_size_ + slen);
		buffer_size_ += slen;
	}

	memcpy(buffer_ + length_, str.data(), str.length());

	size_t len = str.length();
	length_ += len;

	for (; len < slen; ++len)
	{
		buffer_[length_] = 0;
		++length_;
	}
}

unsigned char *Packet::get_buffer(){
	return buffer_;
}

size_t Packet::get_length(){
	return length_;
}

void Packet::reset() {
	delete[] buffer_;
	buffer_ = new unsigned char[kInitialSize]();
	length_ = 0;
	buffer_size_ = kInitialSize;
}

void Packet::SaveToFile(std::string fileName) {
	uofstream  uofstream(fileName, std::ofstream::binary);
	uofstream.write(buffer_, length_);
	uofstream.close();
}