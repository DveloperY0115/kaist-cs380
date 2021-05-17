#ifndef IO_H
#define IO_H

#include <string>
#include <sstream>
#include <vector>
#include <iterator>

//! Utility function working similarily as
//! 'split' method in Python
template <typename T>
std::vector<T> split(const std::string& str, const char delim) {
	std::vector<T> result;

	std::stringstream iss(str);
	std::string item;

	while (std::getline(iss, item, delim)) {
		result.push_back(item);
	}

	return result;
}
#endif