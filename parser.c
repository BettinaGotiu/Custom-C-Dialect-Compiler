#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include<string.h>

#include "parser.h"
#include "ad.h"
#include "utils.h"
#include "at.h"

Token *iTk;		// the iterator in the tokens list
Token *consumedTk;		// the last consumed token
Symbol *owner;

void tkerr(const char *fmt,...){
	fprintf(stderr,"error in line %d: ",iTk->line);
	va_list va;
	va_start(va,fmt);
	vfprintf(stderr,fmt,va);
	va_end(va);
	fprintf(stderr,"\n");
	exit(EXIT_FAILURE);
	}

bool consume(int code){
	if(iTk->code==code){
		consumedTk=iTk;
		iTk=iTk->next;
		return true;
		}
	return false;
	}

// typeBase: TYPE_INT | TYPE_DOUBLE | TYPE_CHAR | STRUCT ID
bool typeBase(Type *t){
    t->n = -1;

	if(consume(TYPE_INT)){
        t->tb=TB_INT;
		return true;
		}
	if(consume(TYPE_DOUBLE)){
        t->tb=TB_DOUBLE;
		return true;
		}
	if(consume(TYPE_CHAR)){
        t->tb=TB_CHAR;
		return true;
		}
	if(consume(STRUCT)){
		if (consume(ID)) {
            Token *tkName = consumedTk;
            t->tb=TB_STRUCT;
            t->s=findSymbol(tkName->text);
            if(!t->s)
                tkerr("Struct undefined: %s !",tkName->text);
																						 
            return true;
        }
        //else tkerr("Struct has not an identifier!");
    }
	return false;
	}

bool structDef(){
    Token *start=iTk;
    if (consume(STRUCT)){
        if(consume(ID)){
            Token *tkName = consumedTk;

            if(consume(LACC)){
                Symbol *s=findSymbolInDomain(symTable,tkName->text);
                if(s)tkerr("Symbol redefinition: %s!",tkName->text);
                s=addSymbolToDomain(symTable,newSymbol(tkName->text,SK_STRUCT));
                s->type.tb=TB_STRUCT;
                s->type.s=s;
                s->type.n=-1;
                pushDomain();
                owner=s;

                for (;;) {
                    if (varDef()) {
                    } else {
                        break;
                    }
                }
                //while(varDef()){}
                if(consume(RACC)){
                    if(consume(SEMICOLON)){
                        owner=NULL;
                        dropDomain();
                        return true;
                    }
                    else tkerr( "Lipseste: ;");
                }
                else tkerr( "Lipseste: }");
            }
        }
        else tkerr( "Lipseste identificatorul dupa structura");
    }
    iTk = start;
    return false;
}

bool varDef(){
    Type t;
    Token *start=iTk;
    if(typeBase(&t)){
        if(consume(ID)){
            Token *tkName = consumedTk;
            if(arrayDecl(&t)){
                if(t.n==0)
                    tkerr("A vector variable must have a specified dimension!");
            }
            if(consume(SEMICOLON)){

                Symbol *var=findSymbolInDomain(symTable,tkName->text);
                if(var)tkerr("symbol redefinition: %s",tkName->text);
                var=newSymbol(tkName->text,SK_VAR);
                var->type=t;
                var->owner=owner;
                addSymbolToDomain(symTable,var);
                if(owner){
                switch(owner->kind){
                case SK_FN:
                var->varIdx=symbolsLen(owner->fn.locals);
                addSymbolToList(&owner->fn.locals,dupSymbol(var));
                break;
                case SK_STRUCT:
                var->varIdx=typeSize(&owner->type);
                addSymbolToList(&owner->structMembers,dupSymbol(var));
                break;
                }
                }else{
                var->varMem=safeAlloc(typeSize(&t));
                }

                return true;
            }
            else tkerr( "Missing: ;");

        }
        else tkerr( "Lipseste identificatorul dupa declaratia de tip");
    }
    iTk = start;
    return false;
}

bool arrayDecl(Type *t){
    Token *start=iTk;
    if(consume(LBRACKET)){
        
        if (consume(INT)) {
            Token *tkSize=consumedTk;
            t->n=tkSize->i;
        } else
            t->n=0;

        if(consume(RBRACKET)){
            return true;
        }
        else tkerr( "Lipseste: ]");
    }
    iTk = start;
    return false;
}

