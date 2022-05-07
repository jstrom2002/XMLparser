#include "XMLparser_test.h"
#include "XMLparser.hpp"
#include <iostream>

namespace XMLparser_test
{
	XMLparser_TESTS::TEST_RESULTS XMLparser_TESTS::ParseTest(const char* filename_to_test)
	{
		TEST_RESULTS result;
		XMLparser::XMLparser xml;

		result.start_time = clock();
		xml.load(filename_to_test, true);
		result.end_time = clock();

		for (auto nd : xml.nodes) {
			std::cout << nd.ToString();
		}
		result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

		std::cout << "\n\n\n===============\nXML file parsed in " << result.seconds << " sec\n\n" << std::endl;

		return result;
	}
	XMLparser_TESTS::TEST_RESULTS XMLparser_TESTS::WriteToDiskTest(const char* filename_to_test)
	{
		TEST_RESULTS result;
		XMLparser::XMLparser xml;
		xml.load("BrainStem.dae", true);

		result.start_time = clock();
		xml.save(filename_to_test, true);
		result.end_time = clock();
		for (auto nd : xml.nodes) {
			std::cout << nd.ToString();
		}
		result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

		std::cout << "\n\n\n===============\nXML file saved in " << result.seconds << " sec\n\n" << std::endl;

		return result;
	}
	XMLparser_TESTS::TEST_RESULTS XMLparser_TESTS::RunAllTests()
	{
		const int num_tests = 3;
		TEST_RESULTS tests[num_tests] = {
			ParseTest("cube_triangulate.dae"),
			ParseTest("BrainStem.dae"),
			WriteToDiskTest("Test_3_XML.xml")
		};
		TEST_RESULTS all_tests;
		for (int i = 0; i < num_tests; ++i) {
			all_tests.passed &= tests[i].passed;
			all_tests.seconds += tests[i].seconds;
		}
		return all_tests;
	}
}