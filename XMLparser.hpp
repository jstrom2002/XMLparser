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
#include <cstdlib>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
namespace XMLparser{
    const struct TAG_TYPE { enum types : size_t {CLOSE=1u,COMMENT=2u,OPEN=3u,SELF_CLOSING=4u,XML_DEFINITION=5u,TAG_COUNT=6u};}tag_type;// replacement for enum which is not available in c++98, open=='<...>', close=='</...>', self-closing=='<.../>',comment=='<!-- ... -->',xml=='<?...?>' 0          
    std::string toLower(std::string str) { for (size_t i = 0; i < str.length(); ++i) { str[i] = std::tolower(str[i]); } return str; }
    std::string trimWhitespace(std::string str) { while (str[0] == ' ') { str = str.substr(1); }while (str[str.length() - 1] == ' ') { str = str.substr(0, str.length() - 1); }return str; }
    std::string TAG_BEGINNINGS[(tag_type.TAG_COUNT)]{"","</","<!--","<","<","<?"};
    std::string TAG_ENDINGS[tag_type.TAG_COUNT]{"",">","--!>",">","/>","?>"};
    void XML_EXCEPTION(std::string err_str) { throw std::invalid_argument(err_str.c_str()); }
    class XMLattribute {
    public:
        std::string key, value;
        XMLattribute(std::string str){
            size_t eq = str.find("=");
            if(eq==std::string::npos){key=str;value="";}
            else{key=str.substr(0,eq);value=str.substr(eq+1,str.length()-eq-1);}
        }
        std::string ToString() { return key + "=" + value; }
    private:
        XMLattribute() { XML_EXCEPTION("ERROR! Cannot use default constructor."); }
    };
    class XMLnode{
    public:
        std::vector<XMLattribute> attributes;
        std::string innerText,tag;
        unsigned int type;
        std::string getGuid() { return guid; }
        bool hasMatchingTag() { return (type == tag_type.OPEN || type == tag_type.CLOSE); }
        bool tagsEqual(const std::string& str) { return toLower(trimWhitespace(tag)) == toLower(trimWhitespace(str)); }
        std::string ToString() {
            std::string str = TAG_BEGINNINGS[int(type)]+(tag+(attributes.size()>0?" ":""));
            for (unsigned int i=0;i<attributes.size();++i)
                str += (attributes[i].ToString()+((i<attributes.size() - 1)?" ":""));             
            return str+TAG_ENDINGS[int(type)]+innerText;
        }
        XMLnode(const std::string& str) {
            if (str.length() == 0) { // validation
                XML_EXCEPTION("ERROR! Attempting to parse bad data to XML node entry.");
            }
            unsigned int pos = 0, start_pos = 0, end_pos = 0;
            char next_char = 0;
            bool in_string = false;
            while (pos < str.length()) {// Split string by whitespace into values: first value is the tag, the rest are attributes. Last attribute has closing bracket.
                start_pos = end_pos = pos;
                next_char = str[pos];
                pos++;
                while ((next_char != ' ' || in_string == true) && pos < str.length()) {
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
            }
            if (tag.find("<?") != std::string::npos) // identify type
                type = tag_type.XML_DEFINITION;
            else if (tag.find("<!--") != std::string::npos)
                type = tag_type.COMMENT;
            else if (tag.find("</") != std::string::npos)
                type = tag_type.CLOSE;
            else if (tag.find("/>") != std::string::npos || (attributes.size() > 0 && attributes[attributes.size() - 1].value.find("/>") != std::string::npos))
                type = tag_type.SELF_CLOSING;
            else if (tag.find("<") != std::string::npos)
                type = tag_type.OPEN;
            std::string* first_str = &tag; // remove tag bracket surroundings
            std::string* last_str = (attributes.size() > 0) ? &attributes[attributes.size() - 1].value : &tag;
            if (first_str->find(TAG_BEGINNINGS[int(type)]) != std::string::npos)
                *first_str = first_str->substr(TAG_BEGINNINGS[int(type)].length(),first_str->length() - TAG_BEGINNINGS[int(type)].length());
            if (last_str->find(TAG_ENDINGS[int(type)]) != std::string::npos)
                *last_str = last_str->substr(0,last_str->length() - TAG_ENDINGS[int(type)].length());
            genGuid();
        }
    private:
        std::string guid = "";//to maintain node uniqueness, a guid is necessary for strict matching
        void genGuid(){
            guid.clear(); guid.reserve(26); char c;
            for (int i = 0; i < 26; ++i) {
                c = '-';
                if (i != 9 && i != 14) {
                    c = (char)(rand() % 16) + 48;
                    if (c >= 58) { c += 7; }
                }
                guid.push_back(c);
            }
        }
    };
	class XMLparser{
	public:
        std::vector<XMLnode> nodes;
        void load(const char* xml_filename, bool remove_formatting = true) { parse(xml_filename, remove_formatting); }
        void save(const char* xml_filename, bool prettify = true) { write_to_disk(xml_filename, prettify); }
        bool check_validation() { return do_validation(); }
        std::vector<XMLnode*> find(std::string tag) {
            std::vector<XMLnode*> nds;
            if (tag.length() == 0)
                return nds;
            for (unsigned int i=0;i<nodes.size();++i) {
                if (nodes[i].tagsEqual(tag))
                    nds.push_back(&nodes[i]);
            }
            return nds;
        }
        XMLnode* findTagPair(XMLnode* nd) {
            for (unsigned int i = 0; i < nodes.size(); ++i){
                if(nd->tagsEqual(nodes[i].tag)&&nd->getGuid()!=nodes[i].getGuid()&&((nd->type == tag_type.OPEN && nodes[i].type == tag_type.CLOSE)
                    || (nd->type == tag_type.CLOSE && nodes[i].type == tag_type.OPEN)))
                    return (&nodes[i]);
            }
            return 0;
        }
    private:
        bool do_validation() { // TO DO: add more complex validation than simple tag matching.
            for (unsigned int i = 0; i < nodes.size(); ++i) {
                if (nodes[i].hasMatchingTag() && findTagPair(&nodes[i])==0) { // Make sure there are matching open/close tag pairs.
                    XML_EXCEPTION(("ERROR! Validation failed. Could not find matching tag pair for open/close tags: " + nodes[i].tag).c_str());
                }
            }
            return (nodes.size()>0) ? true : false;
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
                if (!(str.find("<") != std::string::npos && str.find(">") != std::string::npos)) {
                    XML_EXCEPTION("ERROR! Formatting of tag entry is corrupted.");
                }
                nodes.push_back(XMLnode(str));
                str = innerText = "";
            }
            fin.close();
        }
        void write_to_disk(const char* xml_filename, bool prettify) {
            std::ofstream fout(xml_filename, std::ios::out | std::ios::binary | std::ios::app);
            for (unsigned int i = 0; i < nodes.size();++i){ std::string str = nodes[i].ToString(); fout.write(str.c_str(), str.size()); }
            fout.close();
        }        
	};
}