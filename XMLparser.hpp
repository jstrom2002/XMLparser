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
#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
namespace XMLparser{
#ifdef _WIN64
    typedef uint64_t XMLuint;
#else
    typedef uint32_t XMLuint;
#endif
    XMLuint MAX_FILESIZE = 9999999999999u;
    enum class TAG_TYPE{UNKNOWN,COMMENT,CLOSE,OPEN,SELF_CLOSING,UNIQUE,XML_DEFINITION,TAG_COUNT};// replacement for enum which is not available in c++98, open=='<...>', close=='</...>', self-closing=='<.../>',comment=='<!-- ... -->',xml=='<?...?>' 0          
    std::string toLower(std::string str) { for (size_t i = 0; i < str.length(); ++i) { str[i] = std::tolower(str[i]); } return str; }
    void trimWhitespace(std::string& str){while(str.length()>0&&str[0]==' '){str=str.substr(1);}while(str.length()>0&&str[str.length()-1]==' '){str=str.substr(0,str.length()-1);}}
    std::string TAG_BEGINNINGS[int(TAG_TYPE::TAG_COUNT)]={"","<!--","</","<","<","<!","<?"};
    std::string TAG_ENDINGS[int(TAG_TYPE::TAG_COUNT)]={"","-->",">",">","/>",">","?>"};
    void removeTagBeginnings(std::string& str){
        const std::string to_remove[]={"<!--","</","<!","<?","<"};
        for(int i=0;i<5;++i){
            if (str.length()>0&&str.find(to_remove[i])!=std::string::npos)
                str=str.substr(to_remove[i].length(),str.length()-to_remove[i].length());
        }
    }
    void removeTagEndings(std::string& str) {
        const std::string to_remove[]={"-->","/>","?>",">"};
        for(int i=0;i<4;++i){
            if (str.length()>0&&str.rfind(to_remove[i])!=std::string::npos)
                str=str.substr(0,str.length()-to_remove[i].length());
        }
    }
    TAG_TYPE typeFromTag(const std::string& str) {
        if(str.find("<!--")!=std::string::npos||str.rfind("-->")!=std::string::npos){return TAG_TYPE::COMMENT; }
        else if(str.find("<?")!=std::string::npos||str.rfind("?>")!=std::string::npos){return TAG_TYPE::XML_DEFINITION; }
        else if(str.find("</")!=std::string::npos){return TAG_TYPE::CLOSE; }
        else if(str.find("<!")!=std::string::npos){return TAG_TYPE::UNIQUE;}
        else if(str.rfind("/>")!=std::string::npos){return TAG_TYPE::SELF_CLOSING;}
        else if(str.find("<")!=std::string::npos){return TAG_TYPE::OPEN;}
    }
    void XML_EXCEPTION(std::string err_str) { throw std::invalid_argument(err_str.c_str()); }
    bool hasUnclosedComment(const std::string& str){return str.length()>4&&str.find("<!--")!=std::string::npos&&str.rfind("-->")==std::string::npos;}
    class XMLattribute {
    public:
        std::string key, value;
        XMLattribute(std::string str){
            key=value="";
            size_t eq=str.find("=");
            if(eq==std::string::npos){
                //XML_EXCEPTION("ERROR! Values for XML attributes must follow format 'key=value' for XML 1.0/1.1.");
                key=str;
            }
            else{key=str.substr(0,eq);value=str.substr(eq+1,str.length()-eq-1);}
        }
        std::string ToString(){return key+"="+value;}
        bool validate(){return key.length()>0&&key[0]!='<';}
    };
    class XMLnode{
    public:
        std::vector<XMLattribute> attributes;
        std::vector<std::shared_ptr<XMLnode>> children;
        std::string innerText="", tag="";
        std::shared_ptr<XMLnode> parent=nullptr;
        TAG_TYPE type=TAG_TYPE::UNKNOWN;
        std::string getGuid(){return guid;}
        bool isOpenOrClose(){return (type==TAG_TYPE::OPEN||type==TAG_TYPE::CLOSE);}
        bool tagsEqual(std::string str){ return toLower(tag)==toLower(str); }//tags of nodes are identical, which are case-insensitive.
        bool exactMatch(std::shared_ptr<XMLnode> nd){return guid==nd->getGuid();}//nodes are exactly identical, same guid.
        std::string ToString() {
            std::string str = TAG_BEGINNINGS[int(type)]+(tag+(attributes.size()>0?" ":""));
            for (XMLuint i=0;i<attributes.size();++i)
                str += (attributes[i].ToString()+((i<attributes.size() - 1)?" ":""));             
            return str+TAG_ENDINGS[int(type)]+innerText;
        }
        bool validate() {
            bool is_valid=tag.length()>0&&guid.length()>0&&type!=TAG_TYPE::UNKNOWN;
            for (XMLuint i=0;i<attributes.size();++i)
                is_valid &= attributes[i].validate();
            return is_valid;
        }
        XMLnode(std::string str, std::shared_ptr<XMLnode> _parent=nullptr){
            if (str.length()==0){XML_EXCEPTION("ERROR! Attempting to parse bad data to XML node entry.");}
            genGuid();
            parent = _parent;
            XMLuint pos=0u,start_pos=0u,end_pos=0u;
            char next_char = 0,curr_string_char=0;
            bool in_string = false;
            TAG_TYPE start_tp=TAG_TYPE::UNKNOWN,end_tp=TAG_TYPE::UNKNOWN;
            while (pos<str.length()) {// Split string by whitespace into values: first value is the tag, the rest are attributes. Last attribute has closing bracket.
                start_pos = end_pos = pos;
                next_char = str[pos];
                pos++;
                while ((next_char != ' '||in_string==true) && pos < str.length()) {
                    next_char = str[pos];
                    end_pos = pos;
                    if (next_char == '\''){
                        in_string = !in_string;
                        curr_string_char=in_string?next_char:0;
                    }
                    else if(curr_string_char!='\''&&next_char=='\"'){
                        in_string = !in_string;
                        curr_string_char=in_string?next_char:0;
                    }
                    pos++;
                }
                if(tag.length()==0){
                    tag=str.substr(start_pos,end_pos-start_pos+1);
                    trimWhitespace(tag);
                    if(tag.find("<!--")!=std::string::npos){tag="";type=TAG_TYPE::COMMENT;return;}
                    start_tp=typeFromTag(tag);
                    type = start_tp;
                    removeTagBeginnings(tag);
                    removeTagEndings(tag);
                    if (tag.length() == 0) {
                        XML_EXCEPTION("ERROR! Could not parse tag for entry: " + str);
                    }
                }
                else {
                    std::string attr_str=str.substr(start_pos, end_pos-start_pos+1);
                    if (end_pos>=str.length()-1){
                        end_tp=typeFromTag(attr_str);
                        removeTagEndings(attr_str);
                    }
                    trimWhitespace(attr_str);
                    if(attr_str.length()>0)
                        attributes.push_back(XMLattribute(attr_str));
                }
            }
            if(start_tp==TAG_TYPE::OPEN&&end_tp==TAG_TYPE::SELF_CLOSING)
                type=end_tp;
        }
    private:
        std::string guid;//for node uniqueness
        void genGuid(){
            guid.clear(); guid.resize(26,' '); char c;
            for (int i = 0; i < 26; ++i) {
                c = '-';
                if (i != 9 && i != 14) {
                    c = (char)(rand() % 16) + 48;
                    if (c >= 58) { c += 7; }
                }
                guid[i]=(c);
            }
        }
    };
	class XMLparser{
	public:
        std::vector<std::shared_ptr<XMLnode>> nodes;
        void load(const char* xml_filename, bool remove_formatting = true) { parse(xml_filename, remove_formatting); }
        //void prettify() { do_prettification(); }
        void save(const char* xml_filename) { write_to_disk(xml_filename); }
        bool check_validation() { return do_validation(); }
        std::vector<std::shared_ptr<XMLnode>> find(std::string tag) {
            std::vector<std::shared_ptr<XMLnode>> nds;
            if (tag.length() == 0)
                return nds;
            for (XMLuint i=0;i<nodes.size();++i) {
                if (nodes[i]->tagsEqual(tag))
                    nds.push_back(nodes[i]);
            }
            return nds;
        }
        std::shared_ptr<XMLnode> findTagPair(std::shared_ptr<XMLnode> nd) {
            if (!nd->isOpenOrClose())
                return nullptr;
            int inc=(nd->type==TAG_TYPE::CLOSE)?-1:1;
            for (XMLuint i=0u;i<nodes.size();++i){
                if (nd->exactMatch(nodes[i])){
                    int level=(nd->type==TAG_TYPE::CLOSE)?-1:1;
                    for (int j=(int)i+inc;j<(int)nodes.size()&&j>=0;j+=inc){
                        if(nodes[j]->isOpenOrClose()){level+=(nodes[j]->type==TAG_TYPE::CLOSE)?-1:1;}
                        if (level==0&&nd->tagsEqual(nodes[j]->tag)&&nd->exactMatch(nodes[j])==false&&
                            ((nd->type == TAG_TYPE::OPEN && nodes[j]->type == TAG_TYPE::CLOSE) || (nd->type == TAG_TYPE::CLOSE && nodes[j]->type == TAG_TYPE::OPEN))
                          )
                            return nodes[j];                        
                    }
                    break;
                }
            }
            return 0;
        }
    private:
        bool do_validation() { // TO DO: add more complex validation than simple tag matching.
            for (XMLuint i = 0; i < nodes.size(); ++i) {
                if (nodes[i]->tag.find("<") != std::string::npos || nodes[i]->tag.find(">") != std::string::npos) {//make sure tags have no angle brackets any more
                    XML_EXCEPTION("ERROR! Formatting of tag entry is corrupted: " + nodes[i]->tag);
                }
                if (nodes[i]->isOpenOrClose()){ // validation for open/close tag pairs
                    std::shared_ptr<XMLnode> pair=findTagPair(nodes[i]);
                    if (pair==nullptr) { // Make sure there are matching open/close tag pairs.
                        XML_EXCEPTION(("ERROR! Validation failed. Could not find matching tag pair for open/close tags: " + nodes[i]->tag).c_str());
                    }
                    if (pair->parent != nodes[i]->parent) { // make sure node tree relationship is preserved from file.

                        XML_EXCEPTION(("ERROR! Validation failed. Node tree relationship mismatching for open/close tags: " + nodes[i]->tag).c_str());
                    }
                }
            }
            return (nodes.size()>0) ? true : false;
        }
        void do_prettification() {// destructive op to remove whitespace and other unnecessary values from XML
            //if (& str.find("\r\n") == std::string::npos) { str.append("\r\n"); }
        }
        void parse(const char* xml_filename, bool remove_formatting){//iterate over all tags in file saving position of < and > as ints
            srand(clock());//fully randomize guids
            std::ifstream fin(xml_filename, std::ios::in|std::ios::binary);            
            XMLuint fsize = fin.tellg();
            fin.seekg(0, std::ios::end);
            fsize = (XMLuint)(fin.tellg()) - fsize;// get filesize
            if (fsize > MAX_FILESIZE)
                XML_EXCEPTION("ERROR! File cannot be parsed because it is too large for memory to hold.");
            fin.seekg(0, std::ios::beg);
            char curr_char=0;
            bool in_comment=false;
            std::string innerText,str;
            std::shared_ptr<XMLnode> curr_parent=nullptr;
            char* tmp;
            while (true){
                if (fin.peek() == EOF) 
                    break;
                while (curr_char!='<'&&fin.get(curr_char)){
                    if (curr_char!='<'&&curr_char!='>'&&curr_char>31)
                        innerText += curr_char;                                        
                }
                str += curr_char;
                while((curr_char!='>'||in_comment)&&fin.get(curr_char)){
                    if(curr_char>31)
                        str += curr_char;
                    in_comment=hasUnclosedComment(str)?true:false;
                } 
                if(nodes.size()>0)
                    nodes.back()->innerText=innerText;
                std::shared_ptr<XMLnode> nd=std::make_shared<XMLnode>(str, curr_parent);
                if(nd->type!=TAG_TYPE::COMMENT){nodes.push_back(nd);}
                else{nd->tag=nd->innerText="";}
                if(curr_parent==nullptr&&nd->type==TAG_TYPE::OPEN){curr_parent=nodes.back();}//initialization
                if (curr_parent != nullptr && nodes.back()->exactMatch(curr_parent)==false&&nd->type!=TAG_TYPE::CLOSE) { //assign children
                    curr_parent->children.push_back(nodes.back()); }
                if(nodes.back()->type==TAG_TYPE::OPEN){curr_parent=nodes.back();}//make current node the parent
                else if (nodes.back()->type == TAG_TYPE::CLOSE) {//backtrack
                    std::shared_ptr<XMLnode> match_nd = findTagPair(nodes.back());
                    if(match_nd){curr_parent=match_nd->parent;nodes.back()->parent=curr_parent;}
                }
                //else{curr_parent=nullptr;}
                str = innerText = ""; 
                in_comment = false;
            }
            fin.close();
        }
        void write_to_disk(const char* xml_filename) {
            std::ofstream fout(xml_filename, std::ios::out | std::ios::binary | std::ios::app);
            for (XMLuint i = 0; i < nodes.size(); ++i) { std::string str = nodes[i]->ToString();fout.write(str.c_str(), str.size()); }
            fout.close();
        }        
	};
}