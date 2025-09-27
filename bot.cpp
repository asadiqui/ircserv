/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.cpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: asadiqui <asadiqui@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:10:55 by mnachit           #+#    #+#             */
/*   Updated: 2025/09/27 20:48:16 by asadiqui         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "bot.hpp"
#include "ServerSocket.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>


Bot::Bot( ServerSocket *server)
{
    this->server = server;
    nickname = "BotNick";
    username = "BotUser";
    realname = "BotReal";
    std::cout << "The irc bot is created\n";
}

void Bot::handleGame(const IRCMessage& msg, const Client *client, ServerSocket &server)
{
    std::srand(std::time(0));
    const std::string choices[3] = {"rock", "paper", "scissors"};
    int randn = std::rand() % 3;
    std::string botchoice = choices[randn];
    std::string message = msg.trailing.substr(5);
    std::cout << "Bot received move: " << message << std::endl;
    std::string userMessage = "Bot played: " + botchoice + "\r\n";
    server.sendMessage(client->getFd(), ":server PRIVMSG " + client->getNickname() + " :" + userMessage);
    std::string result;

    if (message != "rock" && message != "scissors"
        && message != "paper")
    {
        server.sendMessage(client->getFd(), ":server PRIVMSG " + client->getNickname() + " :" + "Please enter the correct option to play this game.");
    }
    if (message == botchoice)
        result = "It's a tie!";
    else if ((message == "rock" && botchoice == "scissors") ||
             (message == "paper" && botchoice == "rock") ||
             (message == "scissors" && botchoice == "paper"))
        result = "You win!";
    else
        result = "Bot wins!";

    std::string resultMessage = ":server PRIVMSG " + client->getNickname() + " :" + result + "\r\n";
    server.sendMessage(client->getFd(), resultMessage);  
}

Bot::~Bot()
{
    std::cout << "The irc bot is destroyed" << std::endl;
}

