#pragma once

#include "lexer.h"
#include <stdbool.h>

void parse(Token *tokens);
bool unit();
bool structDef();
bool varDef();
bool typeBase();
bool arrayDecl();
bool fnDef();
bool fnParam();
bool stm();
bool stmCompound(bool newDomain); 
bool expr();
bool exprAssign();
bool exprOr();
bool exprOrSecondary();
bool exprAnd();
bool exprAndSecondary();
bool exprEq();
bool exprEqSecondary();
bool exprRel();
bool exprRelSecondary();
bool exprAdd();
bool exprAddSecondary();
bool exprMul();
bool exprMulSecondary();
bool exprCast();
bool exprUnary();
bool exprPostfix();
bool exprPrimary();
