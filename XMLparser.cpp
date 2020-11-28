#include "XMLparser.h"
#include <libxml/parser.h>
#include <libxml/encoding.h>
#include <libxml/xmlwriter.h>
#include <sstream>
#include <algorithm>

namespace XML
{
    XMLstring::XMLstring() : xstr(nullptr)
    {
    }

    XMLstring::XMLstring(char* str_)
    {
        int len = strlen(str_);
        xstr = new xmlChar[len+1];
        for (int i = 0; i < strlen(str_); ++i)
            xstr[i] = str_[i];
        xstr[len] = '\0';
    }

    XMLstring::XMLstring(const char* str_)
    {
        int len = strlen(str_);
        xstr = new xmlChar[len+1];
        for (int i = 0; i < strlen(str_); ++i)
            xstr[i] = str_[i];
        xstr[len] = '\0';
    }

    XMLstring::XMLstring(std::string str_)
    {
        xstr = new xmlChar[str_.length()+1];
        for (int i = 0; i < str_.length(); ++i)
            xstr[i] = str_[i];
        xstr[str_.length()] = '\0';
    }

    XMLstring::~XMLstring()
    {
        if (xstr && xstr[0] != '\0')
            xmlFree(xstr);
        xstr = nullptr;
    }

    xmlChar* XMLstring::xml_str()
    {
        return xstr;
    }

    std::string XMLstring::str()
    {
        return std::string((char*)xstr);
    }

    std::string XMLbase::convertHexValsToString(std::string str)
    {
        std::string res;
        res.reserve(str.size() / 2);
        for (int i = 0; i < str.size(); i += 2)
        {
            std::istringstream iss(str.substr(i, 2));
            int temp;
            iss >> std::hex >> temp;
            res += static_cast<char>(temp);
        }
        return res;
    }

    XMLstring XMLbase::ConvertInput(std::string in)
    {
        XMLstring str;
        if (encoding == "ISO-8859-1")
        {
            str = convertHexValsToString(in);            
        }

        return str;
    }

    XMLparser::~XMLparser()
    {
        // Free the loaded doc pointer.
        xmlFreeDoc(doc);

        // Free all stored nodes.
        for (int i = 0; i < nodes.size(); ++i)
        {
            if (nodes[i])
                xmlFreeNode(nodes[i]);
        }

        xmlCleanupParser();
        xmlMemoryDump();
    }

    XMLparser::XMLparser(std::string filepath_, std::string encoding_) : root_element(nullptr)
    {
        LIBXML_TEST_VERSION//Initialize libxml2 and check version

        filepath = filepath_;
        encoding = encoding_;

        // Parse the file and get the DOM.
        doc = xmlReadFile(filepath.c_str(), encoding.c_str(), 0);

        // Get the root element node.
        root_element = xmlDocGetRootElement(doc);
        if (!root_element)
            std::cout << "ERROR! Could not load root element of file." << std::endl;

        // Fill the array of nodes from the document starting from the root node.
        getSubtree(root_element, nodes);
    }

    XMLwriter::XMLwriter(std::string filepath_, std::string encoding_)
    {
        filepath = filepath_;
        encoding = encoding_;

        /* Create a new XmlWriter for DOM, with no compression. */
        writer = xmlNewTextWriterDoc(&doc, 0);
        if (writer == NULL)
        {
            printf("testXmlwriterDoc: Error creating the xml writer\n");
            return;
        }

        /* Start the document with the xml default for the version,
        * encoding ISO 8859-1 and the default for the standalone
        * declaration. */
        if (xmlTextWriterStartDocument(writer, NULL, encoding.c_str(), NULL) < 0) {
            printf("testXmlwriterDoc: Error at xmlTextWriterStartDocument\n");
            return;
        }
    }

    XMLwriter::~XMLwriter()
    {
        // Close document pointer, get ready to write file.
        if (xmlTextWriterEndDocument(writer) < 0) {
            printf("XMLwriter: Error at xmlTextWriterEndDocument\n");
            return;
        }

        // Cleanup and save to file.
        xmlFreeTextWriter(writer);
        xmlSaveFileEnc(filepath.c_str(), doc, encoding.c_str());
        xmlFreeDoc(doc);
    }

    xmlNode* XMLparser::findNodeByName(std::vector<xmlNode*> nodes, std::string nodeName)
    {
        for (int i = 0; i < nodes.size(); ++i)
        {
            if (std::string((char*)(nodes[i]->name)) == nodeName)
            {
                return nodes[i];
            }
        }
        return nullptr;
    }

    int XMLparser::findIndexByName(std::vector<xmlNode*> nodes, std::string nodeName)
    {
        for (int i = 0; i < nodes.size(); ++i)
        {
            if (std::string((char*)(nodes[i]->name)) == nodeName)
            {
                return i;
            }
        }
        return -1;
    }

    std::vector<xmlNode*> XMLparser::findNodesByName(xmlNode* a_node, std::string nodeName)
    {
        std::vector<xmlNode*> foundEntries;
        xmlNode* starting_node = a_node;
        xmlNode* cur_node = NULL;
        for (cur_node = starting_node; cur_node; cur_node = cur_node->children)
        {
            if (std::string((char*)(cur_node->name)) == nodeName)
            {
                foundEntries.push_back(xmlCopyNode(cur_node, false));
            }
        }
        return foundEntries;
    }

