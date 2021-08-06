#pragma once
#include <string>
#include <vector>
#include <fstream>
#include <memory>
#include <utility>
#include <set>
#include <regex>
#include <unordered_map>


namespace XMLparser
{   
    // Helper functions for string parsing:
    std::string trimLeadingAndTrailingSpaces(std::string str){
        if (str.length() == 0)
            return str;

        while (str[0] == ' ')
            str = str.substr(1);
        while (str[str.length() - 1] == ' ')
            str = str.substr(0, str.length() - 1);
        return str;
    }

    std::string fileReadToString(std::string filename, unsigned int begin, unsigned int end){
        if (end <= begin)
            return "";
        std::string str = "";
        str.resize(end - begin);
        unsigned int charCounter = 0;
        std::ifstream filePtr = std::ifstream(filename.c_str(), std::ios::in | std::ios::binary);
        filePtr.seekg(begin);
        while (charCounter < end - begin) {
            str[charCounter] = filePtr.get();
            charCounter++;
        }
        filePtr.close();
        return str;
    }

    std::string getNextTag(const std::string& str){
        if (str.length() == 0)
            return "";

        size_t strt = str.find("<");
        size_t endt = str.find(">");
        if (strt != std::string::npos && endt != std::string::npos && strt < endt)
        {
            std::string out_str = str.substr(str.find("<"), str.find(">") + 1);
            return out_str;
        }
        else
            return "";
    }

    std::vector<std::string> tokenize(const std::string& str, const std::string& token){
        std::vector<std::string> ret;
        if (str.length() == 0)
            return ret;

        char* tokenizedStr;
        tokenizedStr = strtok((char*)str.c_str(), token.c_str());
        while (tokenizedStr != NULL){
            ret.push_back(tokenizedStr);
            tokenizedStr = strtok(NULL, token.c_str());
        }
        return ret;
    }

    std::vector<std::string> getTagContents(std::string str){
        std::vector<std::string> contents;
        if (str.length() == 0)
            return contents;

        // Format string to get only next tag.
        str = getNextTag(str);
        if (str == "")
            return contents;

        // Remove all 
        if (str[0] == '<')
            str = str.substr(1);
        if (str[str.length() - 1] == '>')
            str = str.substr(0, str.length() - 1);
        str = trimLeadingAndTrailingSpaces(str);

        // Get tag data. This will also remove extraneous whitespace.
        contents = tokenize(str, " ");

        // Iterate through tokenized strings, search for strings which are not separated by spaces but which contain attributes
        // and break them up into separate entries. Example: 'attribute=1.0'.
        for (int i = 0; i < contents.size(); ++i)
        {
            // Skip entries which are strings or are already parsed into '=' chars.
            if (contents[i][0] == '\"' || contents[i] == "=")
                continue;

            // Else find and parse strings into two sections.
            size_t eqlPos = contents[i].find("=");
            if (eqlPos != std::string::npos)
            {
                std::vector<std::string> parsed = tokenize(contents[i], "=");
                if (parsed.size() == 0)
                    continue;

                // Put erased '=' symbols back in the array.
                for (int j = 0; j < parsed.size(); ++j) {
                    if (parsed[j].length() > 0) {
                        parsed.insert(parsed.begin() + j + 1, "=");
                        j++;
                    }
                }

                // Erase entry where contents were found, and add all new contents to the parent array.
                contents.erase(contents.begin() + i);
                for (int j = 0; j < parsed.size(); ++j) {
                    contents.insert(contents.begin() + i + j, parsed[j]);
                }
                i += parsed.size()-1;

            }
        }

        return contents;
    }

    std::vector<std::pair<std::string, std::string>> getAttributesInTag(const std::string& str)
    {
        std::vector<std::pair<std::string, std::string>> attrs;
        std::vector<std::string> toks = getTagContents(str);
        if (toks.size() <= 1)
            return attrs;

        // Remove the key from tag contents.
        toks.erase(toks.begin());

        // Find entries separated by '=' string, and add them to the attributes array.
        for (int i = 0; i < toks.size() - 2; ++i){
            if (toks[i+1] == "=") {
                attrs.push_back(std::make_pair(toks[i], toks[i+2]));
                i += 2;
            }
        }

        return attrs;
    }

