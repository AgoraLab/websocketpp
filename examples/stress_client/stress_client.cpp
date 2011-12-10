/*
 * Copyright (c) 2011, Peter Thorson. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the WebSocket++ Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 */

#include "../../src/endpoint.hpp"
#include "../../src/roles/client.hpp"

#include <boost/thread.hpp>

#include <iostream>

// PLATFORM SPECIFIC STUFF
#include <unistd.h>
#include <sys/resource.h>

typedef websocketpp::endpoint<websocketpp::role::client,websocketpp::socket::plain> plain_endpoint_type;
typedef plain_endpoint_type::handler_ptr plain_handler_ptr;
typedef plain_endpoint_type::connection_ptr connection_ptr;

class echo_client_handler : public plain_endpoint_type::handler {
public:
	typedef echo_client_handler type;
	typedef plain_endpoint_type::connection_ptr connection_ptr;
	
	void on_message(connection_ptr connection,websocketpp::message::data_ptr msg) {		
		/*if (connection->get_resource() == "/getCaseCount") {
			std::cout << "Detected " << msg->get_payload() << " test cases." << std::endl;
			m_case_count = atoi(msg->get_payload().c_str());
		} else {
			connection->send(msg->get_payload(),(msg->get_opcode() == websocketpp::frame::opcode::BINARY));
		}*/
		
		connection->recycle(msg);
	}
	
	void http(connection_ptr connection) {
		//connection->set_body("HTTP Response!!");
	}
	
	void on_fail(connection_ptr connection) {
		std::cout << "connection failed" << std::endl;
	}
	
	int m_case_count;
};


int main(int argc, char* argv[]) {
	std::string uri;
	
	/*if (argc != 2) {
		std::cout << "Usage: `echo_client test_url`" << std::endl;
	} else {
		uri = argv[1];
	}*/
	
	// 12288 is max OS X limit without changing kernal settings
    const rlim_t ideal_size = 100000;
    rlim_t old_size;
    
    struct rlimit rl;
    int result;
    
    result = getrlimit(RLIMIT_NOFILE, &rl);
    if (result == 0) {
        std::cout << "cur: " << rl.rlim_cur << " max: " << rl.rlim_max << std::endl;
        
        old_size = rl.rlim_cur;
        
        if (rl.rlim_cur < ideal_size) {
            rl.rlim_cur = ideal_size;
            //rl.rlim_cur = rl.rlim_max;
            result = setrlimit(RLIMIT_NOFILE, &rl);
            
            if (result != 0) {
                std::cout << "Unable to request an increase in the file descripter limit. This server will be limited to " << old_size << " concurrent connections. Error code: " << errno << std::endl;
            }
        }
    }

	
	int num_connections = 500;
	
	if (argc == 2) {
		// TODO: input validation?
		num_connections = atoi(argv[1]);
	}
	
	try {
		plain_handler_ptr handler(new echo_client_handler());
		plain_endpoint_type endpoint(handler);
		
		endpoint.alog().unset_level(websocketpp::log::alevel::ALL);
		endpoint.elog().set_level(websocketpp::log::elevel::ALL);
		
		std::set<connection_ptr> connections;
		
		
		connections.insert(endpoint.connect("ws://localhost:9002/"));
		
		boost::thread t(boost::bind(&plain_endpoint_type::run, &endpoint));
		
		std::cout << "launching connections" << std::endl;
		
		for (int i = 0; i < num_connections; i++) {
			if (i % 200 == 0) {
				sleep(1);
			}
			connections.insert(endpoint.connect("ws://localhost:9002/"));
		}
		
		std::cout << "complete" << std::endl;
		
		//endpoint.run();
		
		/*char line[512];
		while (std::cin.getline(line, 512)) {
			std::iterator
			
			c->send(line);
		}*/
		
		t.join();
		
		
		std::cout << "done" << std::endl;
		
	} catch (std::exception& e) {
		std::cerr << "Exception: " << e.what() << std::endl;
	}
	
	return 0;
}
