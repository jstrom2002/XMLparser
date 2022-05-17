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
    // DEFINITIONS:
#ifdef _WIN64
    typedef uint64_t XMLuint;
#else
    typedef uint32_t XMLuint;
#endif
    typedef std::wifstream XMLifstream;
    typedef std::wofstream XMLofstream;
    typedef wchar_t XMLchar;
    typedef std::wstring XMLstring;
    #define MAX_XML_BOM_SIZE 4
    XMLuint MAX_FILESIZE=9999999999999u;
    enum class XML_TAG_TYPE{UNKNOWN,CDATA,COMMENT,CLOSE,OPEN,SELF_CLOSING,UNIQUE,XML_DEFINITION,COUNT};// replacement for enum which is not available in c++98, open=='<...>', close=='</...>', self-closing=='<.../>',comment=='<!-- ... -->',xml=='<?...?>' 0          
    const XMLstring TAG_BEGINNINGS[int(XML_TAG_TYPE::COUNT)] = { L"",L"<![CDATA[",L"<!--",L"</",L"<",L"<",L"<!",L"<?"};
    const XMLstring TAG_ENDINGS[int(XML_TAG_TYPE::COUNT)] = { L"",L"]]>",L"-->",L">",L">",L"/>",L">",L"?>" };
    enum class XML_ENCODING{UNKNOWN,UTF_1,UTF_7,UTF_8,UTF_16LE,UTF_16BE,UTF_32LE,UTF_32BE,UTF_EBCDIC,ISO_8859_1,ISO_10646_UCS_2,SHIFT_JIS,ASCII,BOCU_1,SCSU,GB_18030,COUNT};
    XMLstring toLower(XMLstring str){for(size_t i=0;i<str.length();++i){str[i]=std::tolower(str[i]);}return str;}
    void trimWhitespace(XMLstring& str){while(str.length()>0&&str[0]==L' '){str=str.substr(1);}while(str.length()>0&&str[str.length()-1]==L' '){str=str.substr(0,str.length()-1);}}
    void removeFormattingChars(XMLstring& str){for(XMLuint i=0;i<str.length();++i)if(str[i]<L' '){str.erase(str.begin()+i);i--;}}
    void removeFormattingChars(std::string& str){for(XMLuint i=0;i<str.length();++i)if(str[i]<' '){str.erase(str.begin()+i);i--;}}
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
    XMLuint getFileSize(XMLifstream& fin) {
        XMLuint sz = 0;
        XMLuint curr_pos = (XMLuint)fin.tellg();
        fin.seekg(0, XMLifstream::end);
        sz = (XMLuint)fin.tellg();
        fin.seekg(curr_pos, XMLifstream::beg);
        return sz;
    }
    void remove(XMLstring& str,const XMLstring& to_remove){if(str.length()>0&&str.find(to_remove)!=XMLstring::npos)
        {str=str.substr(to_remove.length(),str.length()-to_remove.length());}}
    void removeTagBeginnings(XMLstring& str){
        const XMLstring to_remove[]={L"<!--",L"</",L"<!",L"<?",L"<"};
        for(XMLuint i=0;i<5;++i){remove(str,to_remove[i]);}
    }
    void removeTagEndings(XMLstring& str) {
        const XMLstring to_remove[]={L"-->",L"/>",L"?>",L">"};
        for(XMLuint i=0;i<4;++i){
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
    bool hasUnclosedComment(const XMLstring& str){
        bool isComment = str.length()>4&&str.find(L"<!--")!=XMLstring::npos&&str.rfind(L"-->")==XMLstring::npos;
        bool isCDATAcomment = str.length()>9&&str.find(L"<![CDATA[")!=XMLstring::npos&&str.rfind(L"]]>")==XMLstring::npos;
        return isComment || isCDATAcomment;
    }     
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
                    if(tag.find(TAG_BEGINNINGS[XMLuint(XML_TAG_TYPE::COMMENT)])!=XMLstring::npos){tag=L"";type=XML_TAG_TYPE::COMMENT;return;}
                    if(tag.find(TAG_BEGINNINGS[XMLuint(XML_TAG_TYPE::CDATA)])!=XMLstring::npos){
                        tag=L"CDATA";
                        type = XML_TAG_TYPE::CDATA;
                        XMLstring in_txt=str;
                        remove(in_txt, TAG_BEGINNINGS[XMLuint(XML_TAG_TYPE::CDATA)]);
                        remove(in_txt, TAG_ENDINGS[XMLuint(XML_TAG_TYPE::CDATA)]);
                        innerText=in_txt;
                        return;
                    }
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
            if(s.length()!=36){return false;}
            if(s[8]!='-'||s[13]!='-'||s[18]!='-'||s[23]!='-'){return false;}
            for(int i=0;i<s.length();i++){if(i==8||i==13||i==18||i==23){continue;}if(isspace(s[i])){return false;}if(!isxdigit(s[i])){return true;}}
            return true;
        }
        int randInt(int _min_val,int _max_val){
            int modulo = _max_val-_min_val+1;
            if (modulo < 0)
                modulo = 0;
            unsigned int val_ = rand() % (unsigned int)modulo;
            return val_ + _min_val;
        }
        XMLstring genGuid() {
            std::stringstream ss;
            int i;
            ss<<std::hex;
            for(i=0;i<8;i++){ss<<randInt(0,15);}
            ss<<"-";
            for(i=0;i<4;i++){ss<<randInt(0,15);}
            ss<<"-4";
            for(i=0;i<3;i++){ss<<randInt(0,15);}
            ss<<"-";
            ss<<randInt(8,11);
            for(i=0;i<3;i++){ss<<randInt(0,15);}
            ss<<"-";
            for(i=0;i<12;i++){ss<<randInt(0,15);}
            XMLstring ret_str = s2ws(ss.str());
            if (!validateGuid(ret_str))
                XML_EXCEPTION(L"ERROR! Invalid guid generated.");
            return ret_str;
        }
    };
    class XMLencodingManager {
    public:
        XMLencodingManager(){} //use 'parse()' or 'set()' methods to set values instead of construtor.
        XMLencodingManager(const char* xml_filename){
            parse(xml_filename);
        }
        bool empty(){return _encoding==XML_ENCODING::UNKNOWN;}
        void encode(XMLifstream& fin){
            if (empty()){return;}
            switch(type()){
            case XML_ENCODING::UTF_16LE:
                fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<XMLchar, 0x10ffff, std::little_endian>));
                break;
            case XML_ENCODING::UTF_16BE:
                fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf16<XMLchar, 0x10ffff>));
                break;
            case XML_ENCODING::UTF_8:
                fin.imbue(std::locale(fin.getloc(), new std::codecvt_utf8_utf16<XMLchar, 0x10FFFF, std::little_endian>));
                break;
            default:
                fin.imbue(std::locale::empty());
                break;
            }
        }
        void encode(XMLofstream& fout) {
            if (empty()) { return; }
            switch (type()) {
            case XML_ENCODING::UTF_16LE:
                fout.imbue(std::locale(fout.getloc(), new std::codecvt_utf16<XMLchar, 0x10ffff, std::little_endian>));
                break;
            case XML_ENCODING::UTF_16BE:
                fout.imbue(std::locale(fout.getloc(), new std::codecvt_utf16<XMLchar, 0x10ffff>));
                break;
            case XML_ENCODING::UTF_8:
                fout.imbue(std::locale(fout.getloc(), new std::codecvt_utf8_utf16<XMLchar, 0x10ffff, std::little_endian>));
                break;
            default:
                fout.imbue(std::locale::empty());
                break;
            }
        }
        void parse(const char* xml_filename) {
            bom.parse(xml_filename);
            if (bom.empty())
                _encoding = findEncoding(xml_filename);
            if(_encoding==XML_ENCODING::UNKNOWN) // All else fails, assume simplest ASCII encoding
                _encoding=XML_ENCODING::ASCII;
        }
        XML_ENCODING parseEncodingFromString(const XMLstring& str) {
            if (str.find(L"ico-10646-ucs-2") != XMLstring::npos) { return XML_ENCODING::ISO_10646_UCS_2; }
            else if (str.find(L"iso-8859-1") != XMLstring::npos) { return XML_ENCODING::ISO_8859_1; }
            else if (str.find(L"shift-js") != XMLstring::npos) { return XML_ENCODING::SHIFT_JIS; }
            else if (str.find(L"ascii") != XMLstring::npos) { return XML_ENCODING::ASCII; }
            else if (str.find(L"utf-32be") != XMLstring::npos) { return XML_ENCODING::UTF_32BE; }
            else if (str.find(L"utf-32") != XMLstring::npos) { return XML_ENCODING::UTF_32LE; }
            else if (str.find(L"utf-16be") != XMLstring::npos) { return XML_ENCODING::UTF_16BE; }
            else if (str.find(L"utf-16") != XMLstring::npos) { return XML_ENCODING::UTF_16LE; }
            else if (str.find(L"utf-8") != XMLstring::npos) { return XML_ENCODING::UTF_8; }
            else if (str.find(L"unicode") != XMLstring::npos) { return XML_ENCODING::UTF_16LE; }
            else { return XML_ENCODING::UTF_8; }//fallback to ASCII parsing.
        }
        bool hasMultipleEndianness() {
            switch (_encoding) {
            case XML_ENCODING::UTF_16LE:
            case XML_ENCODING::UTF_16BE:
            case XML_ENCODING::UTF_32LE:
            case XML_ENCODING::UTF_32BE:
                return true;
            }
            return false;
        }
        bool isUTF() {
            switch (type()) {
            case XML_ENCODING::UTF_1:
            case XML_ENCODING::UTF_7:
            case XML_ENCODING::UTF_8:
            case XML_ENCODING::UTF_EBCDIC:
            case XML_ENCODING::UTF_32LE:
            case XML_ENCODING::UTF_32BE:
            case XML_ENCODING::UTF_16LE:
            case XML_ENCODING::UTF_16BE:
                return true;
            }
            return false;
        }
        void set(XML_ENCODING enc) {
            _encoding = enc;
            if (!bom.empty())
                bom.set(enc);
        }
        void swapEndianness() {
            switch (_encoding) {
            case XML_ENCODING::UTF_16LE:
                _encoding = XML_ENCODING::UTF_16BE;
            case XML_ENCODING::UTF_16BE:
                _encoding = XML_ENCODING::UTF_16LE;
            case XML_ENCODING::UTF_32LE:
                _encoding = XML_ENCODING::UTF_32BE;
            case XML_ENCODING::UTF_32BE:
                _encoding = XML_ENCODING::UTF_32LE;
            }
        }
        void skipBOM(XMLifstream& fin) { bom.skip(fin); }
        XML_ENCODING type() { return bom.empty() ? _encoding : bom.encoding(); }
        bool validation(){return !empty();}
        class XML_BOM {
        public:
            std::vector<unsigned char> bytes() {
                std::vector<unsigned char> _b;
                BOMbytesByEncoding(_b, _encoding);
                return _b;
            }
            bool empty(){return (_encoding==XML_ENCODING::UNKNOWN);}
            XML_ENCODING encoding() { return _encoding; }
            void parse(const char* xml_filename) {
                std::ifstream fin(xml_filename, std::ifstream::in | std::ifstream::binary);
                if (!fin.is_open()) {
                    XML_EXCEPTION(L"ERROR! File " + s2ws(xml_filename) + L" could not be opened.");
                }
                std::vector<unsigned char> _bytes;
                for (int i = 0;fin.peek()!=EOF&&i<MAX_XML_BOM_SIZE;++i){_bytes.push_back(fin.get());}
                parseEncoding(_bytes);
                fin.close();
            }
            void set(XML_ENCODING _encoding) {
                _encoding = _encoding;

            }
            int size() {
                switch (_encoding) {
                case XML_ENCODING::UTF_1:
                case XML_ENCODING::UTF_7:
                case XML_ENCODING::UTF_8:
                case XML_ENCODING::UTF_EBCDIC:
                case XML_ENCODING::SCSU:
                case XML_ENCODING::BOCU_1:
                    return 3;
                case XML_ENCODING::UTF_32LE:
                case XML_ENCODING::UTF_32BE:
                case XML_ENCODING::GB_18030:
                    return 4;
                case XML_ENCODING::UTF_16LE:
                case XML_ENCODING::UTF_16BE:
                    return 2;
                }
                return 0;
            }
            void skip(XMLifstream& fin) { fin.seekg(size(), XMLifstream::beg); }
            void write(XMLofstream& fout) {
                if (!empty()) {
                    std::vector<unsigned char> _bytes = bytes();
                    for (XMLuint i = 0; i < _bytes.size(); ++i) { fout << _bytes[i]; }
                }
            }
        private:
            XML_ENCODING _encoding = XML_ENCODING::UNKNOWN;
            XML_ENCODING parseEncoding(std::vector<unsigned char>& _b) {
                for (XMLuint i=0; i < (XMLuint)XML_ENCODING::COUNT; ++i) {
                    std::vector<unsigned char> _enc_b;
                    XML_ENCODING this_enc = (XML_ENCODING)i;
                    BOMbytesByEncoding(_enc_b, this_enc);
                    if (_enc_b.size() > 0 && _b == _enc_b)
                        return this_enc;
                }
                return XML_ENCODING::UNKNOWN;
            }
            void BOMbytesByEncoding(std::vector<unsigned char>& _b, XML_ENCODING _enc) {//see:https://en.wikipedia.org/wiki/Byte_order_mark#Byte_order_marks_by_encoding
                _b.clear();
                switch (_enc) {
                case XML_ENCODING::BOCU_1:_b = { 251,238,40 }; break;
                case XML_ENCODING::GB_18030:_b = { 132,49,149,51 }; break;
                case XML_ENCODING::SCSU:_b = { 14,254,255 }; break;
                case XML_ENCODING::UTF_EBCDIC:_b = { 221,115,102,115 }; break;
                case XML_ENCODING::UTF_1:_b = { 247,100,76 }; break;
                case XML_ENCODING::UTF_7:_b = { 43,47,118 }; break;
                case XML_ENCODING::UTF_8:_b = { 239,187,191 }; break;
                case XML_ENCODING::UTF_16LE:_b = { 255,254 }; break;
                case XML_ENCODING::UTF_16BE:_b = { 254,255 }; break;
                case XML_ENCODING::UTF_32LE:_b = { 255,254,0,0 }; break;
                case XML_ENCODING::UTF_32BE:_b = { 0,0,254,255 }; break;
                }
            }
        }bom;
    protected:
        XML_ENCODING _encoding = XML_ENCODING::UNKNOWN;//parsed from BOM first, then (if failed) xml tag, ie '<?xml encoding=...?>'
        XML_ENCODING findEncoding(const char* xml_filename) {
            XML_ENCODING enc = XML_ENCODING::UNKNOWN;
            std::string tag_str, innerText;
            char* inner_text;
            std::ifstream fin(xml_filename, std::ifstream::in | std::ifstream::binary);
            if (!fin.is_open()) {
                XML_EXCEPTION(L"ERROR! File " + s2ws(xml_filename) + L" could not be opened.");
            }
            getline(fin, tag_str, '>');
            tag_str += '>';
            removeFormattingChars(tag_str);
            std::shared_ptr<XMLnode> nd = std::make_shared<XMLnode>(s2ws(tag_str), nullptr);
            if (nd->type == XML_TAG_TYPE::XML_DEFINITION) {
                std::shared_ptr<XMLattribute> xml_encoding_attr=nd->getAttribute(L"encoding");
                if (xml_encoding_attr != nullptr) {
                    XMLstring attr = xml_encoding_attr->value;
                    trimWhitespace(attr);
                    attr = toLower(attr);
                    enc = parseEncodingFromString(attr);
                }
            }
            fin.close();
            return enc;
        }
    };
    class XMLparser {
    public:
        XMLencodingManager encoding;
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
        std::shared_ptr<XMLnode> getByTagFirstOrDefault(const XMLstring& _tag){for (XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->tagsEqual(_tag))return nodes[i]; }return nullptr; }
        std::shared_ptr<XMLnode> getByTypeFirstOrDefault(const XML_TAG_TYPE& tp){for(XMLuint i = 0; i < nodes.size(); ++i) { if (nodes[i]->type == tp)return nodes[i]; }return nullptr; }
        int getIndexByTagFirstOrDefault(const XMLstring& _tag){for(XMLuint i=0;i<nodes.size();++i){if(nodes[i]->tagsEqual(_tag))return i;}return -1;}
        int getIndexByTypeFirstOrDefault(const XML_TAG_TYPE& tp){for(XMLuint i=0;i<nodes.size();++i){if(nodes[i]->type==tp)return i;}return -1;}
        bool hasEncodingDefinition(const char* xml_filename) {
            XMLencodingManager enc;
            enc.parse(xml_filename);
            return !enc.empty();
        }
        bool hasBOM(const char* xml_filename) {
            XMLencodingManager enc;
            enc.parse(xml_filename);
            return !enc.empty();
        }
        void load(const char* xml_filename) { parse(xml_filename); }
        //void prettify() { do_prettification(); }
        void printToConsole() {
            std::wcout.imbue(std::locale("en_US.utf8"));
            for(std::shared_ptr<XMLnode> nd : nodes)
                std::wcout << nd->ToString() << std::endl;            
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
            littleEndian = true;
            loadFileSize = 0u;
            nodes.clear();
        }
        bool do_validation(){ // TO DO: add more complex validation than simple tag matching.   
            if (nodes.size()<=1)
                XML_EXCEPTION(L"ERROR! Validation failed. No nodes were parsed from file.");
            if(!encoding.validation())
                XML_EXCEPTION(L"ERROR! Encoding validation failed. No encoding for this XML object is set.");
            for(XMLuint i=0;i<nodes.size();++i){
                if(nodes[i]->isOpenOrClose()){ // validation for open/close tag pairs
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
        XML_ENCODING parseXMLnode(std::shared_ptr<XMLnode> nd) {
            if (!encoding.empty()) //default to using BOM value over xml tag attribute.
                return XML_ENCODING::UNKNOWN;
            if (nd->type == XML_TAG_TYPE::XML_DEFINITION) {
                std::shared_ptr<XMLattribute> xml_encoding_attr = nd->getAttribute(L"encoding");
                if (xml_encoding_attr != nullptr) {
                    XMLstring attr=xml_encoding_attr->value;
                    trimWhitespace(attr);
                    attr=toLower(attr);
                    encoding.parseEncodingFromString(attr);
                }
            }
            return XML_ENCODING::UNKNOWN;
        }
        bool checkFailBits(XMLifstream& fin) {
            if (fin.peek()==WEOF)
                return true;           
            if (fin.bad())
                return true;
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
            curr_char = 0;
            return true;
        }
        void parse(const char* xml_filename, XML_ENCODING _encoding = XML_ENCODING::UNKNOWN){
            reset();
            srand(clock());//fully randomize guids
            if (_encoding != XML_ENCODING::UNKNOWN)
                encoding.set(_encoding);
            else
                encoding.parse(xml_filename);
            XMLifstream fin(xml_filename,XMLifstream::in|XMLifstream::binary);
            if (!fin.is_open()) 
                XML_EXCEPTION(L"ERROR! File "+s2ws(xml_filename)+L" could not be opened.");            
            encoding.encode(fin);
            loadFileSize=getFileSize(fin);
            if (loadFileSize > MAX_FILESIZE){
                XML_EXCEPTION(L"ERROR! File " + s2ws(xml_filename) + L" is too large for available memory.");
            }
            encoding.bom.skip(fin);
            XMLchar curr_char=0;
            bool in_comment=false;
            XMLstring innerText,str;
            std::shared_ptr<XMLnode> curr_parent=nullptr;
            int attempts=0;
            bool pass_result=true;
            while (pass_result&&attempts<2){
                pass_result=doSingleParsePass(fin,curr_parent,str,innerText,curr_char,in_comment);
                if (pass_result==false&&encoding.hasMultipleEndianness()&&nodes.size()==0){//if parsing multi-byte data fails, check other endianness
                    fin.close();
                    nodes.clear();
                    littleEndian=!littleEndian;//try other endianness.
                    XMLifstream fin(xml_filename,XMLifstream::in|XMLifstream::binary);
                    encoding.swapEndianness();
                    encoding.encode(fin);
                    curr_char = 0;
                    in_comment = false;
                    innerText=str=L"";
                    curr_parent = nullptr;
                    pass_result=true;
                    attempts++;
                    if (attempts >= 2) {
                        XML_EXCEPTION(L"ERROR! UTF XML file "+s2ws(xml_filename)+L" could not be opened with either endian-ness.");
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
            encoding.encode(fout);
            encoding.bom.write(fout);
            for (XMLuint i=0;i<nodes.size();++i){XMLstring str = nodes[i]->ToString();fout.write(str.c_str(), str.size()); }
            fout.close();
        }        
	};
}