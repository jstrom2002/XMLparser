/* MIT License

Copyright (c) 2021 JH Strom

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.*/
#pragma once
#include <algorithm>
#include <exception>
#include <fstream>
#include <random>
#include <string>
#include <sstream>
#include <vector>
namespace XMLparser{
#ifdef _WIN64
    typedef uint64_t XMLuint;
#else
    typedef uint32_t XMLuint;
#endif
    enum class ATTRIBUTE_TYPE {NONE,NUMBER,STRING};
    enum class TAG_TYPE {UNKNOWN,CLOSE,COMMENT,OPEN,SELF_CLOSING,XML_DEFINITION};//open=='<...>', close=='</...>', self-closing=='<.../>',comment=='<!-- ... -->',xml=='<?...?>'    
    class XMLattribute {
    public:
        std::string key, value;
        ATTRIBUTE_TYPE type=ATTRIBUTE_TYPE::NONE;
        XMLattribute(std::string str) { // constructor to automatically parse attributes from input string.
            size_t eq = str.find("=");
            if (eq == std::string::npos) { key = str; value = ""; }
            else { key = str.substr(0, eq); value = str.substr(eq + 1, str.length() - eq - 1); }
            if (value.length() > 0) {
                if (value.find("\"") != std::string::npos)
                    type = ATTRIBUTE_TYPE::STRING;
                else {
                    XMLuint cntr = 0;
                    while (cntr < value.length() && value[cntr] <= 32) // find first valid char to test.
                        cntr++;
                    if (std::isdigit(value[cntr]))
                        type = ATTRIBUTE_TYPE::NUMBER;
                }
            }
        }
        std::string ToString() { return key + "=" + value; }
    private:
        XMLattribute() { throw std::exception("ERROR! Cannot use default constructor."); }
    };
    class XMLnode{
    public:
        std::vector<XMLattribute> attributes;
        std::string innerText="";
        std::string tag="";
        TAG_TYPE type=TAG_TYPE::UNKNOWN;
        std::string getGuid() { return guid; }
        std::string ToString() {
            std::string str = "";
            switch (type) {
            case TAG_TYPE::XML_DEFINITION:
                str.append("<?");
                break;
            case TAG_TYPE::COMMENT:
                str.append("<!--");
                break;
            case TAG_TYPE::CLOSE:
                str.append("</");
                break;
            case TAG_TYPE::OPEN:
            case TAG_TYPE::SELF_CLOSING:
                str.append("<");
                break;
            }
            str.append(tag + (attributes.size() > 0 ? " " : ""));
            for (XMLuint i = 0; i < attributes.size(); ++i)
                str.append(attributes[i].ToString() + ((i < attributes.size() - 1) ? " " : ""));             
            switch (type) {
            case TAG_TYPE::XML_DEFINITION:
                str.append("?>");
                break;
            case TAG_TYPE::COMMENT:
                str.append("--!>");
                break;
            case TAG_TYPE::SELF_CLOSING:
                str.append("/>");
                break;
            case TAG_TYPE::OPEN:
            case TAG_TYPE::CLOSE:
                str.append(">");
                break;
            }
            str += innerText;
            return str;
        }
        XMLnode(const std::string& str) {//Construct a node from a string tag entry (with surrounding angle brackets), tokenizing by whitespace to parse values.
            genGuid();
            XMLuint pos = 0, start_pos = 0, end_pos = 0;
            XMLuint str_len = str.length();
            char curr_char = 0, next_char = 0;
            bool in_string = false;
            while (pos < str_len) {
                start_pos = pos;
                end_pos = pos;
                curr_char = str[pos];
                next_char = curr_char;
                pos++;
                while ((next_char != ' ' || in_string == true) && pos < str_len) {
                    next_char = str[pos];
                    end_pos = pos;
                    if (next_char == '\'' || next_char == '\"')
                        in_string = !in_string;
                    pos++;
                }
                if (tag.length() == 0)
                    tag = str.substr(start_pos, end_pos - start_pos + 1);
                else
                    attributes.push_back(XMLattribute(str.substr(start_pos, end_pos - start_pos + 1)));
            }//
            if (tag.find("<?") != std::string::npos)
                type = TAG_TYPE::XML_DEFINITION;
            else if (tag.find("<!--") != std::string::npos)
                type = TAG_TYPE::COMMENT;
            else if (tag.find("</") != std::string::npos)
                type = TAG_TYPE::CLOSE;
            else if (tag.find("<") != std::string::npos)
                type = TAG_TYPE::OPEN;
            else if (attributes[attributes.size() - 1].value.find("/>") != std::string::npos)
                type = TAG_TYPE::SELF_CLOSING;
            std::string* first_str = &tag;
            std::string* last_str = attributes.size() > 0 ? &attributes[attributes.size() - 1].value : &tag;
            switch (type) {// remove ending bracket of entry
            case TAG_TYPE::XML_DEFINITION:
                if (first_str->find("<?") != std::string::npos)
                    *first_str = first_str->substr(2, first_str->length() - 2);
                if (last_str->find("?>") != std::string::npos)
                    *last_str = last_str->substr(0, last_str->length() - 2);
                break;
            case TAG_TYPE::COMMENT:
                if (first_str->find("<!--") != std::string::npos)
                    *first_str = first_str->substr(4, first_str->length() - 4);
                if (last_str->find("--!>") != std::string::npos)
                    *last_str = last_str->substr(0, last_str->length() - 4);
                break;
            case TAG_TYPE::SELF_CLOSING:
                if (first_str->find("<") != std::string::npos)
                    *first_str = first_str->substr(1, first_str->length() - 1);
                if (last_str->find("/>") != std::string::npos)
                    *last_str = last_str->substr(0, last_str->length() - 2);
                break;
            case TAG_TYPE::CLOSE:
                if (first_str->find("</") != std::string::npos)
                    *first_str = first_str->substr(2, first_str->length() - 2);
                if (last_str->find(">") != std::string::npos)
                    *last_str = last_str->substr(0, last_str->length() - 1);
                break;
            case TAG_TYPE::OPEN:
                if (first_str->find("<") != std::string::npos)
                    *first_str = first_str->substr(1, first_str->length() - 1);
                if (last_str->find(">") != std::string::npos)
                    *last_str = last_str->substr(0, last_str->length() - 1);
                break;
            }
        }
    private:
        std::string guid = "";//to maintain node uniqueness, a guid is necessary for strict matching
        void genGuid() { // Generate the 26 digit guid value for uniqueness.
            static const char hexDigits[] = "0123456789ABCDEF";
            guid.clear();
            guid = "";
            guid.reserve(26);
            for (int i = 0; i < 8; ++i) { guid += hexDigits[rand() % (sizeof(hexDigits) - 1)]; }
            guid += '-';
            for (int i = 0; i < 4; ++i) { guid += hexDigits[rand() % (sizeof(hexDigits) - 1)]; }
            guid += '-';
            for (int i = 0; i < 12; ++i) { guid += hexDigits[rand() % (sizeof(hexDigits) - 1)]; }
        }
    };
	class XMLparser{
	public:
        std::vector<XMLnode> nodes;
        XMLparser(){}
        void load(const char* xml_filename, bool remove_formatting = true) { parse(xml_filename, remove_formatting); }
        void save(const char* xml_filename, bool prettify = true) { write_to_disk(xml_filename, prettify); }
        bool check_validation() { do_validation(); }
        std::vector<XMLnode*> find(std::string tag) {
            std::vector<XMLnode*> nds;
            if (tag.length() == 0)
                return nds;
            toLowerCaseAndTrimWhitespace(tag);
            for (int i = 0; i < nodes.size(); ++i) {
                std::string tag2 = nodes[i].tag;
                toLowerCaseAndTrimWhitespace(tag2);
                if (tag == tag2)
                    nds.push_back(&nodes[i]);
            }
            return nds;
        }
    private:
        void toLowerCaseAndTrimWhitespace(std::string& str) {
            std::transform(str.begin(), str.end(), str.begin(), [](unsigned char c) { return std::tolower(c); });//make tag to check lowercase.
            while (str[0]==' ')
                str = str.substr(1);
            while (str[str.length()-1]==' ')
                str = str.substr(0,str.length()-1);
        }
        bool do_validation() {
            if (nodes.size() == 0)
                return false;
            for (int i = 0; i < nodes.size(); ++i) {
                bool found_match = false;
                std::vector<XMLnode*> found_nodes;
                switch (nodes[i].type) {
                case TAG_TYPE::OPEN: 
                    found_nodes = find(nodes[i].tag);
                    for (int j = 0; j < found_nodes.size(); ++j) {
                        if (j == i)
                            continue;
                        if (nodes[i].tag == found_nodes[j]->tag && nodes[i].getGuid() != found_nodes[j]->getGuid() && found_nodes[j]->type == TAG_TYPE::CLOSE) {
                            found_match = true;
                            break;
                        }
                    }
                    break;
                case TAG_TYPE::CLOSE:
                    found_nodes = find(nodes[i].tag);
                    for (int j = 0; j < found_nodes.size(); ++j) {
                        if (j == i)
                            continue;
                        if (nodes[i].tag == found_nodes[j]->tag && nodes[i].getGuid() != found_nodes[j]->getGuid() && found_nodes[j]->type == TAG_TYPE::OPEN) {
                            found_match = true;
                            break;
                        }
                    }
                    break;
                case TAG_TYPE::COMMENT:
                case TAG_TYPE::SELF_CLOSING:
                case TAG_TYPE::UNKNOWN:
                case TAG_TYPE::XML_DEFINITION:
                    found_match = true;
                    break;
                }
                if (!found_match)
                    return false;
            }
            return true;
        }
        void parse(const char* xml_filename, bool remove_formatting){//iterate over all tags in file saving position of < and > as ints
            srand(clock());//fully randomize guids
            std::ifstream fin(xml_filename, std::ios::in | std::ios::binary);
            char curr_char = 0;
            std::string innerText="";
            std::string str="";
            while (true) {
                if (fin.eof()) { break; }
                while (curr_char != '<' && !fin.eof()) {
                    if(curr_char != '<' && curr_char != '>' && (!remove_formatting || curr_char >= 32) )
                        innerText += curr_char;
                    fin.get(curr_char);
                }
                if (fin.eof()) { break; }
                str += curr_char;
                while (curr_char != '>' && !fin.eof()) {
                    fin.get(curr_char);
                    str += curr_char;
                }
                if (fin.eof()) { break; }
                if(nodes.size()>0)
                    nodes.back().innerText = innerText;
                nodes.push_back(XMLnode(str));
                str = innerText = "";
            }
            fin.close();
            bool validXML = do_validation();
            if (!validXML)
                throw std::exception("ERROR! Parsed XML is not valid.");
        }
        void write_to_disk(const char* xml_filename, bool prettify) {
            std::ofstream fout(xml_filename, std::ios::out | std::ios::binary | std::ios::app);
            for (auto nd : nodes) {
                std::string str = nd.ToString();
                fout.write(str.c_str(), str.size());
            }
            fout.close();
        }        
	};
}