    std::vector<xmlNode*> XMLparser::findNodesByName(std::string nodeName)
    {
        return findNodesByName(this->nodes, nodeName);
    }

    std::vector<xmlNode*> XMLparser::findNodesByName(std::vector<xmlNode*> nodes, std::string nodeName)
    {
        std::vector<xmlNode*> foundEntries;
        for (int i = 0; i < nodes.size(); ++i) {
            if (std::string((char*)(nodes[i]->name)) == nodeName) 
            {
                foundEntries.push_back(xmlCopyNode(nodes[i],false));
            }
        }
        return foundEntries;
    }

    void XMLparser::getSubtree(xmlNode* root_node, std::vector<xmlNode*>& outputNodes)
    {
        for (xmlNode* cur_node = root_node; cur_node; cur_node = cur_node->next) 
        {
            if (cur_node)
            {                
                xmlNode* node_ = xmlCopyNode(cur_node,false);
                outputNodes.push_back(node_);
            }
            if(cur_node && cur_node->children)
                getSubtree(cur_node->children, outputNodes);
        }
    }

    std::vector<xmlNode*> XMLparser::getBranch(xmlNode* root_node)
    {
        std::vector<xmlNode*> entries = { root_node };
        if (!root_node) 
            return entries;

        xmlNode* cur_node = root_node;

        // Iterate through subtree nodes, pushing them onto the stack.
        while (cur_node && cur_node->children) {
            cur_node = cur_node->children;
            while (cur_node && cur_node->type != XML_ELEMENT_NODE) { cur_node = cur_node->next; }
            if (cur_node) { entries.push_back(xmlCopyNode(cur_node,false)); }
        }
        entries.push_back(cur_node);

        return entries;
    }

    std::vector<xmlNode*> XMLparser::getEntriesAtDepth(xmlNode* root_node)
    {
        std::vector<xmlNode*> entries;
        if (!root_node) { return entries; }

        xmlNode* cur_node = root_node;

        // Back up to the first node in this depth
        while (cur_node->prev) { cur_node = cur_node->prev; }

        // Iterate through 'next' values, saving them.
        while (cur_node) 
        {
            entries.push_back(xmlCopyNode(cur_node,false));
            cur_node = cur_node->next;
        }

        return entries;
    }

    void XMLparser::getAllNodeAttributes(xmlNode* a_node, std::vector<xmlAttr*>& outputAttributes)
    {
        xmlAttr* attribute = a_node->properties;
        while (attribute) 
        {
            outputAttributes.push_back(attribute);
            attribute = attribute->next;
        }
    }

    void XMLwriter::startElement(XMLstring name){
        if (xmlTextWriterStartElement(writer, BAD_CAST name.xml_str()) < 0) {
            printf("XMLwriter: Error at xmlTextWriterStartElement\n");
            return;
        }
    }
    void XMLwriter::endElement(){
        if (xmlTextWriterEndElement(writer) < 0) {
            printf("XMLwriter: Error at xmlTextWriterEndElement\n");
            return;
        }
    }
    void XMLwriter::writeAttribute(XMLstring heading, XMLstring content) {
        if (xmlTextWriterWriteAttribute(writer, BAD_CAST heading.xml_str(), BAD_CAST content.xml_str()) < 0) {
            printf("XMLwriter: Error at xmlTextWriterWriteAttribute\n");
                return;
        }
    }
    void XMLwriter::writeComment(XMLstring str) {
        if (xmlTextWriterWriteComment(writer, str.xml_str()) < 0) {
            printf("XMLwriter: Error at xmlTextWriterWriteComment\n");
            return;
        }
    }
    void XMLwriter::writeElement(XMLstring heading, XMLstring str){
        if (xmlTextWriterWriteElement(writer, BAD_CAST heading.xml_str(), BAD_CAST str.xml_str()) < 0) {
            printf("XMLwriter: Error at xmlTextWriterWriteElement\n");
            return;
        }
    }
    void XMLwriter::writeFormatComment(XMLstring str, const char* format) {
        if (xmlTextWriterWriteFormatComment(writer, format, str) < 0) {
            printf("XMLwriter: Error at xmlTextWriterWriteFormatComment\n");
            return;
        }
    }
    void XMLwriter::writeFormatElement(XMLstring heading, double n, const char* format){
        if (xmlTextWriterWriteFormatElement(writer, BAD_CAST heading.xml_str(), format, n) < 0){
            printf("XMLwriter: Error at xmlTextWriterWriteFormatElement\n");
            return;
        }
    }
    void XMLwriter::writeFormatElement(XMLstring heading, float n, const char* format) {
        if (xmlTextWriterWriteFormatElement(writer, BAD_CAST heading.xml_str(), format, n) < 0) {
            printf("XMLwriter: Error at xmlTextWriterWriteFormatElement\n");
            return;
        }
    }
    void XMLwriter::writeFormatElement(XMLstring heading, int n, const char* format) {
        if (xmlTextWriterWriteFormatElement(writer, BAD_CAST heading.xml_str(), format, n) < 0) {
            printf("XMLwriter: Error at xmlTextWriterWriteFormatElement\n");
            return;
        }
    }
    void XMLwriter::writeFormatElement(XMLstring heading, unsigned int n, const char* format) {
        if (xmlTextWriterWriteFormatElement(writer, BAD_CAST heading.xml_str(), format, n) < 0) {
            printf("XMLwriter: Error at xmlTextWriterWriteFormatElement\n");
            return;
        }
    }
}