bool fnDef()
{
    Token *start = iTk;
    Type t;
    if (consume(VOID))
    {
        t.tb=TB_VOID;
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (consume(LPAR))
            {
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
                if(fn)tkerr("symbol redefinition: %s",tkName->text);
                fn=newSymbol(tkName->text,SK_FN);
                fn->type=t;
                addSymbolToDomain(symTable,fn);
                owner=fn;
                pushDomain();

                if (fnParam())
                {
                    for (;;)
                    {
                        if (consume(COMMA))
                        {
                            if (fnParam()){}
                            else
                            {
                                tkerr("expected a parameter after ',' in function definition");
                                break;
                            }
                        }
                        else
                        {
                            break;
                        }
                    }
                }
                if (consume(RPAR))
                {
                    if (stmCompound(false))
                    {
                        dropDomain();
                        owner=NULL;
                        return true;
                    }
                }
                else
                {
                    tkerr("expected ) after the function parameters were defined");
                }
            }
            else
            {
                tkerr("expected ( after the function parameters were defined");
            }
        }
        else
        {
            tkerr("missing the name of the function");
        }
    }
    else if (typeBase(&t))
    {
        if (consume(ID))
        {
            Token *tkName = consumedTk;
            if (consume(LPAR))
            {
                Symbol *fn=findSymbolInDomain(symTable,tkName->text);
				if(fn)tkerr("symbol redefinition: %s",tkName->text);
				fn=newSymbol(tkName->text,SK_FN);
				fn->type=t;
				addSymbolToDomain(symTable,fn);
				owner=fn;
				pushDomain();
                if (fnParam())
                {
                    for (;;)
                    {
                        if (consume(COMMA))
                        {
                            if (fnParam()){}
                            else
                            {
                                tkerr("expected a parameter after ',' in function definition");
                                break;
                            }
                        } else break;
                    }
                }
                if (consume(RPAR))
                {
                    if (stmCompound(false))
                    {
                        dropDomain();
						owner=NULL;
                        return true;
                    }
                }
                else
                {
                    tkerr("expected ) after the function parameters were defined");
                }
            }
        }
        else
        {
            tkerr("missing the name of the function");
        }
    }
    iTk = start;
    return false;
}
size_t getPointerSize() {
    return sizeof(void*);
}

bool fnParam() {
    Type t;
    Token *start = iTk;
    if (typeBase(&t)) {
        if (consume(ID)) {
            Token *tkName = consumedTk;
            if (arrayDecl(&t)) {
                t.n = 0; // For function parameters, set array size to 0
            }
            Symbol *param = findSymbolInDomain(symTable, tkName->text);
            if (param) tkerr("Symbol redefinition: %s !", tkName->text);
            param = newSymbol(tkName->text, SK_PARAM);
            param->type = t;
            param->owner = owner;
            param->paramIdx = symbolsLen(owner->fn.params);

            // Use dynamic pointer size
            param->type.n = t.tb == TB_STRUCT && t.n > 0 ? t.n * getPointerSize() : getPointerSize();

            addSymbolToDomain(symTable, param);
            addSymbolToList(&owner->fn.params, dupSymbol(param));
            return true;
        } else {
            tkerr("Lipseste identificatorul de tips");
        }
    }
    iTk = start;
    return false;
}

/*bool fnParam(){
    Type t;
    Token *start=iTk;
    if(typeBase(&t)){
        if(consume(ID)){
           Token *tkName = consumedTk;
            if(arrayDecl(&t)){
                t.n=0;
            }
            Symbol *param=findSymbolInDomain(symTable,tkName->text);
            if(param)tkerr("Symbol redefinition: %s !",tkName->text);
            param=newSymbol(tkName->text,SK_PARAM);
            param->type=t;
            param->owner=owner;
            param->paramIdx=symbolsLen(owner->fn.params);
																					
            addSymbolToDomain(symTable,param);
            addSymbolToList(&owner->fn.params,dupSymbol(param));
            return true;
        }
        else tkerr( "Lipseste identificatorul de tips");
    }
    iTk = start;
    return false;
}*/

