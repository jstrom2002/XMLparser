# XMLparser

### _C++11 Compliant Single-header C++ XML Parser._  

[![GitHub license](https://img.shields.io/badge/license-MIT-blue.svg)](https://raw.githubusercontent.com/jstrom2002/XMLparser/main/LICENSE)
[![GitHub Releases](https://img.shields.io/github/release/jstrom2002/XMLparser.svg)](https://github.com/jstrom2002/json/releases)
[![GitHub Downloads](https://img.shields.io/github/downloads/jstrom2002/XMLparser/total)](https://github.com/jstrom2002/json/releases)
[![GitHub Issues](https://img.shields.io/github/issues/jstrom2002/XMLparser.svg)](https://github.com/jstrom2002/XMLparser/issues)
[![Average time to resolve an issue](https://isitmaintained.com/badge/resolution/jstrom2002/XMLparser.svg)](https://isitmaintained.com/project/jstrom2002/XMLparser "Average time to resolve an issue")
  
<br>

### Usage Examples  
load/save a file:
````
XMLparser xml; // Create instance of the parser object to hold nodes, etc.
if(xml.isParseable("local_file.xml") { // Check the formatting of the file to see if it has 
  xml.load("local_file.xml"); // Load and parse a file into a ilnear list of nodes with related child/parent pointers to preserve XML tree structure.
  int idx = xml.getIndexByTagFirstOrDefault("data"); // Get the index of a particular node by type.
  XMLnode open_node(L"<test_val id="2">"); // Create an scoped 'open' node with tag 'test_val'.
  XMLnode close_node(L"</test_val>")); // Create a closing node to match the open 'test_val' node to close this scope.
  xml.nodes.insert(xml.nodes.begin() + idx, open_node); // Insert both nodes into the linear node list.
  xml.nodes.insert(xml.nodes.begin() + idx + 1, close_node);
  xml.save("local_file.xml"); // Write file to disk, overwriting the local_file.xml file's contents by default.
}
else{
  std::cout << "Cannot parse file.";
}
````

### Supported  
XML versions: 1.0, 1.1  
Encodings: UTF-8, UTF-16  


### Notes  
Currently this library will parse nearly any valid XML formatted file (including file types like .dae which use XML formatting). This parser requires either a node with an 'xml' tag with the 'encoding' attribute or a BOM with the appropriate encoding bits to parse correctly, else it is assumed all XML files are UTF-8 encoded. Development is ongoing to accurately parse all encodings for .xml files, but generally all UTF-8 or UTF-16 files should be parseable. There are no external dependencies other than C/C++ STL or standard library headers.  
  
  
### About  
This code has been tested on a Windows 10 machine with MSVC and GCC C++11, C++14, C++17, and C++20 compilers with all Wall/pedantic options (thanks to cpp.sh). Some testing performed using data from [Wikipedia's backups](http://dumps.wikimedia.your.org/backup-index.html). For questions about formatting, see the [W3 XML standard doc](https://www.w3.org/TR/xml/).  If you are having trouble parsing a particular XML file, please create an issue on this repo. Thank you for your patience.  
