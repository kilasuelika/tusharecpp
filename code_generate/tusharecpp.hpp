#ifndef __TUSHARECPP__
#define __TUSHARECPP__

#include "data_container.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <sstream>
#include<map>

namespace ts {
	using namespace std;

	template<typename data_store=data_container<vector> >
	class pro_api {
	private:
		map<string, string> _fields_map;
		map<string, string> _types_map;

		string host = "api.tushare.pro";
		string port = "80";
		string token;
		
		int max_retry = 5;
		int connect_error = 0;
		
		boost::asio::io_context ioc;
		boost::asio::ip::tcp::resolver resolver{ ioc };
		boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> results;
		boost::asio::ip::tcp::socket sockets{ ioc };
		boost::beast::http::request<boost::beast::http::string_body> req;
		boost::beast::http::response<boost::beast::http::dynamic_body> res;
		boost::beast::flat_buffer buffer;

		ptree base_api_req_tree;

		void connect_socket() {
			results = resolver.resolve(host, port);
			boost::asio::connect(sockets, results.begin(), results.end());
			req.method(boost::beast::http::verb::post);
			req.target("/");
			req.set(boost::beast::http::field::host, host);
			req.set(boost::beast::http::field::content_type, "application/json");
			req.keep_alive(true);
		};


		void initialize_base_api_req_tree() {
			base_api_req_tree.put("token", token);
		};

		ptree gen_api_req_tree(const string& apiname, const ptree& paramsjson) {
			ptree req_tree = base_api_req_tree;
			req_tree.put("api_name", apiname);
			req_tree.put("fields", _fields_map[apiname]);
			req_tree.put_child("params", paramsjson);

			return req_tree;
		};

		void fetch_data(const ptree& req_tree) {
			connect_error=0;
			
			ostringstream buf;
			write_json(buf, req_tree, false);
			string reqstr = buf.str();
			req.body() = reqstr;
			req.prepare_payload();

			//Get response from server.
			try {
				boost::beast::http::write(sockets, req);
				res = {};
				buffer.consume(buffer.size());
				boost::beast::http::read(sockets, buffer, res);

			}
			catch (...) {
				++connect_error;
				
				if(connect_error<=max_retry){
					connect_socket();
					fetch_data(req_tree);
				}else{
					cerr<<"Connection error."<<endl;
				};
			};

			
		};

		data_store process(const string& apiname, const ptree& parmtree) {
			ptree req_tree = gen_api_req_tree(apiname, parmtree);
			fetch_data(req_tree);
			auto data_tree=parse_data();
			auto dt=data_container(data_tree,_types_map[apiname]);
			return dt;
		};

		ptree parse_data() {
			ptree datajson;
			stringstream datastr;
			datastr << boost::beast::buffers_to_string(res.body().data());
			read_json(datastr, datajson);
			return datajson.get_child("data.items.");
		};

		void initialize_data_map() {
##_AUTO_GENERATED_FIELDS_MAP_##

##_AUTO_GENERATED_TYPES_MAP_##
		};

	public:
		pro_api(string token) :token(token) {
			initialize_data_map();
			initialize_base_api_req_tree();
			connect_socket();
		};

		//Auto generated API.
##_AUTO_GENERATED_API_##
		
	};


};

#endif