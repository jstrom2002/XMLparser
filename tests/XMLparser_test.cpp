#include "XMLparser_test.h"
#include "XMLparser.hpp"
#include <iostream>

namespace XMLparser_test
{
	XMLparser_TESTS::TEST_RESULTS XMLparser_TESTS::ParseTest(const char* filename_to_test)
	{
		TEST_RESULTS result;
		result.start_time = clock();
		XMLparser::XMLreader xml(filename_to_test, true);
		result.end_time = clock();
		for (auto nd : xml.nodes) {
			std::cout << nd.ToString();
		}
		result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

		std::cout << "\n\n\n===============XML file parsed in " << result.seconds << " sec\n\n" << std::endl;

		return result;
	}
	XMLparser_TESTS::TEST_RESULTS XMLparser_TESTS::RunAllTests()
	{
		TEST_RESULTS test1 = ParseTest("cube_triangulate.dae");
		TEST_RESULTS test2 = ParseTest("BrainStem.dae");

		TEST_RESULTS all_tests;

		all_tests.passed |= test1.passed;
		all_tests.passed |= test2.passed;

		all_tests.seconds += test1.seconds;
		all_tests.seconds += test2.seconds;

		return all_tests;
	}
}