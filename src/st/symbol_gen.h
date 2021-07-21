#pragma once

#include <string>
#include <vector>
#include <memory>
#include "symbol_def.h"
#include "symbol_table.h"
#include "../cgen/code_gen.h"
#include "../ts/type_checker.h"
#include "../ast/ast_node.h"
#include "../../comm/type_tree.h"


                    for (unsigned idx = 0; idx <  pRoot->size(); idx++)
                        ast::pASTNode child = pRoot->getChild(idx);
                        if (getSymbolType(pTT) == E_ID_TYPE_FN_TYPE) {
                        pTypeTree pArgsTree = genFnTypes(pRoot->getChild(1));

                    }
   

高阶的特性要自己处理自己翻译成llvm
所以自己保留pTT及相关符号表
接下来是基于pTT的符号表和类型系统，然后cgen
基于pTT的符号表(Type * into pTT)