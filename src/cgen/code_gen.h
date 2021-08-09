#pragma once

#include <memory>
#include <string>
#include <unordered_map>
#include "../ast/ast_node.h"
#include "../st/symbol_def.h"
#include "../st/symbol_table.h"
#include "../../comm/type_tree.h"
#include "../../comm/jhin_assert.h"

namespace jhin
{
namespace cgen
{
/* memory layout:
 * -----------
 * | code    |       Low address
 * -----------
 * | heap    |
 * -----------
 * | ...     |
 * -----------
 * | stack   |       High address
 * -----------
 *
 * register:
 * rbp: bottom of stack ptr
 * rsp: top of stack ptr
 * rip: code ptr
 * rax: function return value
 * rbx: result register
 * rdx: tmp register
 *
 * one register stack machine:
 * -----------      High address
 * | ...     |
 * -----------
 * | ret adr |  ... pushq
 * -----------
 * | rbp     |  ... rbp
 * -----------
 * | args... |  ... link with symbol table
 * -----------
 * | loc var |  ... link with symbol table, and jmp
 * | ...     |
 * -----------  ... rsp (available address)
 * | ...     |
 * -----------      Low address
 * */


}   /* namespace cgen */
}   /* namespace jhin */

