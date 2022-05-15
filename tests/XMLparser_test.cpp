#include "XMLparser_test.h"
#include "XMLparser.hpp"
#include <filesystem>
#include <iostream>
#include <Windows.h>
#include <io.h>
#include <fcntl.h>

namespace XMLparser_test
{
	TEST_RESULTS UnitTests::NodeConstructorTest(){
		TEST_RESULTS result;
		result.passed = true;
		XMLparser::XMLstring test_str = L"<?xml sdljk=\"2\" version=\'\"fake_v alue \" is \"test value\"\'?>";
		XMLparser::XMLstring test_str2 = L"<!DOCTYPE asf SYSTEM \"http://ns.adobe.com/asf/asf_1_0.dtd\">";
		
		result.start_time = clock();
		XMLparser::XMLnode test_node(test_str);
		XMLparser::XMLnode test_node2(test_str2);
		result.end_time = clock();

		result.passed &= test_node.tag == XMLparser::XMLstring(L"xmL");
		result.passed &= test_node.attributes.size() == 2;
		result.passed &= test_node.type == XMLparser::XML_TAG_TYPE::XML_DEFINITION;

		result.passed &= test_node2.tag == XMLparser::XMLstring(L"DOCTYPE");
		result.passed &= test_node2.attributes.size() == 3;
		result.passed &= test_node2.type == XMLparser::XML_TAG_TYPE::UNIQUE;

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
		XMLparser::XMLstring test_str = L" ; test. vaLuE     ";

		result.start_time = clock();

		test_str=XMLparser::toLower(test_str);
		result.passed &= test_str == L" ; test. value     ";

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
		XMLparser::XMLstring test_str = L" ; test. vaLuE     ";

		result.start_time = clock();

		XMLparser::trimWhitespace(test_str);
		result.passed &= test_str == L"; test. vaLuE";

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
	TEST_RESULTS XMLparser_TESTS::BOMformatFailureTest(const char* filename_to_test)
	{
		TEST_RESULTS result;

		try {

			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
			std::cout << "\n\n\n===============\nIncorrectly formatted file " << filename_to_test << " parsed successfully despite bad formatting "
				<< result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.validate();
			throw new std::exception("ERROR! html formatted file should not have been successfully parsed by XMLparser lib.");
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
		}

		// This test should fail out 
		result.passed = !result.passed;

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::EntireDiskTest()
	{
		TEST_RESULTS result;
		result.passed = true;

		try {
			for (const auto& dirEntry : std::filesystem::recursive_directory_iterator("C:\\Program Files")) {
				
				try {
					auto p = std::filesystem::status(dirEntry).permissions();
				}
				catch (std::exception e1) {
					continue;
				}

				std::string str = dirEntry.path().string();
				if (str.rfind(".xml") != XMLparser::XMLstring::npos) {


					if (str == "C:\\Program Files\\AMD\\CIM\\Config\\AMDUWPLauncher.xml") {
						int BREAK_HERE = 0;//DELETE THIS
						continue;
					}

					XMLparser::XMLparser xml;
					if (!xml.isParseable(str.c_str())) {
						std::cout << "ERROR! File " << str << " is not parseable due to bad formatting." << std::endl;
						continue;
					}

					std::cout << "parsing: " << str << std::endl;
					result.start_time = clock();
					xml.load(str.c_str());
					result.end_time = clock();
					result.seconds += float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
					result.passed &= xml.validate();
				}
			}
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
		}

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::FormatFailureTest(const char* filename_to_test)
	{
		TEST_RESULTS result;

		try {

			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
			std::cout << "\n\n\n===============\nIncorrectly formatted file " << filename_to_test << " parsed successfully despite bad formatting " 
				<< result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.validate();
			throw new std::exception("ERROR! html formatted file should not have been successfully parsed by XMLparser lib.");
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
		}

		// This test should fail out 
		result.passed = !result.passed;

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::NodePairTest(const char* filename_to_test)
	{
		TEST_RESULTS result;

		try {
			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			xml.printToConsole();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.


			std::cout << "\n\n\n===============\nXML file with potential node pair errors " << filename_to_test << " parsed successfully in " 
				<< result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.validate()&&xml.nodes.size()>3;
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
		}

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::ParseTest(const char* filename_to_test)
	{
		TEST_RESULTS result;

		try {
			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
			xml.printToConsole();
			std::cout << "\n\n\n===============\nXML file " << filename_to_test << " parsed successfully in " 
				<< result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.validate()&&xml.nodes.size()>3;
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
		}

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::UTFformattingTest(const char* filename_to_test)
	{
		TEST_RESULTS result;

		try {
			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

			std::cout << "\n\n\n===============\nXML file with UTF-16 formatting " << filename_to_test << " parsed successfully in " 
				<< result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.validate()&&xml.nodes.size()>3;
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
		}

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::WriteToDiskTest(const char* filename_to_test)
	{
		TEST_RESULTS result;
		XMLparser::XMLparser xml;
		try {
			xml.load("BrainStem.dae");

			result.start_time = clock();
			xml.save(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

			std::cout << "\n\n\n===============\nXML file saved in " << result.seconds << " sec\n\n" << std::endl;
			result.passed = std::filesystem::exists(filename_to_test);
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
		}

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::XMLredefinitionTest(const char* filename_to_test)
	{
		TEST_RESULTS result;

		try {
			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
			xml.printToConsole();
			std::cout << "\n\n\n===============\nXML file with potential XML definition tag parsing error " << filename_to_test << " parsed successfully in "
				<< result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.validate();
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
		}

		return result;
	}
	TEST_RESULTS XMLparser_TESTS::RunAllTests(){

		TEST_RESULTS all_tests;

		std::vector<TEST_RESULTS> tests;

			

		tests.push_back(EntireDiskTest());
			
		//tests.push_back(BOMformatFailureTest("AMDUWPLauncher.xml"));
			 
		//tests.push_back(FormatFailureTest("SOpage.html"));
			
		//tests.push_back(ParseTest("cube_triangulate.dae"));
		//tests.push_back(ParseTest("BrainStem.dae"));
		//tests.push_back(ParseTest("abb_irb52_7_120.dae"));
		//tests.push_back(ParseTest("kawada-hironx.dae"));
		//tests.push_back(ParseTest("SkinAndMorph.dae"));

		//tests.push_back(XMLredefinitionTest("GeneratedByWord.xml"));

		//tests.push_back(UTFformattingTest("SearchRedactPatterns.xml"));
		//tests.push_back(UTFformattingTest("SearchRedactPatterns_DEU.xml"));
		//tests.push_back(UTFformattingTest("AMDAUEPInstaller.xml"));
			
		//tests.push_back(NodePairTest("BuildHighlights.xml"));
		//tests.push_back(NodePairTest("MonetRSA.xml"));
		//
		//tests.push_back(WriteToDiskTest("Test_3_XML.xml"));
			
		//tests.push_back(unitTests.RunAllTests());



		for (TEST_RESULTS& test : tests) {
			all_tests.passed &= test.passed;
			all_tests.seconds += test.seconds;
		}

		if (all_tests.passed)
			std::cout << "\n\n\n=========ALL TESTS PASSED\n==============\n" << std::endl;
		else
			std::cout << "\n\n===========ERROR! Not all tests passed. Step through to find failed tests." << std::endl;

		return all_tests;
	}
}