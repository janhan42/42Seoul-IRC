#include "IsInt.hpp"
#include <limits>

/**
 * @brief
 * 들어온 const char* num을 strtol로 long 형태로 변환후
 * int범위 내에 있는지와 정상적인 인자인지 확인하는 함수
 * @param num 검사할 문자열
 * @return true (int형)
 * @return false (int형이 아니거나 정상적이지 않음)
 */
bool	isInt(const char* num)
{
	char*	endPtr;
	long	longValue;

	longValue = strtol(num, &endPtr, 10);
	if (static_cast<long>(std::numeric_limits<int>::min()) <= longValue
	&& longValue <= (static_cast<long>(std::numeric_limits<int>::max()))
	&& *endPtr == '\0')
		return true;
	else
		return false;
}
