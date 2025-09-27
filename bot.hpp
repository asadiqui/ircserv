/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   bot.hpp                                            :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mnachit <mnachit@student.42.fr>            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/08/15 16:12:26 by mnachit           #+#    #+#             */
/*   Updated: 2025/09/26 10:44:30 by mnachit          ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef BOT_HPP
#define BOT_HPP


#include <string>
#include <iostream>
#include "ServerSocket.hpp"


class ServerSocket;

class Bot {

    private:
        std::string nickname;
        std::string username;
        std::string realname;
        ServerSocket* server;
    public :
        Bot(ServerSocket *server);
        void handleGame(const IRCMessage& msg, const Client* client, ServerSocket& server);
        std::string getNickname() const { return nickname; }
        ~Bot();
    };

#endif