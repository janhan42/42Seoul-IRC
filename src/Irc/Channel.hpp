#ifndef CHANNEL_HPP
# define CHANNEL_HPP

/**
 * @file Channel.hpp
 * @author jang hun han (janhan@student.42Seoul.kr)
 * @brief Channel의 관한 선언 및 매크로
 * @version 0.1
 * @date 2024-10-13
 *
 * @copyright Copyright (c) 2024
 *
 */

#include "../../common.hpp"

class Channel
{
	public:
		Channel(const std::string&, int);
		~Channel();
	private: // member variables


	private: // delete OCCF
		Channel();
		Channel(const Channel& rhs);
		Channel& operator=(const Channel& rhs);
};

#endif
