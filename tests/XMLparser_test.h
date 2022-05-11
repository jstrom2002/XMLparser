#pragma once
#include <time.h>

namespace XMLparser_test {
	struct TEST_RESULTS
	{
		bool passed = false;
		clock_t start_time=0;
		clock_t end_time=0;
		float seconds = 0.0f;
	};

	class UnitTests
	{
	public:
		TEST_RESULTS RunAllTests();

	private:

		TEST_RESULTS NodeConstructorTest();
		TEST_RESULTS ToLowerTest();
		TEST_RESULTS TrimWhitespaceTest();
	};

	class XMLparser_TESTS {
	public:
		XMLparser_TESTS() {}

		// Main entry point for all tests for XMLparser.hpp.
		void Run() { RunAllTests(); }

		UnitTests unitTests;

	private:

		// Individual tests.
		TEST_RESULTS FormatFailureTest(const char* filename_to_test);
		TEST_RESULTS ParseTest(const char* filename_to_test);
		TEST_RESULTS WriteToDiskTest(const char* filename_to_test);

		TEST_RESULTS RunAllTests();
	};
}