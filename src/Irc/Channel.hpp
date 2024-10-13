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
		/* OCCF */
		Channel(const std::string&, int);
		~Channel();
	private: // member variables
		// 채널이 가지고 있을 것 뭘까
		Bot*				mBot;
		std::vector<int>	mOperatorFdList;
		std::string			mChannelName;
		std::vector<int>	m
	private: // delete OCCF
		Channel();
		Channel(const Channel& rhs);
		Channel& operator=(const Channel& rhs);
};

#endif
