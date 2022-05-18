#include "XMLparser_test.h"
#include "XMLparser.hpp"
#include <filesystem>
#include <iostream>
#include <Windows.h>
#include <io.h>
#include <fcntl.h>
#include <cstdint>
#include <typeinfo>
#include <string>


namespace XMLparser_test
{

	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::UnitTests::NodeConstructorTest()
	{
		TEST_RESULTS result("NodeConstructorTest");
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

		if(env.print_output)
			std::cout << "\n\n\n===============\nNode constructor unit test finished in " << result.seconds << " sec\n\n" << std::endl;

		if (!result.passed) {
			throw std::exception("\n\nERROR! Node constructor test failed.");
		}

		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::UnitTests::ToLowerTest()
	{
		TEST_RESULTS result("ToLowerTest");
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

		if (env.print_output)
			std::cout << "\n\n\n===============\ntoLower() unit test finished in " << result.seconds << " sec\n\n" << std::endl;
		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::UnitTests::TrimWhitespaceTest()
	{
		TEST_RESULTS result("TrimWhiteSpaceTest");
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

		if (env.print_output)
			std::cout << "\n\n\n===============\ntrimWhitespace() unit test finished in " << result.seconds << " sec\n\n" << std::endl;
		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::UnitTests::RunAllTests() 
	{
		std::vector<XMLparserTestEnvironment::TEST_RESULTS> tests;
		
		try {
			tests.push_back(NodeConstructorTest());
			tests.push_back(ToLowerTest());
			tests.push_back(TrimWhitespaceTest());
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
		}

		TEST_RESULTS result("AllUnitTests");
		for (int i = 0; i < tests.size(); ++i) {
			result.passed &= tests[i].passed;
			result.seconds += tests[i].seconds;
		}

		if(result.passed && env.print_output)
			std::cout << "\n\n\n===============\nAll unit tests finished in " << result.seconds << " sec\n\n" << std::endl;

		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::BOMformatFailureTest(const char* filename_to_test)
	{		
		TEST_RESULTS result("BOMformatFailureTest: " + std::string(filename_to_test));

		try {

			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
			if (print_output)
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
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::EntireDiskTest()
	{
		TEST_RESULTS result("EntireDiskTest");
		result.passed = true;
		unsigned int files_parsed = 0;

		std::vector<std::string> paths_to_try=std::vector<std::string>{
			//"C:\\Windows",
			"C:\\Users",
			"C:\\Program Files"
		};

		try {
			for(int i=0;i<paths_to_try.size();++i) {
				for (auto& dirEntry : std::filesystem::recursive_directory_iterator(paths_to_try[i])) {
				
					try {
						auto p = std::filesystem::status(dirEntry).permissions();
					}
					catch (std::exception e1) {
						continue;
					}

					std::string str = dirEntry.path().string();
					if (str == "C:\\Program Files\\AMD\\CIM\\Config\\AMDUWPLauncher.xml") {
						int BREAK_HERE = 0;//DELETE THIS
						continue;
					}
					
					if (dirEntry.path().extension().string() == ".xml" && !std::filesystem::is_directory(dirEntry.path())) {
						XMLparser::XMLparser xml;
						if (print_output)
							std::cout << "parsing: " << str << std::endl;
						result.start_time = clock();
						xml.load(str.c_str());
						result.end_time = clock();
						result.seconds += float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
						result.passed &= xml.validate();
						files_parsed++;
					}
				}

				if (print_output)
					std::cout << "Total XML files parsed: " << files_parsed << std::endl;
			}
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
			result.passed = false;
		}


		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::FormatFailureTest(const char* filename_to_test)
	{
		TEST_RESULTS result("FormatFailureTest: " + std::string(filename_to_test));

		try {

			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
			if (print_output)
				std::cout << "\n\n\n===============\nIncorrectly formatted file " << filename_to_test << " parsed successfully despite bad formatting "
					<< result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.validate();
			throw new std::exception("ERROR! html formatted file should not have been successfully parsed by XMLparser lib.");
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << std::endl;
			result.passed = false;
		}

		// This test should fail out 
		result.passed = !result.passed;

		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::NodePairTest(const char* filename_to_test)
	{
		TEST_RESULTS result("NodePairTest: " + std::string(filename_to_test));

		try {
			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			if (print_output)
				xml.printToConsole();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.


			if (print_output)
				std::cout << "\n\n\n===============\nXML file with potential node pair errors " << filename_to_test << " parsed successfully in "
					<< result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.validate()&&xml.nodes.size()>3;
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
			result.passed = false;
		}

		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::ParseTest(const char* filename_to_test)
	{
		TEST_RESULTS result("ParseTest: " + std::string( filename_to_test));

		XMLparser::XMLparser xml;

		try {
			result.start_time = clock();
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
			if (print_output) {
				xml.printToConsole();
				std::cout << "\n\n\n===============\nXML file " << std::string(filename_to_test) << " parsed successfully with " <<
					(int)xml.nodes.size() << " nodes in " << result.seconds << " sec\n\n" << std::endl;
			}
			result.passed = xml.validate();
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
			result.passed = false;
		}

		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::UTFformattingTest(const char* filename_to_test)
	{
		TEST_RESULTS result("UTFformattingTest: " + std::string(filename_to_test));

		try {
			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

			if (print_output)
				std::cout << "\n\n\n===============\nXML file with UTF-16 formatting " << filename_to_test << " parsed successfully in "
					<< result.seconds << " sec\n\n" << std::endl;
			result.passed = xml.validate()&&xml.nodes.size()>3;
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
			result.passed = false;
		}

		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::WriteToDiskTest(const char* filename_to_test)
	{
		TEST_RESULTS result("WriteToDiskTest: " + std::string(filename_to_test));

		XMLparser::XMLparser xml;
		try {
			xml.load("BrainStem.dae");

			result.start_time = clock();
			xml.save(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.

			if (print_output)
				std::cout << "\n\n\n===============\nXML file saved in " << result.seconds << " sec\n\n" << std::endl;
			result.passed = std::filesystem::exists(filename_to_test);
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
			result.passed = false;
		}

		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::XMLredefinitionTest(const char* filename_to_test)
	{
		TEST_RESULTS result("XMLredefinitionTest: " + std::string(filename_to_test));

		try {
			result.start_time = clock();
			XMLparser::XMLparser xml;
			xml.load(filename_to_test);
			result.end_time = clock();
			result.seconds = float(result.end_time - result.start_time) / 1000.0f;//elapsed seconds to parse file.
			if (print_output) {
				xml.printToConsole();
				std::cout << "\n\n\n===============\nXML file with potential XML definition tag parsing error " << filename_to_test << " parsed successfully in "
					<< result.seconds << " sec\n\n" << std::endl;
			}
			result.passed = xml.validate();
		}
		catch (std::exception e1) {
			std::cerr << e1.what() << "\n*** Press ENTER to continue.****" << std::endl;
			std::cin.get();
			result.passed = false;
		}

		return result;
	}
	XMLparserTestEnvironment::TEST_RESULTS XMLparserTestEnvironment::RunAllTests(){

		TEST_RESULTS all_tests("RunAllTests");
		all_tests.passed = true;

		std::vector<TEST_RESULTS> tests;

			

		//tests.push_back(unitTests->RunAllTests());			
		//tests.push_back(FormatFailureTest("SOpage.html"));			
		//tests.push_back(ParseTest("AMDUWPLauncher.xml"));			 
		//tests.push_back(ParseTest("cube_triangulate.dae"));
		//tests.push_back(ParseTest("BrainStem.dae"));
		//tests.push_back(ParseTest("abb_irb52_7_120.dae"));
		//tests.push_back(ParseTest("kawada-hironx.dae"));
		//tests.push_back(ParseTest("SkinAndMorph.dae"));
		//tests.push_back(ParseTest("base_jpn.xml"));
		//tests.push_back(ParseTest("EQDefaultCurves.xml"));
		//tests.push_back(ParseTest("Microsoft.Build.NuGetSdkResolver.xml"));
		//tests.push_back(ParseTest("ipsar.xml"));
		//tests.push_back(ParseTest("Manifest.xml"));
		//tests.push_back(ParseTest("ProjectTemplateMRU.xml"));
		//tests.push_back(XMLredefinitionTest("GeneratedByWord.xml"));
		//tests.push_back(UTFformattingTest("SearchRedactPatterns.xml"));
		//tests.push_back(UTFformattingTest("SearchRedactPatterns_DEU.xml"));
		//tests.push_back(UTFformattingTest("AMDAUEPInstaller.xml"));			
		//tests.push_back(NodePairTest("BuildHighlights.xml"));
		//tests.push_back(NodePairTest("MonetRSA.xml"));		
		//tests.push_back(NodePairTest("System.IO.Pipelines.xml"));
		//tests.push_back(WriteToDiskTest("Test_3_XML.xml"));			
		tests.push_back(EntireDiskTest());




		std::vector<std::string> failed_tests;
		for (TEST_RESULTS& test : tests) {
			if (!test.passed)
				failed_tests.push_back(test.name);			
			all_tests.passed &= test.passed;
			all_tests.seconds += test.seconds;
		}

		if(print_output){
			if (all_tests.passed)			
				std::cout << "\n\n\n=========ALL TESTS PASSED==============\n" << std::endl;
			else {
				std::cout << "\n\n===========ERROR! Not all tests passed. Failed tests:" << std::endl;
				for (std::string nm : failed_tests)
					std::cout << nm << std::endl;				
			}

		}

		return all_tests;
	}
}