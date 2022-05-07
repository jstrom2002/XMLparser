#pragma once
#include <time.h>

namespace XMLparser_test
{
	class XMLparser_TESTS {
	public:
		XMLparser_TESTS() {}

		// Run all tests for XMLparser.hpp.
		void Run() {
			RunAllTests();
		}

	private:
		struct TEST_RESULTS
		{
			bool passed=false;
			clock_t start_time;
			clock_t end_time;
			float seconds=0.0f;
		};

		TEST_RESULTS ParseTest(const char* filename_to_test);
		TEST_RESULTS WriteToDiskTest(const char* filename_to_test);
		TEST_RESULTS RunAllTests();
	};
}