    bool cropNextTagPair(std::string& str)
    {
        if (str.length() == 0)
            return false;

        // End early condition: no tags found.
        size_t strt = str.find(">") + 1;
        size_t endt = str.find("</") - 0;
        if (strt == std::string::npos || endt == std::string::npos)
            return false;

        // Do crop.
        str = str.substr(strt, endt - strt);
        return true;
    }

    bool removeTags(std::string& str){
        bool anyFound = false;
        while ((anyFound = cropNextTagPair(str))) {}
        return anyFound;
    }

    std::string getNextKey(std::string str)
    {
        if (str.length() == 0)
            return str;

        std::string next_key = getNextTag(str);
        next_key = trimLeadingAndTrailingSpaces(next_key);
        while (next_key.length() > 0 && next_key[0] == '<')
            next_key = next_key.substr(1);
        if (next_key.length() > 0 && next_key[0] == '?')//Skip '<?'
            next_key = next_key.substr(1);
        if (next_key.length() > 0 && next_key[0] == '/')//Skip '</'
            next_key = next_key.substr(1);
        while (next_key.length() > 0 && next_key[next_key.length() - 1] == '>')
            next_key = next_key.substr(0, next_key.length() - 1);
        next_key = next_key.substr(0, next_key.find(" "));
        return next_key;
    }

    std::string getNextComment(std::string& str)
    {
        size_t strt = str.find("<?");
        if (strt == std::string::npos)
            return "";
        size_t endt = str.substr(strt).find(">");
        if (endt == std::string::npos)
            return "";

        return str.substr(strt, endt + 1);
    }



    // ----------------------------------------------------------



    // Helper struct for organizing parsed file contents into a tree.
    struct XMLnode
    {
        unsigned int begin = 0;
        unsigned int end = 0;
        std::string key = "";
        std::string filename = "";
        std::vector<std::pair<std::string,std::string>> attributes;
        std::vector<std::shared_ptr<XMLnode>> children;

        inline bool empty()
        {
            return begin == 0 && end == 0;
        }

        // Return entire statement contained within the key tags
        std::string getElement() { return fileReadToString(filename, this->begin, this->end); }

        // Return only the content of a single node element found between its tags, ie for element '<X ..> entry </X>' only 'entry' will be returned.
        std::string getContents() {
            std::string cont = getElement();
            removeTags(cont);
            return cont;
        }
    };

    // Class for parsing nodes from an XML file. This class uses positions in the file to access data, rather than storing it
    // locally in strings. This is to make parsing XML files with data larger than can be reasonably stored on the heap feasible.
    class XMLparser
    {
    public:
        size_t entryCount() {
            return entry_map.size();
        }

        std::string getXMLversion(){
            return XMLversion;
        }
        
        std::unordered_multimap<std::string, std::shared_ptr<XMLnode>>* entries() {
            return &entry_map;
        }

        // Find entries for a given key. NOTE: since data is stored in a multimap, each key may have multiple entries.
        // As such, the 'equal_range()' function is used here rather than 'find()' for returning an interator.
        std::pair<std::unordered_multimap<std::string, std::shared_ptr<XMLnode>>::iterator,
            std::unordered_multimap<std::string, std::shared_ptr<XMLnode>>::iterator> find(std::string key) {
            return entry_map.equal_range(key);
        }

        std::shared_ptr<XMLnode> getEntry(std::string key) {
            return entry_map.find(key)->second;
        }

        const std::set<std::string> getKeys() {
            return keys;
        }

    private:
        // XML version parsed from file.
        std::string XMLversion = "";

        // Hash map of all nodes, referenced by key string.
        std::unordered_multimap<std::string, std::shared_ptr<XMLnode>> entry_map;

        // Parent set of unique keys found in the XML file.
        std::set<std::string> keys;

        // Temporary container string for use while parsing the XML file.
        std::string file_contents;

        // Node for entries as they appear in the file.
        std::shared_ptr<XMLnode> rootNode;

