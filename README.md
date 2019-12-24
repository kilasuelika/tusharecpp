# tusharecpp
 tushare C++ API
 
 基本使用：
 ```
#include <iostream >
#include "../../../ tusharecpp/tusharecpp.hpp"
using namespace std;
int main()
{
	auto pro = ts:: pro_api("xxxxxxxxxxxxxxxxxxxxxxx");
	auto data=pro.stock_basic("","","");
	cout << "Begin write;" << endl;
	data.to_csv("data.csv");
	std::cout << "Hello World !\n";
}
 ```