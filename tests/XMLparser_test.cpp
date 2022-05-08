#include "XMLparser_test.h"
#include "XMLparser.hpp"
#include <filesystem>
#include <iostream>

namespace XMLparser_test
{
	TEST_RESULTS UnitTests::NodeConstructorTest(){
		TEST_RESULTS result;
		result.passed = true;
		std::string test_str = "< ? xml sdljk version=\"fake_v alue \" ? >";

		result.start_time = clock();

		XMLparser::XMLnode test_node(test_str);
		result.passed &= 
			test_node.tag == "xml" &&
			test_node.attributes.size() == 2 && 
			test_node.type == XMLparser::TAG_TYPE::XML_DEFINITION
		;

		result.end_time = clock();


		result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

		std::cout << "\n\n\n===============\nNode constructor unit test finished in " << result.seconds << " sec\n\n" << std::endl;
		return result;
	}
	TEST_RESULTS UnitTests::TrimWhitespaceTest(){
		TEST_RESULTS result;
		result.passed = true;
		std::string test_str = " ; test. vaLuE     ";

		result.start_time = clock();

		XMLparser::trimWhitespace(test_str);
		result.passed &= test_str == "; test. vaLuE";

		result.end_time = clock();


		result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

		std::cout << "\n\n\n===============\ntrimWhitespace() unit test finished in " << result.seconds << " sec\n\n" << std::endl;
		return result;
	}
	TEST_RESULTS UnitTests::RunAllTests() {
		const int num_tests = 2;
		TEST_RESULTS tests[num_tests] = {
			NodeConstructorTest(),
			TrimWhitespaceTest()
		};
		TEST_RESULTS all_tests;
		for (int i = 0; i < num_tests; ++i) {
			all_tests.passed &= tests[i].passed;
			all_tests.seconds += tests[i].seconds;
		}
		return all_tests;
	}
	TEST_RESULTS XMLparser_TESTS::ParseTest(const char* filename_to_test)
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
		result.passed = xml.check_validation();
		return result;
	}
	TEST_RESULTS XMLparser_TESTS::WriteToDiskTest(const char* filename_to_test)
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
		result.passed = std::filesystem::exists(filename_to_test);
		return result;
	}
	TEST_RESULTS XMLparser_TESTS::RunAllTests(){
		std::vector<TEST_RESULTS> tests = {
			ParseTest("cube_triangulate.dae"),
			ParseTest("BrainStem.dae"),
			WriteToDiskTest("Test_3_XML.xml"),
			unitTests.RunAllTests()
		};
		TEST_RESULTS all_tests;
		for (auto test : tests) {
			all_tests.passed &= test.passed;
			all_tests.seconds += test.seconds;
		}
		return all_tests;
	}
}