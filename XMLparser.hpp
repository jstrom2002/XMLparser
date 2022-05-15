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
#include <codecvt>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <locale>
#include <memory>
#include <random>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#include <Windows.h>
#endif
namespace XMLparser{
#ifdef _WIN64
    typedef uint64_t XMLuint;
#else
    typedef uint32_t XMLuint;
#endif
    typedef std::wifstream XMLifstream;
    typedef std::wofstream XMLofstream;
    typedef wchar_t XMLchar;
    typedef std::wstring XMLstring;
    XMLuint MAX_FILESIZE = 9999999999999u;
    enum class XML_TAG_TYPE{UNKNOWN,COMMENT,CLOSE,OPEN,SELF_CLOSING,UNIQUE,XML_DEFINITION,COUNT};// replacement for enum which is not available in c++98, open=='<...>', close=='</...>', self-closing=='<.../>',comment=='<!-- ... -->',xml=='<?...?>' 0          
    enum class XML_ENCODING{UNKNOWN,UTF_8,UTF_16,UTF_32,ISO_8859_1,ISO_10646_UCS_2,SHIFT_JIS,ASCII,COUNT};    
    XMLstring toLower(XMLstring str){for(size_t i=0;i<str.length();++i){str[i]=std::tolower(str[i]);}return str;}
    void trimWhitespace(XMLstring& str){while(str.length()>0&&str[0]==L' '){str=str.substr(1);}while(str.length()>0&&str[str.length()-1]==L' '){str=str.substr(0,str.length()-1);}}
    void removeFormattingChars(XMLstring& str){for(int i=0;i<str.length();++i)if(str[i]<L' '){str.erase(str.begin()+i);i--;}}
    void removeFormattingChars(std::string& str){for(int i=0;i<str.length();++i)if(str[i]<' '){str.erase(str.begin()+i);i--;}}
    std::wstring s2ws(const std::string& str){return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().from_bytes(str);}
    std::string ws2s(const std::wstring& wstr){return std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>().to_bytes(wstr);}
    //std::u16string ws2u16s(const std::wstring& wstr){
    //    std::wstring_convert<std::codecvt_utf16<char16_t>, char16_t> convert;
    //    std::u16string b = convert.from_bytes(wstr);
    //}
    std::wstring u16s2ws(const std::u16string& wstr){
        std::wstring_convert<std::codecvt_utf16<wchar_t, 0x10ffff, std::little_endian>, wchar_t> conv;
        return conv.from_bytes(reinterpret_cast<const char*> (&wstr[0]), reinterpret_cast<const char*> (&wstr[0] + wstr.size()));
    }
    std::u16string s2u16s(char* str) { std::u16string u16s = reinterpret_cast<char16_t*>(str); return u16s; }
    std::string u16s2s(char16_t* str) { return reinterpret_cast<char*>(str); }
    std::u32string s2u32s(char* str) { std::u32string u32s = reinterpret_cast<char32_t*>(str); return u32s; }
    std::string u32s2s(char32_t* str) { return reinterpret_cast<char*>(str); }
    XMLstring TAG_BEGINNINGS[int(XML_TAG_TYPE::COUNT)]={L"",L"<!--",L"</",L"<",L"<",L"<!",L"<?"};
    XMLstring TAG_ENDINGS[int(XML_TAG_TYPE::COUNT)]={L"",L"-->",L">",L">",L"/>",L">",L"?>"};
    void removeTagBeginnings(XMLstring& str){
        const XMLstring to_remove[]={L"<!--",L"</",L"<!",L"<?",L"<"};
        for(int i=0;i<5;++i){
            if (str.length()>0&&str.find(to_remove[i])!=XMLstring::npos)
                str=str.substr(to_remove[i].length(),str.length()-to_remove[i].length());
        }
    }
    void removeTagEndings(XMLstring& str) {
        const XMLstring to_remove[]={L"-->",L"/>",L"?>",L">"};
        for(int i=0;i<4;++i){
            if (str.length()>0&&str.rfind(to_remove[i])!=XMLstring::npos)
                str=str.substr(0,str.length()-to_remove[i].length());
        }
    }
    XML_TAG_TYPE typeFromTag(const XMLstring& str) {
        if(str.find(L"<!--")!=XMLstring::npos||str.rfind(L"-->")!=XMLstring::npos){return XML_TAG_TYPE::COMMENT; }
        else if(str.find(L"<?")!=XMLstring::npos){return XML_TAG_TYPE::XML_DEFINITION; }
        else if(str.find(L"</")!=XMLstring::npos){return XML_TAG_TYPE::CLOSE; }
        else if(str.find(L"<!")!=XMLstring::npos){return XML_TAG_TYPE::UNIQUE;}
        else if(str.rfind(L"/>")!=XMLstring::npos){return XML_TAG_TYPE::SELF_CLOSING;}
        else if(str.find(L"<")!=XMLstring::npos){return XML_TAG_TYPE::OPEN;}
    }
    void XML_EXCEPTION(XMLstring err_str){
        throw std::invalid_argument(ws2s(err_str).c_str());
    }
    bool hasUnclosedComment(const XMLstring& str){return str.length()>4&&str.find(L"<!--")!=XMLstring::npos&&str.rfind(L"-->")==XMLstring::npos;}
    class XMLattribute {
    public:
        XMLstring key, value;
        XMLattribute(XMLstring str){
            key=value=L"";
            size_t eq=str.find(L"=");
            if(eq==XMLstring::npos)
                key=str;            
            else{key=str.substr(0,eq);value=str.substr(eq+1,str.length()-eq-1);}
        }
        XMLstring ToString(){return key+((value.length()>0)?(L"="+value):(L"")); }
        bool validate(){return key.length()>0&&key[0]!=L'<';}
    };
    class XMLnode{
    public:
        std::vector<std::shared_ptr<XMLattribute>> attributes;
        std::vector<std::shared_ptr<XMLnode>> children;
        XMLstring innerText=L"", tag=L"";
        std::shared_ptr<XMLnode> parent=nullptr;
        XML_TAG_TYPE type=XML_TAG_TYPE::UNKNOWN;
        bool exists(const XMLstring& key){for(XMLuint i=0;i<attributes.size();++i){if(attributes[i]->key==key)return true;}return false;}
        std::shared_ptr<XMLattribute> getAttribute(const XMLstring& key){for(XMLuint i=0;i<attributes.size();++i){if(attributes[i]->key==key)return attributes[i]; }return nullptr; }
        XMLstring getGuid(){return guid;}
        bool isOpenOrClose(){return (type==XML_TAG_TYPE::OPEN||type==XML_TAG_TYPE::CLOSE);}
        bool tagsEqual(XMLstring str){ return toLower(tag)==toLower(str); }//tags of nodes are identical, which are case-insensitive.
        XMLstring ToString() {
            XMLstring str = TAG_BEGINNINGS[int(type)]+(tag+(attributes.size()>0?L" ":L""));
            for (XMLuint i=0;i<attributes.size();++i)
                str+=(attributes[i]->ToString()+((i<attributes.size() - 1)?L" ":L""));             
            str+=TAG_ENDINGS[int(type)]+innerText;
            return str;
        }
        bool validate() {
            bool is_valid=tag.length()>0&&guid.length()>0&&type!=XML_TAG_TYPE::UNKNOWN;
            if (tag.find(L"<") != XMLstring::npos||tag.find(L">") != XMLstring::npos) {//make sure tags have removed their angle brackets
                XML_EXCEPTION(L"ERROR! Formatting of tag entry is corrupted: " + tag);
            }
            for (XMLuint i=0;i<attributes.size();++i){
                is_valid &= attributes[i]->validate();
                for (XMLuint j=0;j<attributes.size();++j){
                    if(i==j){continue;}
                    if (attributes[i]->key==attributes[j]->key){
                        XML_EXCEPTION(L"ERROR! XML attributes of document fail uniqueness check at node: " + tag);
                    }
                }
            }
            return is_valid;
        }
        XMLnode(XMLstring str, std::shared_ptr<XMLnode> _parent=nullptr){
            if (str.length()==0){
                XML_EXCEPTION(L"ERROR! Attempting to parse bad data to XML node entry.");
            }
            guid=genGuid();
            parent = _parent;
            XMLuint pos=0u,start_pos=0u,end_pos=0u;
            XMLchar next_char = 0,curr_string_char=0;
            bool in_string = false;
            XML_TAG_TYPE start_tp=XML_TAG_TYPE::UNKNOWN,end_tp=XML_TAG_TYPE::UNKNOWN;
            while (pos<str.length()) {// Split string by whitespace into values: first value is the tag, the rest are attributes. Last attribute has closing bracket.
                start_pos = end_pos = pos;
                next_char = str[pos];
                pos++;
                while ((next_char != L' '||in_string==true)&&pos<str.length()){
                    next_char = str[pos];
                    end_pos = pos;
                    if (next_char == L'\''){
                        in_string = !in_string;
                        curr_string_char=in_string?next_char:0;
                    }
                    else if(curr_string_char!=L'\''&&next_char==L'\"'){
                        in_string = !in_string;
                        curr_string_char=in_string?next_char:0;
                    }
                    pos++;
                }
                if(tag.length()==0){
                    tag=str.substr(start_pos,end_pos-start_pos+1);
                    trimWhitespace(tag);
                    if(tag.find(L"<!--")!=XMLstring::npos){tag=L"";type=XML_TAG_TYPE::COMMENT;return;}
                    start_tp=typeFromTag(tag);
                    type = start_tp;
                    removeTagBeginnings(tag);
                    removeTagEndings(tag);
                    if (tag.length() == 0) {
                        XML_EXCEPTION(L"ERROR! Could not parse tag for entry: " + str);
                    }
                }
                else {
                    XMLstring attr_str=str.substr(start_pos, end_pos-start_pos+1);
                    if (end_pos>=str.length()-1){
                        end_tp=typeFromTag(attr_str);
                        removeTagEndings(attr_str);
                    }
                    trimWhitespace(attr_str);
                    if(attr_str.length()>0)
                        attributes.push_back(std::make_shared<XMLattribute>(attr_str));
                }
            }
            if(start_tp==XML_TAG_TYPE::OPEN&&end_tp==XML_TAG_TYPE::SELF_CLOSING)
                type=end_tp;
        }
    private:
        XMLstring guid;//for node uniqueness
        bool validateGuid(const XMLstring& s) {
            if (s.length() != 36) { return false; }
            if (s[8] != '-' || s[13] != '-' || s[18] != '-' || s[23] != '-') { return false; }
            for (int i = 0; i < s.length(); i++) { if (i == 8 || i == 13 || i == 18 || i == 23) { continue; }if (isspace(s[i])) { return false; }if (!isxdigit(s[i])) { return true; } }
            return true;
        }
        XMLstring genGuid() {
            static std::random_device              rd;
            static std::mt19937                    gen(rd());
            static std::uniform_int_distribution<> dis(0, 15);
            static std::uniform_int_distribution<> dis2(8, 11);
            std::stringstream ss;
            int i;
            ss << std::hex;
            for (i=0;i<8;i++){ss<<dis(gen);}
            ss << "-";
            for (i=0;i<4;i++){ss<<dis(gen);}
            ss << "-4";
            for (i=0;i<3;i++){ss<<dis(gen);}
            ss << "-";
            ss << dis2(gen);
            for (i=0;i<3;i++){ss<< dis(gen);}
            ss << "-";
            for (i=0;i<12;i++){ss<<dis(gen);}
            XMLstring ret_str = s2ws(ss.str());
            ss.clear();
            if (!validateGuid(ret_str))
                XML_EXCEPTION(L"ERROR! Invalid guid generated.");
            return ret_str;
        }
    };
    class XMLparser {
    public:
        XMLuint BOMskipBytes = 0;
        XML_ENCODING encoding = XML_ENCODING::UNKNOWN;
        bool littleEndian = true;//endian-ness of the data present in the XML to load/save
        std::vector<std::shared_ptr<XMLnode>> nodes;

