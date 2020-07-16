LALR:
-- RE_VALUE: TypeName(Int String), ClassName(Animal)

Prog'   ::= Prog
Prog    ::= (Class | Proc)+
Class   ::= class RE_VALUE [inherits RE_VALUE]? { Declare }

Command ::= RE_ID <- Exp
        |   while 

-- Exp, separate Exp by several sub-Exps, can be join by '|'
Exp     ::= ExpV
        |   ExpO
        |   ExpB
        |   ExpT
        |   RE_ID       -- id can be a type, eg. m: Type = Int, or a value
        |   case Exp of {(RE_ID: ExpT|RE_ID|Type => Exp) (| RE_ID: ExpT|RE_ID|Type => Exp)*}    -- otherwise

-- Value exp
ExpV    ::= RE_INT
        |   RE_DECIMAL
        |   RE_STRING
        |   ExpV + ExpV
        |   ExpV - ExpV
        |   ExpV * ExpV
        |   ExpV / ExpV
        |   Expv == ExpV
        |   Expv != ExpV   -- Not Equal need to implementing first
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
        |   OBJECT      -- Type value
        |   BOOL
        |   INT
        |   FLOAT
        |   DOUBLE
        |   LONG
        |   STRING
        |   UNIT
        |   ExpT + ExpT
        |   ExpT * ExpT

