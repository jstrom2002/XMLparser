#include "XMLparser_test.h"
#include "XMLparser.hpp"
#include <filesystem>
#include <iostream>

namespace XMLparser_test
{
	TEST_RESULTS UnitTests::NodeConstructorTest(){
		TEST_RESULTS result;
		result.passed = true;
		std::string test_str = "<?xml sdljk=\"2\" version=\'\"fake_v alue \" is \"test value\"\'?>";
		std::string node_name = std::string("xml");
		
		result.start_time = clock();
		XMLparser::XMLnode test_node(test_str);
		result.end_time = clock();

		result.passed &= test_node.tag == node_name;
		result.passed &= test_node.attributes.size() == 2;
		result.passed &= test_node.type == XMLparser::TAG_TYPE::XML_DEFINITION;

		result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

		std::cout << "\n\n\n===============\nNode constructor unit test finished in " << result.seconds << " sec\n\n" << std::endl;

		if (!result.passed) {
			throw std::exception("\n\nERROR! Node constructor test failed.");
		}

		return result;
	}
	TEST_RESULTS UnitTests::ToLowerTest(){
		TEST_RESULTS result;
		result.passed = true;
		std::string test_str = " ; test. vaLuE     ";

		result.start_time = clock();

		test_str=XMLparser::toLower(test_str);
		result.passed &= test_str == " ; test. value     ";

		result.end_time = clock();


		result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

		if (!result.passed) {
			throw std::exception("\n\nERROR! toLower() test failed.");
		}

		std::cout << "\n\n\n===============\ntoLower() unit test finished in " << result.seconds << " sec\n\n" << std::endl;
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

		if (!result.passed) {
			throw std::exception("\n\nERROR! trimWhitespace() test failed.");
		}

		std::cout << "\n\n\n===============\ntrimWhitespace() unit test finished in " << result.seconds << " sec\n\n" << std::endl;
		return result;
	}
	TEST_RESULTS UnitTests::RunAllTests() {
		std::vector<TEST_RESULTS> tests;
		
		try {
			tests.push_back(NodeConstructorTest());
			tests.push_back(ToLowerTest());
			tests.push_back(TrimWhitespaceTest());
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
		}

		TEST_RESULTS all_tests;
		for (int i = 0; i < tests.size(); ++i) {
			all_tests.passed &= tests[i].passed;
			all_tests.seconds += tests[i].seconds;
		}

		if(all_tests.passed)
			std::cout << "\n\n\n===============\nAll unit tests finished in " << all_tests.seconds << " sec\n\n" << std::endl;


		return all_tests;
	}
	TEST_RESULTS XMLparser_TESTS::FormatFailureTest(const char* filename_to_test)
	{
		TEST_RESULTS result;
		XMLparser::XMLparser xml;

		try {

			result.start_time = clock();
			xml.load(filename_to_test, true);
			result.end_time = clock();

			for (auto nd : xml.nodes) {
				std::cout << nd->ToString();
			}
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

			std::cout << "\n\n\n===============\nXML file " << filename_to_test << " parsed in " << result.seconds << " sec\n\n" << std::endl;
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
		}

		// This test should fail out 
		result.passed = !result.passed;

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::ParseTest(const char* filename_to_test)
	{
		TEST_RESULTS result;
		XMLparser::XMLparser xml;

		try {

			result.start_time = clock();
			xml.load(filename_to_test, true);
			result.end_time = clock();

			for (auto nd : xml.nodes) {
				std::cout << nd->ToString();
			}
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

			std::cout << "\n\n\n===============\nXML file " << filename_to_test << " parsed in " << result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.check_validation()&&xml.nodes.size()>3;
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
			std::cin.get();
		}

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::WriteToDiskTest(const char* filename_to_test)
	{
		TEST_RESULTS result;
		XMLparser::XMLparser xml;
		try {
			xml.load("BrainStem.dae", true);

			result.start_time = clock();
			xml.save(filename_to_test);
			result.end_time = clock();
			for (auto nd : xml.nodes) {
				std::cout << nd->ToString();
			}
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

			std::cout << "\n\n\n===============\nXML file saved in " << result.seconds << " sec\n\n" << std::endl;
			result.passed = std::filesystem::exists(filename_to_test);
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
			std::cin.get();
		}

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::RunAllTests(){

		TEST_RESULTS all_tests;

			std::vector<TEST_RESULTS> tests; 
			
			tests.push_back(FormatFailureTest("SOpage.html"));
			tests.push_back(ParseTest("cube_triangulate.dae"));
			tests.push_back(ParseTest("BrainStem.dae"));
			tests.push_back(ParseTest("abb_irb52_7_120.dae"));
			tests.push_back(ParseTest("kawada-hironx.dae"));
			tests.push_back(ParseTest("GeneratedByWord.xml"));
			tests.push_back(ParseTest("SkinAndMorph.dae"));
			tests.push_back(WriteToDiskTest("Test_3_XML.xml"));
			tests.push_back(unitTests.RunAllTests());

			for (TEST_RESULTS& test : tests) {
				all_tests.passed &= test.passed;
				all_tests.seconds += test.seconds;
			}

			if (all_tests.passed)
				std::cout << "\n\n\n=========ALL TESTS PASSED\n==============\n" << std::endl;

		return all_tests;
	}
}