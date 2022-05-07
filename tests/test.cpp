#include "XMLparser.hpp"
#include <iostream>
int main()
{
	XMLparser::XMLreader xml("cube_triangulate.dae", true);
	for (auto nd : xml.nodes) {
		std::cout << nd.ToString();
	}
	return 0;
}