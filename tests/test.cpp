#include "XMLparser_test.h"
#include <iostream>
#include <exception>
int main(){
	XMLparser_test::XMLparser_TESTS tests;
	try {
		tests.Run();
	}
	catch (std::exception e1) {
		std::cout << e1.what() << std::endl;
	}
	return 0;
}