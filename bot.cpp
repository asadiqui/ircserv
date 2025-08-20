/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnachit <mnachit@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:10:55 by mnachit           #+#    #+#             */
/*   Updated: 2025/08/15 16:10:57 by mnachit          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "bot.hpp"
#include "ServerSocket.hpp"
#include <iostream>


Bot::Bot( ServerSocket *server)
{
    this->server = server;
    nickname = "BotNick";
    username = "BotUser";
    realname = "BotReal";
    std::cout << "The irc bot is created\n";
}

Bot::~Bot()
{
    std::cout << "The irc bot is destroyed" << std::endl;
}
