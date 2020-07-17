LALR:  
-- RE_VALUE: TypeName(Int String), ClassName(Animal)  
  
Prog'   ::= Prog  
Prog    ::= (Class | Proc)+  
Class   ::= class RE_VALUE [inherits RE_VALUE]? { Declare }  
  
-- elif may account for the complexity of code logic, so I remove the syntax. elif can be replaced by "recursive if-else" or "case of" syntax  
Command ::= RE_ID <- Exp  
        |   while (Exp) { Formal }  
        |   if (Exp) { Formal }  
        |   if (Exp) { Formal } else { Formal }  -- if (Exp) { Formal } [elif (Exp) { Formal }]* [else { Formal }]  
        |   RE_ID()  
  
Formal  ::= (Command|Exp|Declare) *  
  
-- Declare  
Declare ::= RE_ID: Type  
        |   RE_ID: Type <- Exp  
        |   RE_ID([RE_ID: Type [, RE_ID: Type]*])[: Type] { '\n'* [(Exp|Command|Declare) ('\n'+ (Exp|Command|Declare))*] '\n'* }  
  
-- Exp, separate Exp by several sub-Exps, can be joint by '|'  
-- return value of multi-exp is the return value of the last exp  
Exp     ::= ExpV  
        |   ExpO  
        |   ExpB  
        |   ExpT  
        |   RE_ID       -- id can be a type, eg. m: TYPE = Int, or a value  
        |   case Exp of {(RE_ID: Type => Exp '\n'|)* (otherwise => Exp)}  
        |   return Exp  
        |   let Declare  
        |   lambda  ... -> =    // TODO  
  
-- Types  
Type    ::= ExpT|RE_ID|TYPE  
  
-- Value exp  
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
  
             
-- Object exp  
ExpO    ::= THIS  
  
-- Bool exp  
ExpB    ::= TRUE  
        |   FALSE  
        |   !ExpB  
  
-- Type exp  
ExpT    ::= RE_VALUE    -- Type is a kind of value(ClassName)  
        |   OBJECT      -- supertype of any type  
        |   BOOL  
        |   INT  
        |   FLOAT  
        |   DOUBLE  
        |   LONG  
        |   STRING  
        |   UNIT  
        |   NOTYPE      -- subtype of any type  
        |   ExpT + ExpT  
        |   ExpT * ExpT  

