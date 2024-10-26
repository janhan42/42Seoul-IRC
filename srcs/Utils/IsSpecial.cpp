#include "IsSpecial.hpp"

bool	isSpecial(char c)
{
	if (c == '[' || c == ']' || c == '{' || c == '}' || c == '\\' || c == '|' || c == '_')
		return (true);
	return (false);
}
