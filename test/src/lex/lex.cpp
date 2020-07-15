#include <iostream>
#include <fstream>
#include "../../../src/lex/lex.h"
#include "../../../comm/comm_define.h"
using namespace std;
using namespace jhin::comm;
using namespace jhin::lex;

int main()
{
    char source[MAX_FILE_SIZE];
    ifstream srcfile;
    srcfile.open("lex.jhin");
    srcfile.getline(source, MAX_FILE_SIZE, EOF);
    cout << source << endl;

    srcfile.close();
    return 0;

    /*
    string source = "Int main <- 6;";
    Lex* l = new Lex();
    vector<pair<string, string>> parseResult;

    pair<bool, string> res = l->parse(source, parseResult);

    if (res.first == false) {
        cout << res.second << endl << endl;
    } else {
        cout << "lexical parse successfully" << endl << endl;
    }

    for (const auto it: parseResult) {
        cout << it.first << "\t" << it.second << endl;
    }

    return 0;
    */
}
