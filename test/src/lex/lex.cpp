#include <iostream>
#include <fstream>
#include "../../../src/lex/lex.h"
#include "../../../comm/comm_define.h"
using namespace std;
using namespace jhin::comm;
using namespace jhin::lex;

int main()
{
    /* get source code */
    char src[MAX_FILE_SIZE];
    ifstream srcfile;
    string filename = "lex.jhin";
    string fileout = "lex.out";
    srcfile.open(filename);
    srcfile.getline(src, MAX_FILE_SIZE, EOF);
    srcfile.close();
    string source = src;
    cout << "length of file " << filename << ": " << source.length() << endl << endl;
    if (source.length() >= MAX_FILE_SIZE - 5) {
        cout << "ERROR: file too big" << endl;
        return 0;
    }
    cout << source << endl;

    /* start parsing */
    Lex* l = new Lex();
    vector<pair<string, string>> parseResult;

    pair<bool, string> res = l->parse(source, filename, parseResult);

    if (res.first == false) {
        cout << res.second << endl << endl;
    } else {
        cout << "lexical parse successfully" << endl << endl;
    }

    ofstream outfile;
    outfile.open(fileout);
    for (const auto it: parseResult) {
        string s = it.first + "\t" + it.second + "\n";
        cout << s;
        outfile.write(s.c_str(), s.length());
    }
    outfile.close();

    delete l;

    return 0;
}

