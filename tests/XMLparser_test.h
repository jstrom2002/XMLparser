#pragma once
#include <time.h>
#include <memory>

namespace XMLparser_test {


	class XMLparserTestEnvironment {
	public:
		struct TEST_RESULTS{
			bool passed = false;
			clock_t start_time = 0;
			clock_t end_time = 0;
			float seconds = 0.0f;
		};

	private:
		class UnitTests {
		public:
			UnitTests(XMLparserTestEnvironment& _env) :env(_env) {}
			TEST_RESULTS RunAllTests();

		private:

			TEST_RESULTS NodeConstructorTest();
			TEST_RESULTS ToLowerTest();
			TEST_RESULTS TrimWhitespaceTest();

			XMLparserTestEnvironment& env;
		};

	public:
		bool print_output = true;
		std::shared_ptr<UnitTests> unitTests=nullptr;

		XMLparserTestEnvironment(bool _print):print_output(_print){unitTests=std::make_shared<UnitTests>(*this);}

		// Main entry point for all tests for XMLparser.hpp.
		void Run() { RunAllTests(); }

	private:

		// Individual tests.
		TEST_RESULTS BOMformatFailureTest(const char* filename_to_test);
		TEST_RESULTS EntireDiskTest();
		TEST_RESULTS FormatFailureTest(const char* filename_to_test);
		TEST_RESULTS NodePairTest(const char* filename_to_test);
		TEST_RESULTS ParseTest(const char* filename_to_test);
		TEST_RESULTS UTFformattingTest(const char* filename_to_test);
		TEST_RESULTS WriteToDiskTest(const char* filename_to_test);
		TEST_RESULTS XMLredefinitionTest(const char* filename_to_test);

		TEST_RESULTS RunAllTests();
	};
}