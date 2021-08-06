#include "XMLparser.hpp"
#include <iostream>

// TEST CODE, do not include in lib.
int main()
{
	// Parse XML file.
	XMLparser::XMLparser xml("test.dae");

	// Test -- display all entries and their properties.
	for (auto& k : *xml.entries())
	{ 
		std::cout << "key: " << k.first << std::endl; 		
		std::cout << "content: " << k.second->getElement() << std::endl; 		
		std::cout << "attributes: ";
		for(auto& l : k.second->attributes)
			std::cout << l.first << "=" << l.second << " "; 
		std::cout << std::endl;
		std::cout << "children: ";
		for(auto& c : k.second->children)
			std::cout << c->key << " "; 
		std::cout << "\n============================\n" << std::endl;
	}	

	return 0;
}