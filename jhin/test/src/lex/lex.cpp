#include <iostream>
#include <fstream>
#include "../../../src/lex/lex.h"
#include "../../../comm/comm_define.h"
using namespace jhin::comm;
using namespace jhin::lex;

int main()
{
    /* get source code */
    char src[MAX_FILE_SIZE];
    std::ifstream srcfile;
    std::string filename = "lex.jhin";
    std::string fileout = "lex.out";
    srcfile.open(filename);
    srcfile.getline(src, MAX_FILE_SIZE, EOF);
    srcfile.close();
    std::string source = src;
    std::cout << "length of file " << filename << ": " << source.length() << std::endl << std::endl;
    if (source.length() >= MAX_FILE_SIZE - 5) {
        std::cout << "ERROR: file too big" << std::endl;
        return 0;
    }
    std::cout << source << std::endl;

    /* start parsing */
    Lex* l = new Lex();
    std::vector<std::pair<unsigned, std::string>> parseResult;

    std::pair<bool, std::string> res = l->parse(source, filename, parseResult);

    if (res.first == false) {
        std::cout << res.second << std::endl << std::endl;
    } else {
        std::cout << "lexical parse successfully" << std::endl << std::endl;
    }

    std::ofstream outfile;
    outfile.open(fileout);
    for (const auto it: parseResult) {
        std::string s = jhin::lex::tokenId2String[it.first] + "\t" + it.second + "\n";
        std::cout << s;
        outfile.write(s.c_str(), s.length());
    }
    outfile.close();

    delete l;

    return 0;
}

