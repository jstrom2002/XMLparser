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
#include <exception>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
namespace XMLparser{
#ifdef _WIN64
    typedef uint64_t XMLuint;
#else
    typedef uint32_t XMLuint;
#endif
    enum class ATTRIBUTE_TYPE {NUMBER, STRING};
    enum class TAG_TYPE {UNKNOWN,CLOSE,COMMENT,OPEN,SELF_CLOSING,XML};//open=='<...>', close=='</...>', self-closing=='<.../>',comment=='<!-- ... -->',xml=='<?...?>'    
    class XMLattribute {
    public:
        std::string key, value;
        XMLattribute(std::string str) { // constructor to automatically parse attributes from input string.
            size_t eq = str.find("=");
            if (eq == std::string::npos) { key = str; value = ""; }
            else { key = str.substr(0, eq); value = str.substr(eq + 1, str.length() - eq - 1); }
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
        XMLnode(){}
        //XMLnode(XMLuint _begin, XMLuint _end) : begin_pos(_begin), end_pos(_end) {}
        TAG_TYPE type=TAG_TYPE::UNKNOWN;
        std::string ToString() {
            std::string str = "";
            switch (type) {
            case TAG_TYPE::XML:
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
            case TAG_TYPE::XML:
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
    private:
        //XMLnode() : begin_pos(0),end_pos(0) { throw std::exception("ERROR! Cannot use default constructor."); }
    };
	class XMLreader{
	public:
        const bool remove_formatting;
        std::vector<XMLnode> nodes;
        XMLreader(const char* filename, bool rmv_formatting = true) : remove_formatting(rmv_formatting) { parse(filename); }
    private:
        void semanticParsing(XMLnode& nd, const std::string& str) {//tokenize tag contents by whitespace and parse values.
            XMLuint pos=0,start_pos=0,end_pos=0;
            XMLuint str_len = str.length();
            char curr_char=0,next_char=0;
            bool in_string = false;
            while (pos < str_len){
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
                if (nd.tag.length() == 0)
                    nd.tag = str.substr(start_pos, end_pos - start_pos + 1);
                else
                    nd.attributes.push_back(XMLattribute(str.substr(start_pos, end_pos-start_pos+1)));
            }//
            if (nd.tag.find("<?") != std::string::npos)
                nd.type = TAG_TYPE::XML;
            else if (nd.tag.find("<!--") != std::string::npos)
                nd.type = TAG_TYPE::COMMENT;
            else if (nd.tag.find("</") != std::string::npos)
                nd.type = TAG_TYPE::CLOSE;
            else if (nd.tag.find("<") != std::string::npos)
                nd.type = TAG_TYPE::OPEN;
            else if (nd.attributes[nd.attributes.size() - 1].value.find("/>") != std::string::npos)
                nd.type = TAG_TYPE::SELF_CLOSING;
            std::string* first_str = &nd.tag;
            std::string* last_str = nd.attributes.size() > 0 ? &nd.attributes[nd.attributes.size() - 1].value : &nd.tag;
            switch (nd.type) {// remove ending bracket of entry
            case TAG_TYPE::XML:
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
        void parse(const char* xml_filename){//iterate over all tags in file saving position of < and > as ints
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
                nodes.push_back(XMLnode());
                semanticParsing(nodes.back(), str);
                str = innerText = "";
            }
            fin.close();
        }
	};
}