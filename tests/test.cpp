#include "XMLparser_test.h"
#include <iostream>
#include <exception>
int main(int argc, char* argv[]){
	bool print_output=false;
	if (argc>=2){
		if (strcmp(argv[1], "-p") == 0)// '-p' option allows printing output
			print_output=true;		
	}
	else {
		std::cout << "valid options for XMLparser.hpp test environment are '-p'" << std::endl;
	}
	if (print_output) {
		std::cout << "running tests for XMLparser.hpp..." << std::endl;
	}
	XMLparser_test::XMLparserTestEnvironment tests(print_output);
	tests.Run();
}