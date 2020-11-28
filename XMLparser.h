/**
*   Wrapper class for parsing .XML formatted files using libxml2.
*   See libxml2's examples at: http://www.xmlsoft.org/examples
*   Note: files should be unindented according to libxml2 spec.
*   Note: all functions use 'xmlChar*' strings, which use unsigned chars rather than
*   signed chars, as in the C standard. This is to make re-serialization possible.
*/
#pragma once
#include <libxml/xmlversion.h>
#if defined(LIBXML_WRITER_ENABLED) && defined(LIBXML_OUTPUT_ENABLED)

#include <string>
#include <vector>
#include <iostream>
#include <stdio.h>
#include <libxml/tree.h>
#include <libxml/xmlwriter.h>

namespace XML
{
    // Helper functions for xmlChar*.
    inline std::string to_string(const xmlChar* str)
    {
        if (str == nullptr || str[0] == '\0')
            return "";

        unsigned char* len = (unsigned char*)str;
        return std::string((char*)len);
    }
    inline char* to_Cstr(const xmlChar* str)
    {
        if (str == nullptr || str[0] == '\0')
            return (char*)"\0";

        unsigned char* len = (unsigned char*)str;
        return (char*)len;
    }
    inline size_t strlength(const xmlChar* str)
    {
        return to_string(str).length();
    }

    // Helper class to auto-convert C/C++ strings to xmlChar* arrays to pass to functions.
    class XMLstring
    {
    public:
        XMLstring();
        XMLstring(char* str);
        XMLstring(const char* str);
        XMLstring(std::string str);
        ~XMLstring();

        std::string str();
        xmlChar* xml_str();
    private:
        xmlChar* xstr;
    };


    /*
    *   Base XML reader/writer class.    
    */
    class XMLbase
    {
    public:
        std::string filepath;
        std::string encoding;

        // Helper function for converting strings to match encoding standard.
        XMLstring ConvertInput(std::string in);

    private:
        std::string convertHexValsToString(std::string str);
    };


    /**
    *   Main file saving interface: will open an .XML file and close and flush it when writing is done.
    *   Users can call writing functions before a XMLwriter object is destroyed.
    */
    class XMLwriter : public XMLbase
    {
    public:
        xmlTextWriterPtr writer;
        xmlDocPtr doc;

        XMLwriter() = delete;//no default init.        
        XMLwriter(std::string filename, std::string encoding = "ISO-8859-1");
        ~XMLwriter();

        void startElement(XMLstring name);
        void endElement();
        void writeAttribute(XMLstring heading, XMLstring content);
        void writeComment(XMLstring str);
        void writeElement(XMLstring heading, XMLstring str);//Note: use this function to write all string values.
        void writeFormatComment(XMLstring str, const char* format = "%s");
        void writeFormatElement(XMLstring heading, double n, const char* format = "%d");
        void writeFormatElement(XMLstring heading, float n, const char* format = "%f");
        void writeFormatElement(XMLstring heading, int n, const char* format = "%i");
        void writeFormatElement(XMLstring heading, unsigned int n, const char* format = "%u");
    };


    /**
    *  \brief Main file loading interface: this will load a .XML from a given filepath and pass its DOM off to helpers.
    */
    class XMLparser : public XMLbase
    {
        public:
        xmlNode* root_element;
        xmlDocPtr doc;

        /*
        *   Linear array of all nodes in the XML file to make searching easier.
        */
        std::vector<xmlNode*> nodes;

        XMLparser() = delete;//No default init.
        XMLparser(std::string filepath, std::string datatype = "ASCII");
        ~XMLparser();

        /**
        *  \brief Helper function to return the first entry matching a certain name, which wull be 'nullptr' if no match is found.
        */
        xmlNode* findNodeByName(std::vector<xmlNode*> nodes, std::string nodeName);

        /**
        *  \brief Helper function to return the index for the first entry matching a certain name, which will be -1 if no match is found.
        */
        int findIndexByName(std::vector<xmlNode*> nodes, std::string nodeName);

        /**
        *  \brief Helper function to return all entries matching a certain name.
        */
        std::vector<xmlNode*> findNodesByName(xmlNode* a_node, std::string nodeName);

        /**
        *  \brief Helper function to return all entries matching a certain name from this object's saved nodes.
        */
        std::vector<xmlNode*> findNodesByName(std::string nodeName);

        /**
        *  \brief Helper function to return all entries matching a certain name from an input vector of element nodes.
        */
        std::vector<xmlNode*> findNodesByName(std::vector<xmlNode*> nodes, std::string nodeName);

        /**
        *  \brief Helper function to recurse through all entries at depths below the root node and add them to an array of nodes.
        *         Ref: http://www.xmlsoft.org/examples/tree1.c
        */
        void getSubtree(xmlNode* root_node, std::vector<xmlNode*>& outputNodes);

        /**
        *  \brief Return an array of all nodes (including the 'root_node') in a branch at depths greater than that of
        *         the 'root_node' param by depth-first traversal.
        */
        std::vector<xmlNode*> getBranch(xmlNode* root_node);

        /**
        *  \brief Return an array of all nodes at the same depth as the 'root_node' param by breadth-first traversal.
        */
        std::vector<xmlNode*> getEntriesAtDepth(xmlNode* root_node);

        /**
        *  \brief Helper function to get all attributes for a node (ie for '<node id=0 name="example1">' it would return id and name xmlAttr*s).
        */
        void getAllNodeAttributes(xmlNode* a_node, std::vector<xmlAttr*>& outputAttributes);
    };
#endif
}