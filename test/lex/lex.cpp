#include <iostream>
#include "../../src/lex/lex.h"
using namespace std;
using namespace jhin::tools;
using namespace jhin::lex;

int main()
{
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
}
