#pragma once
#include <exception>
#include <iostream>

struct ChannelNotFound : public std::exception {
	const char * what() const throw () {
		return "pc channel not found";
	}
};