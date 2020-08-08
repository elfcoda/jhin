#ifndef __LEX_CLIENT_H__
#define __LEX_CLIENT_H__

#include <iostream>
#include <fstream>
#include <memory>
#include "../lex/lex.h"
#include "client.h"
#include "../../comm/comm_define.h"
using namespace jhin::comm;
using namespace jhin::lex;

namespace jhin
{
namespace client
{

std::vector<std::pair<unsigned, std::string>> lexClient()
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
        assert(false);
        return std::vector<std::pair<unsigned, std::string>>{};
    }
    std::cout << source << std::endl;

    /* start parsing */
    std::shared_ptr<Lex> l = std::make_shared<Lex>();
    std::vector<std::pair<unsigned, std::string>> parseResult;

    std::pair<bool, std::string> res = l->parse(source, filename, parseResult);

    if (res.first == false) {
        std::cout << res.second << std::endl << std::endl;
        assert(false);
    } else {
        std::cout << "lexical parse successfully" << std::endl << std::endl;
    }

    std::ofstream outfile;
    outfile.open(fileout);
    for (const auto& it: parseResult) {
        std::string s = jhin::lex::tokenId2String[it.first] + "\t" + it.second + "\n";
        outfile.write(s.c_str(), s.length());
    }
    outfile.close();

    return parseResult;
}

}   /* namespace client */
};  /* namespace jhin */


#endif

