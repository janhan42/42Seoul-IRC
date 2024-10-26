#include "Split.hpp"

std::vector<std::string> split(std::string str, char delimiter)
{
	std::vector<std::string> result;
	std::stringstream ss(str);
	std::string tok;

	while (getline(ss, tok, delimiter))
		result.push_back(tok);
	return (result);
}
