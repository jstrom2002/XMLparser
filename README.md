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
XMLparser xml;
if(xml.isParseable("local_file.xml"){
  xml.load("local_file.xml");
  int idx = xml.getIndexByTagFirstOrDefault("data");
  XMLnode open_node(L"<test_val id="2">");
  XMLnode close_node(L"</test_val>"));
  xml.nodes.insert(xml.nodes.begin() + idx, open_node);
  xml.nodes.insert(xml.nodes.begin() + idx + 1, close_node);
  xml.save("local_file.xml");
}
else{
  std::cout << "Cannot parse file.";
}
````

### Notes  
Currently this library will parse nearly any valid XML formatted file (including file types like .dae which use XML formatting. All you need is either an xml definintion tag). Development is ongoing to accurately parse all encodings for .xml files, but generally all UTF-8 or UTF-16 files should be parseable.  
  
  
### About  
This code has been tested with MSVC and GCC C++11, C++14, C++17, and C++20 compilers with all Wall/pedantic options (thanks to cpp.sh). Some testing performed using data from [Wikipedia's backups](http://dumps.wikimedia.your.org/backup-index.html). For questions about formatting, see the [W3 XML standard doc](https://www.w3.org/TR/xml/).  If you are having trouble parsing a particular XML file, please create an issue on this repo.