    public:
        // Constructor. This method will parse the text file, initialize the root node, fill the node tree, then close the file.
        XMLparser(std::string filename)
        {
            rootNode = std::make_shared<XMLnode>();

            // Load entire file to 'file_contents' string.
            std::ifstream t(filename, std::ios::in | std::ios::binary);
            t.seekg(0, std::ios::end);
            file_contents.reserve(t.tellg());
            t.seekg(0, std::ios::beg);
            file_contents.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
            t.close();

            // Look for version comment, usually in the first line.
            if (file_contents.find("<?xml ") != std::string::npos)
                XMLversion = getNextComment(file_contents);            

            // Use regular expressions to find all keys in file and their beginning and end positions.
            std::regex r("<(/*)(\\S+?)\\b(.*?)(/*)>");
            for (std::sregex_iterator i = std::sregex_iterator(file_contents.begin(), file_contents.end(), r);
                i != std::sregex_iterator();
                ++i)
            {
                std::smatch m = *i;

                // Get key, add to key list.
                std::string key = getNextKey(m.str());
                keys.emplace(key);

                // Check if this entry exists. If not, create a new node for it.
                bool entryExists = entry_map.find(key) != entry_map.end();
                if (!entryExists)
                {
                    // Make new node.
                    std::shared_ptr<XMLnode> nd = std::make_shared<XMLnode>();
                    nd->begin = m.position();
                    nd->key = key;
                    nd->filename = filename;                    
                    entry_map.emplace(key, nd);
                }
                else
                {
                    // Find the last element in the multimap for this entry.
                    std::shared_ptr<XMLnode> nd = nullptr;
                    auto pair_of_its = entry_map.equal_range(key);
                    auto it = pair_of_its.first;
                    for (; it != pair_of_its.second; ++it)
                    {                       
                        nd = it->second;
                    }

                    // If this is the second time this key has appeared (ie a closing tag), then update this node with 
                    // closing position. If both beginning and end positions for this node already have been found, create
                    // a new node with the same key.
                    if (nd->end == 0)
                    {
                        nd->end = m.position() + file_contents.substr(m.position()).find(">");
                        nd->attributes = getAttributesInTag(getNextTag(nd->getElement()));
                    }
                    else
                    {
                        std::shared_ptr<XMLnode> nd = std::make_shared<XMLnode>();
                        nd->begin = m.position();
                        nd->key = key;
                        nd->filename = filename;
                        entry_map.emplace(key,nd);
                    }
                }
            }

            // Fix mistakes caused by entry_map which occupy a single element, ie '<key ... />'
            for (auto& e : entry_map) 
            {
                auto& nd = e.second;

                // Do quick sanity check for nodes that already have a valid ending to ensure accurate results.
                if (nd->end > 0) {
                    std::string str = nd->getElement();
                    if (str[0] == '<')
                        str[0] = ' ';
                    size_t test_end = str.find("/>");
                    size_t test_end2 = str.find("?>");
                    size_t test_end3 = str.find("<");
                    test_end = test_end < test_end2 ? test_end : test_end2;
                    if (test_end != std::string::npos
                        && test_end < nd->end
                        && test_end < test_end3
                        )
                    {
                        nd->end = nd->begin + test_end + 2;
                        nd->attributes = getAttributesInTag(getNextTag(nd->getElement()));
                    }

                    // Skip next part.
                    continue;
                }

                // If end position is 0, iterate through a string gathered from the file and look for an unusual line 
                // ending. For example, '/>' or '?>'.
                unsigned int charCounter = 0;
                std::ifstream filePtr = std::ifstream(filename.c_str(), std::ios::in | std::ios::binary);
                filePtr.seekg(nd->begin);
                std::string test_str;
                char c = ' ';
                while (c != EOF) {
                    c = filePtr.get();
                    test_str += c;
                    charCounter++;
                    if ( test_str.length() >= 2
                        && (test_str[test_str.length() - 2] == '/' || test_str[test_str.length() - 2] == '?')
                        && test_str[test_str.length() - 1] == '>'
                    )
                    {
                        nd->end = nd->begin + charCounter + 2;
                        nd->attributes = getAttributesInTag(getNextTag(nd->getElement()));
                        break;
                    }
                }
                filePtr.close();
            }

            // Calculate which nodes are child nodes based upon boundaries in the file.
            for (auto& e : entry_map){
                auto& nd = e.second;
                for (auto& e2 : entry_map){
                    if (e2.second->end > 0 && e2.second->begin > nd->begin && e2.second->end < nd->end){
                        nd->children.push_back(e2.second);
                    }
                }
            }

            // Cleanup.
            file_contents.clear();
        }
    };
}