#ifndef __DATA_CONTAINER__
#define __DATA_CONTAINER__

#include<vector>
#include<string>
#include<tuple>
#include<utility>
#include<typeinfo>
#include<typeindex>
#include<boost/property_tree/ptree.hpp>
#include<boost/tokenizer.hpp>
#include<filesystem>
#include<fstream>

namespace ts {
	using namespace std;
	using namespace boost::property_tree;
	using namespace boost;

	
	template<template<typename> typename vec = vector>
	class data_container {
	private:
		vec<void*> data;
		vec<string> varnames;
		vec<string> vartypes;
		int nvars = 0;
		pair<int, int> dim;

		vec<type_index> _typeid_vec;
		

	public:

		data_container(ptree jsondata, string vartypestr, string varnamestr="") {
			
			//Parse a whole string to a vector of strings.
			tokenizer<escaped_list_separator<char>> tok(vartypestr);
			for (tokenizer<escaped_list_separator<char>>::iterator it = tok.begin();
				it != tok.end(); ++it) {
				vartypes.push_back(*it);
				cout << *it << endl;
			};
			nvars = vartypes.size();

			//Set up varnames. If no name is given, then create automatically.
			if (varnamestr == "") {
				for (int i = 0; i < nvars; ++i) {
					varnames.push_back("X"+to_string(i));
				};
			};

			//Initialize a vector of pointers.
			for (int i = 0; i < nvars; ++i) {
				if (vartypes[i] == "double") {
					data.push_back(new vec<double>);
					_typeid_vec.push_back(typeid(double));
				} else if (vartypes[i] == "string") {
					data.push_back(new vec<string>);
					_typeid_vec.push_back(typeid(string));
				} else if (vartypes[i] == "int") {
					data.push_back(new vec<int>);
					_typeid_vec.push_back(typeid(int));
				};
			};

			//Loop over each row and insert them into data.
			int row = 0;
			for (auto & rowdata: jsondata) {
				++row;
				
				int col = 0;
				for (auto it=rowdata.second.begin(); it!=rowdata.second.end(); ++it,++col) {
					//cout << "what" << endl;
					if (_typeid_vec[col] == typeid(double)) {
						reinterpret_cast<vec<double>*>(data[col])->push_back(it->second.get_value<double>());
					} else if (_typeid_vec[col] == typeid(string)) {
						reinterpret_cast<vec<string>*>(data[col])->push_back(it->second.get_value<string>());
					} else if (_typeid_vec[col] == typeid(int)) {
						reinterpret_cast<vec<int>*>(data[col])->push_back(it->second.get_value<int>());
					} else {
						cerr << "Unrecognized data type";
					};
				}
			};

			dim.first = row;
			dim.second = nvars;
		};

		void to_csv(string filename) {

			ofstream file(filename);
			for (int j = 0; j < dim.second - 1; ++j) {
				file << varnames[j]<<",";
			};
			file << varnames[dim.second - 1] << endl;

			for (int row = 0; row < dim.first; ++row) {
				for (int col = 0; col < dim.second; ++col) {
					if (_typeid_vec[col] == typeid(double)) {
						file << (*reinterpret_cast<vec<double>*>(data[col]))[row];
					} else if (_typeid_vec[col] == typeid(string)) {
						file << (*reinterpret_cast<vec<string>*>(data[col]))[row];
					} else if (_typeid_vec[col] == typeid(int)) {
						file << (*reinterpret_cast<vec<int>*>(data[col]))[row];
					};
					if (col < dim.second - 1) {
						file << ", ";
					};
				};
				file << endl;
			};
			file.close();
		};

	};
};

#endif