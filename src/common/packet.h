#pragma once
#include <iostream>
#include "consts.h"

typedef std::basic_ofstream<unsigned char, std::char_traits<unsigned char> > uofstream;

class Packet{ 

private:
	unsigned char *buffer_;
	std::size_t length_;
	std::size_t buffer_size_;

public:
	Packet();
	~Packet();
	void write_hex(const unsigned char* hex, int size);
	void write_null(size_t amount);
	void write_string(std::string str, std::size_t slen);
	void write_time();
	void SaveToFile(std::string fileName);
	void reset();
	unsigned char *get_buffer();
	size_t get_length();

	template<typename TYPE>
	void write(TYPE value){
		if (length_ + sizeof(TYPE) >= buffer_size_){
			buffer_ = (unsigned char*)realloc(buffer_, buffer_size_ + kIncreaseSize);
			buffer_size_ += kIncreaseSize;
		}

		*(TYPE*)(buffer_ + length_) = value;
		length_ += sizeof(TYPE);
	}

	template<>
	void write<bool>(bool value)
	{
		if (length_ + sizeof(bool) >= buffer_size_){
			buffer_ = (unsigned char*)realloc(buffer_, buffer_size_ + kIncreaseSize);
			buffer_size_ += kIncreaseSize;
		}

		buffer_[length_] = (value ? 1 : 0);
		++length_;
	}

	template<>
	void write<std::string>(std::string str){
		size_t len = str.length();
		if (length_ + len >= buffer_size_){
			buffer_ = (unsigned char*)realloc(buffer_, buffer_size_ + kIncreaseSize);
			buffer_size_ += kIncreaseSize;
		}

		write<short>(static_cast<short>(len));

		memcpy(buffer_ + length_, str.c_str(), len);
		length_ += len;
	}

};