/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sangshin <zxcv1867@naver.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/10/15 18:12:54 by sangshin          #+#    #+#             */
/*   Updated: 2024/10/15 18:19:20 by sangshin         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <string>

// server class 만들기 전까지만

int main(void)
{
	int			port = 6667;
	std::string passwd = "1234";

	// input_check(port, passwd);

	Server server(port, passwd);
	server.run();

	return (0);
}
