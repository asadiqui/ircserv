/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnachit <mnachit@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/07/19 21:19:16 by mnachit           #+#    #+#             */
/*   Updated: 2025/07/19 22:19:44 by mnachit          ###   ########.fr       */
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

