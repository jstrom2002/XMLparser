#include "UnitTests.h"
#include "XMLparser.h"
#include <libxml/xmlwriter.h>

void UnitTests::xmlReadTest()
{
    // Open XML file, read its contents to the XMLparser object.
    XML::XMLparser xml("duck.dae", "ISO-8859-1");

	// Print all stored nodes and their details.
	for (int i = 0; i < xml.nodes.size(); ++i)
	{
        if (xml.nodes[i])
        {            
            std::cout << XML::to_string(xml.nodes[i]->name) << ": ";
            std::string str = XML::to_string(xml.nodes[i]->content);
            std::cout << str << std::endl;
        }
	}

	// Get array of nodes by name, 'text' nodes usually contain the relevant info in this file.         
	std::vector<xmlNode*> meshNodes = xml.findNodesByName(xml.nodes, "text");

	// Print all content in 'text' entries.
    for (int i = 0; i < meshNodes.size(); ++i)    
        if(meshNodes[i] && meshNodes[i]->content)
            std::cout << meshNodes[i]->content << std::endl;    

    // Clean up allocated nodes.
    for (int i = 0; i < meshNodes.size(); ++i)    
        xmlFreeNode(meshNodes[i]);    
}

void UnitTests::xmlWriteTest()
{	// Code for this test adapted from libxml2's examples: http://www.xmlsoft.org/examples/testWriter.c
    // Compare the results with the 'test_check.json' file.

    // Open file and init all necessary objects.
    XML::XMLwriter xml("test.xml", "ISO-8859-1");

    // Begin writing sections.
    xml.startElement("EXAMPLE");
    xml.writeFormatComment(xml.ConvertInput("<\xE4\xF6\xFC>"),
        "This is another comment with special characters %s");

        xml.startElement("ORDER");
        xml.writeAttribute("ORDER", "1.0");
        xml.writeAttribute("xml:lang", "de");

            xml.startElement("HEADER");
            xml.writeFormatElement("X_ORDER_ID", 53535, "%010d");
            xml.writeFormatElement("CUSTOMER_ID", 1010, "%d");
            xml.writeElement("NAME_1", xml.ConvertInput("M\xFCller"));
            xml.writeElement("NAME_2", xml.ConvertInput("J\xF6rg"));
            xml.endElement();

            xml.startElement("ENTRIES");
                xml.startElement("ENTRY");
                xml.writeElement("ARTICLE", "<Test>");
                xml.writeFormatElement("ENTRY_NO", 10, "%d");
                xml.endElement();

                xml.startElement("ENTRY");
                xml.writeElement("ARTICLE", "<Test 2>");
                xml.writeFormatElement("ENTRY_NO", 20, "%d");
                xml.endElement();
            xml.endElement();

            xml.startElement("FOOTER");
            xml.writeElement("TEXT", "This is a text.");
            xml.endElement();

    // (At the end of this block, the destructor for the 'XMLwriter' object is called and the file is written).
    // (All the elements for which no 'endElement()' function was called will be closed and written).    
}

void UnitTests::runAllTests()
{
    xmlReadTest();
    xmlWriteTest();
}