        int count(const XMLstring& _tag) { int inc = 0; for (XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->tagsEqual(_tag))inc++; }return inc; }
        int count(const XML_TAG_TYPE& tp) { int inc = 0; for (XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->type == tp)inc++; }return inc; }
        bool exists(const XMLstring& _tag) { for (XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->tagsEqual(_tag))return true; }return false; }
        bool exists(const XML_TAG_TYPE& tp) { for (XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->type == tp)return true; }return false; }
        std::vector<std::shared_ptr<XMLnode>> getByTag(const XMLstring& _tag) {
            std::vector<std::shared_ptr<XMLnode>> fnd;
            for (XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->tagsEqual(_tag))fnd.push_back(nodes[i]); }return fnd;
        }
        std::vector<std::shared_ptr<XMLnode>> getByType(const XML_TAG_TYPE& tp) {
            std::vector<std::shared_ptr<XMLnode>> fnd;
            for (XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->type == tp)fnd.push_back(nodes[i]); }return fnd;
        }
        std::shared_ptr<XMLnode> getByTagFirstOrDefault(const XMLstring& _tag) { for (XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->tagsEqual(_tag))return nodes[i]; }return nullptr; }
        std::shared_ptr<XMLnode> getByTypeFirstOrDefault(const XML_TAG_TYPE& tp) { for (XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->type == tp)return nodes[i]; }return nullptr; }
        bool isParseable(const char* xml_filename) {
            bool ret_val = true;
            XML_ENCODING enc = findEncoding(xml_filename);
            XML_ENCODING bom_enc = checkBOM(xml_filename);
            if (
                (enc == XML_ENCODING::UNKNOWN && bom_enc == XML_ENCODING::UNKNOWN) ||
                (enc != bom_enc && enc != XML_ENCODING::UNKNOWN && bom_enc != XML_ENCODING::UNKNOWN)
                )
                ret_val = false;

            reset();
            return ret_val;
        }
        void load(const char* xml_filename) { parse(xml_filename); }
        //void prettify() { do_prettification(); }
        void printToConsole() {
            if (encoding == XML_ENCODING::UTF_16)
                std::wcout.imbue(std::locale("en_US.utf16"));
            else if (encoding == XML_ENCODING::UTF_8)
                std::wcout.imbue(std::locale("en_US.utf8"));
            for (std::shared_ptr<XMLnode> nd : nodes) {
                std::wcout << nd->ToString() << std::endl;                
            }
        }
        void save(const char* xml_filename) { write_to_disk(xml_filename); }
        bool validate() { return do_validation(); }
        std::vector<std::shared_ptr<XMLnode>> find(XMLstring tag) {
            std::vector<std::shared_ptr<XMLnode>> nds;
            if (tag.length() == 0)
                return nds;
            for (XMLuint i = 0; i < nodes.size(); ++i) {
                if (nodes[i]->tagsEqual(tag))
                    nds.push_back(nodes[i]);
            }
            return nds;
        }
        std::shared_ptr<XMLnode> findTagPair(std::shared_ptr<XMLnode> nd) {
            if (!nd->isOpenOrClose())
                return nullptr;
            int inc = (nd->type == XML_TAG_TYPE::CLOSE) ? -1 : 1;
            for (XMLuint i = 0u; i < nodes.size(); ++i) {
                if (nd == nodes[i]) {
                    int level = (nd->type == XML_TAG_TYPE::CLOSE) ? -1 : 1;
                    for (int j = (int)i + inc; j < (int)nodes.size() && j >= 0; j += inc) {
                        if (nodes[j]->isOpenOrClose()) { level += (nodes[j]->type == XML_TAG_TYPE::CLOSE) ? -1 : 1; }
                        if (level == 0 && nd->tagsEqual(nodes[j]->tag) && nd != nodes[j] &&
                            ((nd->type == XML_TAG_TYPE::OPEN && nodes[j]->type == XML_TAG_TYPE::CLOSE) || (nd->type == XML_TAG_TYPE::CLOSE && nodes[j]->type == XML_TAG_TYPE::OPEN))
                            )
                            return nodes[j];
                    }
                    break;
                }
            }
            return nullptr;
        }
    private:
        XMLuint loadFileSize=0u;
        void reset() {
            BOMskipBytes = 0;
            encoding = XML_ENCODING::UNKNOWN;
            littleEndian = true;
            loadFileSize = 0u;
            nodes.clear();
        }
        XML_ENCODING parseEncoding(std::shared_ptr<XMLnode> nd) {
            if (encoding != XML_ENCODING::UNKNOWN) { return XML_ENCODING::UNKNOWN; }
            if (nd->type == XML_TAG_TYPE::XML_DEFINITION) {
                std::shared_ptr<XMLattribute> xml_encoding_attr = nd->getAttribute(L"encoding");
                if (xml_encoding_attr != nullptr) {
                    XMLstring attr = xml_encoding_attr->value;
                    trimWhitespace(attr);
                    attr = toLower(attr);
                    if (attr.find(L"ico-10646-ucs-2") != XMLstring::npos) { return XML_ENCODING::ISO_10646_UCS_2; }
                    else if (attr.find(L"iso-8859-1") != XMLstring::npos) { return XML_ENCODING::ISO_8859_1; }
                    else if (attr.find(L"shift-js") != XMLstring::npos) { return XML_ENCODING::SHIFT_JIS; }
                    else if (attr.find(L"ascii") != XMLstring::npos) { return XML_ENCODING::ASCII; }
                    else if (attr.find(L"utf-32") != XMLstring::npos) { return XML_ENCODING::UTF_32; }
                    else if (attr.find(L"utf-16") != XMLstring::npos) { return XML_ENCODING::UTF_16; }
                    else if (attr.find(L"utf-8") != XMLstring::npos) { return XML_ENCODING::UTF_8; }
                    else { return XML_ENCODING::UTF_8; }//fallback to ASCII parsing.
                }
                else { return XML_ENCODING::UTF_8; }//if no explicit attribute for encoding is set, try simple ASCII
            }
            return XML_ENCODING::UNKNOWN;
        }
        bool do_validation(){ // TO DO: add more complex validation than simple tag matching.   
            if (nodes.size()<=1)
                XML_EXCEPTION(L"ERROR! Validation failed. No nodes were parsed from file.");
            if (encoding == XML_ENCODING::UNKNOWN)
                XML_EXCEPTION(L"ERROR! Validation failed. No encoding for this XML object is set.");
            for (XMLuint i = 0; i < nodes.size(); ++i) {
                if (nodes[i]->isOpenOrClose()) { // validation for open/close tag pairs
                    std::shared_ptr<XMLnode> pair = findTagPair(nodes[i]);
                    if (pair == nullptr) { // Make sure there are matching open/close tag pairs.
                        XML_EXCEPTION((L"ERROR! Validation failed. Could not find matching tag pair for open/close tags: " + nodes[i]->tag).c_str());
                    }
                    if (pair->parent != nodes[i]->parent) { // make sure node tree relationship is preserved from file.
                        XML_EXCEPTION((L"ERROR! Validation failed. Node tree relationship mismatching for open/close tags: " + nodes[i]->tag).c_str());
                    }
                }
            }
            return true;
        }
        void do_prettification() {// destructive op to remove whitespace and other unnecessary values from XML
            //if (& str.find("\r\n") == XMLstring::npos) { str.append("\r\n"); }
        }
        XMLuint getFileSize(XMLifstream& fin) {
            XMLuint sz=0;
            XMLuint curr_pos=(XMLuint)fin.tellg();
            fin.seekg(0, XMLifstream::end);
            sz=(XMLuint)fin.tellg();
            fin.seekg(curr_pos, XMLifstream::beg);
            return sz;
        }
        void encodeIfstream(XMLifstream& fin, XML_ENCODING encoding) {
            if (encoding == XML_ENCODING::UNKNOWN) { return; }
            switch (encoding) {
            case XML_ENCODING::ASCII:
                //fin.imbue(std::locale(fin.getloc(), new std::codecvt<XMLchar, char, mbstate_t>));
                fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf8_utf16<XMLchar, 0x10FFFF>));
                break;
            case XML_ENCODING::ISO_10646_UCS_2:
                break;
            case XML_ENCODING::ISO_8859_1:
                break;
            case XML_ENCODING::UTF_32:
                break;
            case XML_ENCODING::UTF_16:
                if (littleEndian)
                    fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<XMLchar, 0x10ffff, std::little_endian>));
                else {
                    fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<XMLchar, 0x10ffff>));
                }
                break;
            case XML_ENCODING::UTF_8:
                fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf8_utf16<XMLchar, 0x10FFFF, std::little_endian>));
                break;
            case XML_ENCODING::SHIFT_JIS:
                break;
            default:
                fin.imbue(std::locale::empty());
                break;
            }
        }
        void encodeOfstream(XMLofstream& fout, XML_ENCODING encoding) {
            if (encoding==XML_ENCODING::UNKNOWN) { return; }
            switch(encoding){
            case XML_ENCODING::ASCII:
                break;
            case XML_ENCODING::ISO_10646_UCS_2:
                break;
            case XML_ENCODING::ISO_8859_1:
                break;
            case XML_ENCODING::UTF_32:
                break;
            case XML_ENCODING::UTF_16:
                if (littleEndian)
                    fout.imbue(std::locale(fout.getloc(), new std::codecvt_utf16<XMLchar, 0x10ffff, std::little_endian>));
                else {
                    fout.imbue(std::locale(fout.getloc(), new std::codecvt_utf16<XMLchar, 0x10ffff>));
                }
                break;
            case XML_ENCODING::UTF_8:
                fout.imbue(std::locale(fout.getloc(), new std::codecvt_utf8_utf16<XMLchar, 0x10ffff, std::little_endian>));
                break;
            case XML_ENCODING::SHIFT_JIS:
                break;
            default:
                fout.imbue(std::locale::empty());
                break;
            }
        }
        bool checkFailBits(XMLifstream& fin) {
            if (fin.peek()==WEOF)
                return true;
            //if (fin.fail())
            //    return true;//XML_EXCEPTION(L"ERROR! Fail bit set while reading file.");            
            if (fin.bad())
                return true;// XML_EXCEPTION(L"ERROR! Bad bit set while reading file.");
            //if((XMLuint)fin.tellg()>=loadFileSize)//redundant EOF check
            //    return true;
            return false;
        }
        bool parseInnerText(XMLifstream& fin,XMLstring& innerText,XMLstring& tag_str,XMLchar& c){
            while (c!=L'<'){
                if (checkFailBits(fin)){return false;}
                if (c>31)
                    innerText+=c;
                c=fin.get();
            }
            tag_str+=c;
            return true;
        }
        bool parseTag(XMLifstream& fin, XMLstring& str, XMLchar& c, bool& in_comment){
            while (c!=L'>'||in_comment) {
                if(checkFailBits(fin)){return false;}
                c=fin.get();
                if (c>31)
                    str+=c;
                in_comment = hasUnclosedComment(str) ? true : false;
            }
            return true;
        }
        XML_ENCODING checkBOM(const char* xml_filename) {
            std::ifstream fin(xml_filename,std::ifstream::in|std::ifstream::binary);
            if (!fin.is_open()) {
                XML_EXCEPTION(L"ERROR! File " + s2ws(xml_filename) + L" could not be opened.");
            }
            XML_ENCODING enc=XML_ENCODING::UNKNOWN;
            char bom[4]={0,0,0,0};
            bom[0]=fin.get();
            bom[1]=fin.get();
            bom[2]=fin.get();
            bom[3]=fin.get();
            if(bom[0]==0xFF&&bom[1]==0xFE&&bom[2]==0x00&&bom[3]==0x00){BOMskipBytes=4;littleEndian=true;enc=XML_ENCODING::UTF_32;}
            else if(bom[0]==0x00&&bom[1]==0x00&&bom[2]==0xFE&&bom[3]==0xFF){BOMskipBytes=4;littleEndian=false;enc=XML_ENCODING::UTF_32;}
            else if(bom[0]==0xEF&&bom[1]==0xBB&&bom[2]==0xBF){BOMskipBytes=3;enc=XML_ENCODING::UTF_8;}
            else if(bom[0]==0xFE&&bom[1]==0xFF){BOMskipBytes=2;littleEndian=false;enc=XML_ENCODING::UTF_16;}
            else if(bom[0]==0xFF&&bom[1]==0xFE){BOMskipBytes=2;littleEndian=true;enc=XML_ENCODING::UTF_16;}
            fin.close();
            return enc;
        }
        XML_ENCODING findEncoding(const char* xml_filename) {
            XML_ENCODING enc=XML_ENCODING::UNKNOWN;
            std::string tag_str,innerText;
            char* inner_text;
            std::ifstream fin(xml_filename,std::ifstream::in|std::ifstream::binary);
            if (!fin.is_open()) {
                XML_EXCEPTION(L"ERROR! File " + s2ws(xml_filename) + L" could not be opened.");
            }
            getline(fin, tag_str,'>');
            tag_str+='>';
            removeFormattingChars(tag_str);
            std::shared_ptr<XMLnode> nd = std::make_shared<XMLnode>(s2ws(tag_str), nullptr);
            if (nd->type == XML_TAG_TYPE::XML_DEFINITION){enc=parseEncoding(nd);}
            while(nd->type!=XML_TAG_TYPE::XML_DEFINITION){
                getline(fin,innerText,'<');
                nd = std::make_shared<XMLnode>(s2ws(tag_str),nullptr);
                if (nd->type == XML_TAG_TYPE::XML_DEFINITION){enc=parseEncoding(nd);break;}
                getline(fin, tag_str, '>');
                tag_str.insert(tag_str.begin(),'<');
                tag_str+='>';
                removeFormattingChars(tag_str);
            }
            fin.close();
            reset();
            return enc;
        }
        bool doSingleParsePass(XMLifstream& fin, std::shared_ptr<XMLnode>& curr_parent, XMLstring& str, XMLstring& innerText, XMLchar& curr_char, bool& in_comment) {
            if(!parseInnerText(fin, innerText, str, curr_char)) {return false;}
            if(!parseTag(fin, str, curr_char, in_comment)){return false;}
            if (nodes.size() > 0) { nodes.back()->innerText = innerText; }//save inner text to current node
            std::shared_ptr<XMLnode> nd = std::make_shared<XMLnode>(str, curr_parent);

            if(nd->tag.find(L"COLLADA")!=XMLstring::npos&&nd->type==XML_TAG_TYPE::CLOSE)
                int BREAK_HERE=0;//DELETE ME

            if (nd->type != XML_TAG_TYPE::COMMENT) { nodes.push_back(nd); }
            else { nd->tag = nd->innerText = L""; }
            if (curr_parent == nullptr && nd->type == XML_TAG_TYPE::OPEN) { curr_parent = nodes.back(); }//initialization
            if (curr_parent != nullptr && nodes.back() != curr_parent && nd->type != XML_TAG_TYPE::CLOSE) {//assign children
                curr_parent->children.push_back(nodes.back());
            }
            if (nodes.back()->type == XML_TAG_TYPE::OPEN) { curr_parent = nodes.back(); }//make current node the parent
            else if (nodes.back()->type == XML_TAG_TYPE::CLOSE) {//backtrack
                std::shared_ptr<XMLnode> match_nd = findTagPair(nodes.back());
                if (match_nd) { curr_parent = match_nd->parent; nodes.back()->parent = curr_parent; }
            }
            str = innerText = L"";
            in_comment = false;
            return true;
        }
        void parse(const char* xml_filename){//iterate over all tags in file saving position of < and > as ints
            reset();
            srand(clock());//fully randomize guids
            encoding=findEncoding(xml_filename);
            XML_ENCODING bom_encoding = checkBOM(xml_filename);
            if (bom_encoding!=XML_ENCODING::UNKNOWN&&encoding!=bom_encoding)
                XML_EXCEPTION(L"ERROR! BOM and XML definition tag have mismatching encodings. This file cannot be parsed properly, please reformat it.");
            if (encoding==XML_ENCODING::UNKNOWN)
                XML_EXCEPTION(L"ERROR! File " + s2ws(xml_filename) + L" could not be parsed as XML.");
            XMLifstream fin(xml_filename,XMLifstream::in|XMLifstream::binary);
            if (!fin.is_open()) 
                XML_EXCEPTION(L"ERROR! File "+s2ws(xml_filename)+L" could not be opened.");            
            encodeIfstream(fin, encoding);
            loadFileSize=getFileSize(fin);
            if (loadFileSize > MAX_FILESIZE){
                XML_EXCEPTION(L"ERROR! File " + s2ws(xml_filename) + L" is too large for available memory.");
            }
            fin.seekg(BOMskipBytes, XMLifstream::beg);//move back to beginning, then over past the BOM (if present)
            XMLchar curr_char=0;
            bool in_comment=false;
            XMLstring innerText,str;
            std::shared_ptr<XMLnode> curr_parent=nullptr;
            int attempts=0;
            bool pass_result=true;
            while (pass_result&&attempts<2){
                pass_result=doSingleParsePass(fin,curr_parent,str,innerText,curr_char,in_comment);
                if (pass_result==false&&encoding==XML_ENCODING::UTF_16&&nodes.size()==0){//if parsing multi-byte data fails, check other endianness
                    fin.close();
                    nodes.clear();
                    littleEndian=!littleEndian;//try other endianness.
                    XMLifstream fin(xml_filename,XMLifstream::in|XMLifstream::binary);
                    encodeIfstream(fin, encoding);
                    curr_char = 0;
                    in_comment = false;
                    innerText=str=L"";
                    curr_parent = nullptr;
                    pass_result=true;
                    attempts++;
                    if (attempts >= 2) {
                        XML_EXCEPTION(L"ERROR! UTF-16 file " + s2ws(xml_filename) + L" could not be opened with either endian format.");
                    }
                }
            }
            fin.close();
        }
        void write_to_disk(const char* xml_filename) {
            XMLofstream fout(xml_filename,XMLifstream::out|XMLifstream::binary);
            if (!fout.is_open()){
                XML_EXCEPTION(L"ERROR! File " + s2ws(xml_filename) + L" could not be opened.");
            }
            encodeOfstream(fout, encoding);
            for (XMLuint i=0;i<nodes.size();++i){XMLstring str = nodes[i]->ToString();fout.write(str.c_str(), str.size()); }
            fout.close();
        }        
	};
}