/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                    _______    _                                     *
 *                   (  ____ \  ( \     |\     /|                      * 
 *                   | (    \/  | (     ( \   / )                      *
 *                   | (__      | |      \ (_) /                       *
 *                   |  __)     | |       \   /                        *
 *                   | (        | |        ) (                         *
 *                   | )        | (____/\  | |                         *
 *                   |/         (_______/  \_/                         *
 *                                                                     *
 *                                                                     *
 *     fly is an awesome c++11 network library.                        *
 *                                                                     *
 *   @author: lichuan                                                  *
 *   @qq: 308831759                                                    *
 *   @email: 308831759@qq.com                                          *
 *   @github: https://github.com/lichuan/fly                           *
 *   @date: 2015-06-10 13:34:21                                        *
 *                                                                     *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <unistd.h>
#include <unordered_map>
#include <iostream>
#include "init.hpp"
#include "net/server.hpp"
#include "base/logger.hpp"

using namespace std::placeholders;
using fly::net::Wsock;

class Test_Server : public fly::base::Singleton<Test_Server>
{
public:
    bool init(std::shared_ptr<fly::net::Connection<Wsock>> connection)
    {
        std::lock_guard<std::mutex> guard(m_mutex);
        m_connections[connection->id()] = connection;
        CONSOLE_LOG_INFO("connection count: %u", m_connections.size());
    }
    
    void dispatch(std::unique_ptr<fly::net::Message<Wsock>> message)
    {
        std::shared_ptr<fly::net::Connection<Wsock>> connection = message->get_connection();
        const fly::net::Addr &addr = connection->peer_addr();
        CONSOLE_LOG_INFO("recv message from %s:%d raw_data: %s", addr.m_host.c_str(), addr.m_port, message->raw_data().c_str());
        std::string data = "";

        for(auto i = 0; i < 100; ++i)
        {
            data += "1234567890";
        }

        connection->send(data.c_str(), data.length());
    }
    
    void close(std::shared_ptr<fly::net::Connection<Wsock>> connection)
    {
        CONSOLE_LOG_INFO("close connection from %s:%d", connection->peer_addr().m_host.c_str(), connection->peer_addr().m_port);
        std::lock_guard<std::mutex> guard(m_mutex);
        m_connections.erase(connection->id());
        CONSOLE_LOG_INFO("connection count: %u", m_connections.size());
    }
    
    void be_closed(std::shared_ptr<fly::net::Connection<Wsock>> connection)
    {
        CONSOLE_LOG_INFO("connection from %s:%d be closed", connection->peer_addr().m_host.c_str(), connection->peer_addr().m_port);
        std::lock_guard<std::mutex> guard(m_mutex);
        m_connections.erase(connection->id());
        CONSOLE_LOG_INFO("connection count: %u", m_connections.size());
    }
    
    void main()
    {
        //init library
        fly::init();
        
        //init logger
        fly::base::Logger::instance()->init(fly::base::DEBUG, "server", "./log/");
        
        //test tcp server
        std::unique_ptr<fly::net::Server<Wsock>> server(new fly::net::Server<Wsock>(fly::net::Addr("0.0.0.0", 8088),
                                                                      std::bind(&Test_Server::init, this, _1),
                                                                      std::bind(&Test_Server::dispatch, this, _1),
                                                                      std::bind(&Test_Server::close, this, _1),
                                                                      std::bind(&Test_Server::be_closed, this, _1), 1));
        
        // std::thread thd([&]()
        // {
        //     std::string cmd;
        //     std::cin >> cmd;
        //     if(cmd == "stop")
        //     {
        //         server->stop();
        //         std::cout << "stop finished." << std::endl;
        //     }
        // });
        
        if(server->start())
        {
            CONSOLE_LOG_INFO("start server ok!");
            server->wait();
            //thd.join();
            CONSOLE_LOG_INFO("stop server ok!");
        }
        else
        {
            CONSOLE_LOG_FATAL("start server failed");
        }
    }
    
private:
    std::unordered_map<uint64, std::shared_ptr<fly::net::Connection<Wsock>>> m_connections;
    std::mutex m_mutex;
};

int main()
{
    Test_Server::instance()->main();
}
