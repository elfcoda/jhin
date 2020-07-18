## LALR:  
-- RE_VALUE: TypeName(Int String), ClassName(Animal)  
  
### -- program  
Prog'   ::= Prog  
Prog    ::= (Class | Proc)+  
  
  
### -- elif may account for the complexity of code logic, so I remove the syntax. elif can be replaced by "recursive if-else" or "case of" syntax, return NoType?
Cmd     ::= CmdN  
        |   CmdC
        |   CmdU

### -- normal
CmdN    ::= RE_ID <- Exp
       
### -- curly if(while)-exp can recurse, by other if(while)-exp  
CmdC    ::= while (Exp) { Formals }  
        |   if (Exp) { Formals }  
        |   if (Exp) { Formals } else { Formals }   -- if (Exp) { Formals } [elif (Exp) { Formals }]* [else { Formals }]  
        
### -- uncurly. donot followed by if-exp and while-exp. in this case, recursive if-exp may lead to ambigious semantics, cannot recurse with other if(while)-exp  
CmdU    ::= while (Exp) FormalU  
        |   if (Exp) FormalU else FormalU   
        |   if (Exp) FormalU  
  
### -- Formal  
Formals ::= (Cmd|Exp|DeclN) *  
Formal  ::= Cmd|Exp|DeclN  
FormalU ::= CmdN|Exp|DeclN  
  
  
### -- Decl  
Decl    ::= DeclN  
        |   Class  
  
### -- DeclN  
DeclN   ::= RE_ID: Type  
        |   RE_ID: Type <- Exp  
        |   Proc  

### -- class  
Class   ::= class RE_VALUE [inherits RE_VALUE]? { Decl * }  
### -- procedure  
Proc    ::= RE_ID([RE_ID: Type [, RE_ID: Type] * ])[: Type] { '\n' * [(Exp|Cmd|DeclN) ('\n'+ (Exp|Cmd|DeclN)) * ] '\n' * }  
  
### -- Types  
Type    ::= ExpT|RE_ID|TYPE  
  
  
### -- Exp, separate Exp by several sub-Exps, can be joint by '|', return value of multi-exp is the return value of the last exp  
Exp     ::= ExpV  
        |   ExpO  
        |   ExpB  
        |   ExpT  
        |   RE_ID       -- id can be a type, eg. m: TYPE = Int, or a value  
        |   case Exp of {(RE_ID: Type => Exp '\n'|)* (otherwise => Exp)}  
        |   return Exp  
        |   let DeclN (, DeclN)* in Exp  
        |   RE_ID()  
        |   RE_ID(Exp [, Exp] * )  
        |   lambda -> Exp  
        |   lambda DeclN [,DeclN] * -> Exp  
  
### -- Value exp  
ExpV    ::= RE_INT  
        |   RE_DECIMAL  
        |   RE_STRING  
        |   ExpV + ExpV  
        |   ExpV - ExpV  
        |   ExpV * ExpV  
        |   ExpV / ExpV  
        |   Expv == ExpV  
        |   Expv != ExpV   -- Not Equal need implementing first  
        |   (ExpV)  
        |   !ExpV  
        |   ExpV < ExpV  
        |   ExpV <= ExpV  
        |   ExpV > ExpV  
        |   ExpV >= ExpV  
        |   isVoid ExpV  
        |   new RE_VALUE  
  
             
### -- Object exp  
ExpO    ::= THIS  
  
### -- Bool exp  
ExpB    ::= TRUE  
        |   FALSE  
        |   !ExpB  
  
### -- Type exp  
ExpT    ::= RE_VALUE    -- Type is a kind of value(ClassName)  
        |   OBJECT      -- supertype of any type  
        |   BOOL  
        |   INT  
        |   FLOAT  
        |   DOUBLE  
        |   LONG  
        |   STRING  
        |   UNIT  
        |   ExpT + ExpT  
        |   ExpT * ExpT  