bool stm(){
    Token *start=iTk;
    
    if(stmCompound(true)){
        return true;
    }
    iTk=start;
    if(consume(IF)){
        if(consume(LPAR)){
            if(expr(&rCond)){
                 if(!canBeScalar(&rCond))
                    tkerr("The if condition must be a scalar value!");
                if(consume(RPAR)){
                    if(stm()){
                        if(consume(ELSE)){
                            if(stm()){
                                return true;
                            }
                            else tkerr( "Lipseste branch: else");
                        }
                        return true;
                    }
                    else tkerr( "Lipseste branch: if");
                }
                else tkerr( "Lipseste: )");
            }
            else tkerr("Lipseste conditia pentru if");
        }
    }
    iTk=start;
    if(consume(WHILE)){
        if(consume(LPAR)){
            if(expr(&rCond)){
                if(!canBeScalar(&rCond))
                    tkerr("The while condition must be a scalar value!");
                if(consume(RPAR)){
                    if(stm()){
                        return true;
                    }
                    else tkerr( "Lipseste: while");
                }
                else tkerr( "Lipseste: )");
            }
            else tkerr( "Lipseste conditia");
        }
    }
    iTk=start;
    if(consume(RETURN)){
        if(expr(&rExpr)){
            if(owner->type.tb==TB_VOID)
                tkerr("A void function cannot return a value!");
            if(!canBeScalar(&rExpr))
                tkerr("The return value must be a scalar value!");
            if(!convTo(&rExpr.type,&owner->type))
                tkerr("Cannot convert the return expression type to the function return type!");																 
        }
        else {
		if(owner->type.tb!=TB_VOID)
                tkerr("A non-void function must return a value!");
        }
        if(consume(SEMICOLON)){
            return true;
        } else tkerr("Missing ; after RETURN!");
    }
    else if(expr(&rExpr)) {
        if (consume(SEMICOLON)) {
            return true;
        } else tkerr("Expected ; after expression!");
    }
    iTk = start;
    return false;
}


bool stmCompound(bool newDomain){
    Token *start=iTk;
    if(consume(LACC)){
        if(newDomain)
            pushDomain();
        for(;;){
            if(varDef()){}
            else if(stm()){}
            else break;
        }
        if(consume(RACC)){
            if(newDomain)
                dropDomain();
            return true;
        }
        else tkerr( "Lipseste: }");
    }
    iTk = start;
    return false;
}

bool expr(Ret *r){
    Token *start=iTk;
    if(exprAssign(r)){
        return true;
    }
    iTk = start;
    return false;
}

bool exprAssign(){
    Token *start=iTk;
    if(exprUnary()){
        if(consume(ASSIGN)){
            if(exprAssign()){
                return true;
            }
            else tkerr( "Lipseste termenul drept al expresiei");
        }
    }
    iTk=start;
    if(exprOr()){
        return true;
    }
    iTk = start;
    return false;
}

