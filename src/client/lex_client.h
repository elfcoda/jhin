#pragma once

#include "llvm/Support/raw_ostream.h"
#include <fstream>
#include <memory>
#include "../lex/lex.h"
#include "client.h"
#include "../../comm/comm_define.h"
#include "../../comm/jhin_assert.h"
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
    llvm::outs() << "length of file " << filename << ": " << source.length() << "\n\n";
    if (source.length() >= MAX_FILE_SIZE - 5) {
        llvm::outs() << "ERROR: file too big\n";
        JHIN_ASSERT_BOOL(false);
        return std::vector<std::pair<unsigned, std::string>>{};
    }
    llvm::outs() << source << "\n";

    /* start parsing */
    std::shared_ptr<Lex> l = std::make_shared<Lex>();
    std::vector<std::pair<unsigned, std::string>> parseResult;

    std::pair<bool, std::string> res = l->parse(source, filename, parseResult);

    if (res.first == false) {
        llvm::outs() << res.second << "\n\n";
        JHIN_ASSERT_BOOL(false);
    } else {
        llvm::outs() << "lexical parse successfully\n\n";
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
}   /* namespace jhin */