bool exprOr(){
    Token *start=iTk;
    if(exprAnd()){
        if(exprOrSecondary()){
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprOrSecondary(){
    Token *start=iTk;
    if(consume(OR)){
        if(exprAnd()){
            if(exprOrSecondary()){
                return true;
            }
        }
        else tkerr( "Lipseste expresia de dupa: ||");
    }
    iTk = start;
    return true;
}

bool exprAnd(){
    Token *start=iTk;
    if(exprEq()){
        if(exprAndSecondary()){
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprAndSecondary(){
    Token *start=iTk;
    if(consume(AND)){
        if(exprEq()){
            if(exprAndSecondary()){
                return true;
            }
        }
        else tkerr( "Lipseste expresia dupa: &&");
    }
    iTk = start;
    return true;
}

bool exprEq(){
    Token *start=iTk;
    if(exprRel()){
        if(exprEqSecondary()){
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprEqSecondary(){
    Token *start=iTk;
    char c;
    if(start->code == EQUAL) 
      c = '=';
    else 
      c='!';

    if(consume(EQUAL)||consume(NOTEQ)){
        if(exprRel()){
            if(exprEqSecondary()){
                return true;
            }
        }
        else tkerr( "Lipseste expresia de dupa %c=", c);
    }
    iTk = start;
    return true;
}

bool exprRel(){
    Token *start=iTk;
    if(exprAdd()){
        if(exprRelSecondary()){
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprRelSecondary(){
    Token *start=iTk;
    char c[3];
    if(start->code == LESS) 
      strcpy(c, "<");
    else if(start->code == LESSEQ)
      strcpy(c, "<=");
    else if(start->code == GREATER)
      strcpy(c, ">");
    else if(start->code == GREATEREQ)
      strcpy(c, ">=");

    if(consume(LESS)||consume(LESSEQ)||consume(GREATER)||consume(GREATEREQ)){
        if(exprAdd()){
            if(exprRelSecondary()){
                return true;
            }
        }
        else tkerr( "Lipseste expresia de dupa %s", c);
    }
    iTk = start;
    return true;
}
bool exprAdd(){
    Token *start=iTk;
    if(exprMul()){
        if(exprAddSecondary()){
            return true;
        }
    }
    iTk = start;
    return false;
}


bool exprAddSecondary(){
    Token *start=iTk;
    char c;
    if(start->code == ADD) 
     c = '+';
    else 
      c = '-';
    if(consume(ADD)||consume(SUB)){
        if(exprMul()){
            if(exprAddSecondary()){
                return true;
            }
        }
        else tkerr( "Lipseste expresia de dupa %c", c);
    }
    iTk = start;
    return true;
}
bool exprMul(){
    Token *start=iTk;
    if(exprCast()){
        if(exprMulSecondary()){
            return true;
        }
    }
    iTk = start;
    return false;
}

bool exprMulSecondary(){
    Token *start=iTk;
    char c;
    if(start->code == MUL) 
      c = '*';
    else 
      c='/';

    if(consume(MUL)||consume(DIV)){
        if(exprCast()){
            if(exprMulSecondary()){
                return true;
            }
        }
        else tkerr( "Lipseste expresia de dupa %c", c);
    }
    iTk = start;
    return true;
}

bool exprUnary(){
    Token *start=iTk;
    char c;
    if(start->code == SUB) 
      c = '-';
    else 
      c='!';

    if(consume(SUB)||consume(NOT)){
        if(exprUnary()){
            return true;
        }
        else tkerr( "Lipseste expresia de dupa %c", c);
    }
    iTk=start;
    if(exprPostfix()){
        return true;
    }
    iTk = start;
    return false;
}

bool exprPostfixSecondary(){
    Token *start=iTk;
    if(consume(LBRACKET)){
        if(expr()){
            if(consume(RBRACKET)){
                if(exprPostfixSecondary()){
                    return true;
                }
            }
            else tkerr( "Lipseste: ]");
        }
        else tkerr( "Lipseste expresia dintre: []");
    }
    iTk=start;
    if(consume(DOT)){
        if(consume(ID)){
            if(exprPostfixSecondary()){
                return true;
            }
        }
        else tkerr( "Lipseste identificatorul de dupa .");
    }
    iTk = start;
    return true;
}

bool exprSecondaryary(){
    Token *start=iTk;
    if(consume(ID)){
        if(consume(LPAR)){
            if(expr()){
                while(consume(COMMA)){
                    if(expr()){}
                    else{
                        tkerr( "Lipseste expresie dupa ,");
                    }
                }
            }
            if(consume(RPAR)){
                return true;
            }
            else tkerr( "Lipseste: )");
        }
        return true;
    }
    iTk=start;
    if(consume(TYPE_INT)){
        return true;
    }
    if(consume(TYPE_DOUBLE)){
        return true;
    }
    if(consume(TYPE_CHAR)){
        return true;
    }
    if(consume(LPAR)){
        if(expr()){
            if(consume(RPAR)){
                return true;
            }
            else tkerr( "Lipseste )");
        }
        else tkerr( "Lipseste expresia dupa (");
    }
    iTk = start;
    return false;
}

bool exprCast(){
    Token *start=iTk;
    if(consume(LPAR)){
        Type t;
        if(typeBase(&t)){
            if(arrayDecl(&t)){}
            if(consume(RPAR)){
                if(exprCast()){
                    return true;
                }
            }
            else tkerr(" Lipseste : )");
        }
    }
    iTk=start;
    if(exprUnary()){
        return true;
    }
    iTk = start;
    return false;
}

bool exprPostfix(){
    Token *start=iTk;
    if(exprPrimary()){
        if(exprPostfixSecondary()){
            return true;
        }
    }
    iTk = start;
    return false;
}


bool exprPrimary(){
    Token *start=iTk;
    if(consume(ID)){
        if(consume(LPAR)){
            if(expr()){
                while(consume(COMMA)){
                    if(expr()){}
                    else{
                        tkerr(" Lipseste expresie dupa  ,");
                    }
                }
            }
            if(consume(RPAR)){
                return true;
            }
            else tkerr(" Lipseste: )");
        }
        return true;
    }
    iTk=start;
    if(consume(INT)){
        return true;
    }
    if(consume(DOUBLE)){
        return true;
    }
    if(consume(CHAR)){
        return true;
    }
    if(consume(STRING)){
        return true;
    }
    if(consume(LPAR)){
        if(expr()){
            if(consume(RPAR)){
                return true;
            }
            else tkerr(" Lipseste: )");
        }
        else tkerr(" Lipseste expresie dupa (");
    }
    iTk = start;
    return false;
}

// unit: ( structDef | fnDef | varDef )* END
bool unit(){
	for(;;){
		if(structDef()){}
		else if(fnDef()){}
		else if(varDef()){}
		else break;
		}
	if(consume(END)){
		return true;
		}
	return false;
	}

void parse(Token *tokens){
	iTk=tokens;
	if(!unit())tkerr("syntax error");
	}
