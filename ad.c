#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "utils.h"
#include "ad.h"

Domain *symTable=NULL;

// typeBaseSize: This function returns the size in bytes of a type base (e.g., int, double, char, void).
// For structures, it calculates the total size by summing the sizes of its members.
int typeBaseSize(Type *t) {
    switch(t->tb) {
        case TB_INT: return sizeof(int);
        case TB_DOUBLE: return sizeof(double);
        case TB_CHAR: return sizeof(char);
        case TB_VOID: return 0;
        default: { // TB_STRUCT
            int size = 0;
            for (Symbol *m = t->s->structMembers; m; m = m->next) {
                size += typeSize(&m->type);
            }
            return size;
        }
    }
}

// typeSize: This function returns the size of a type, taking into account arrays.
// It handles base types, arrays, and pointers (arrays of size 0 are treated as VOID).
int typeSize(Type *t) {
    if (t->n < 0) return typeBaseSize(t);
    if (t->n == 0) return sizeof(void*); 
    return t->n * typeBaseSize(t);
}

// freeSymbols: This function frees a list of symbols from memory.
// It iterates over the list and calls freeSymbol on each symbol.
void freeSymbols(Symbol *list) {
    for (Symbol *next; list; list = next) {
        next = list->next;
        freeSymbol(list);
    }
}

// newSymbol: This function creates a new symbol with the given name and kind (e.g., variable, function, struct).
// It initializes the symbol's fields and returns a pointer to the new symbol.
Symbol *newSymbol(const char *name, SymKind kind) {
    Symbol *s = (Symbol*)safeAlloc(sizeof(Symbol));
    memset(s, 0, sizeof(Symbol)); // sets all the fields to 0/NULL
    s->name = name;
    s->kind = kind;
    return s;
}

// dupSymbol: This function duplicates a symbol by creating a new symbol and copying the fields from the original symbol.
// The next pointer is set to NULL to ensure it's a standalone symbol.
Symbol *dupSymbol(Symbol *symbol) {
    Symbol *s = (Symbol*)safeAlloc(sizeof(Symbol));
    *s = *symbol;
    s->next = NULL;
    return s;
}

// addSymbolToList: This function adds a symbol to a list of symbols.
// If the list is empty, it initializes the list with the new symbol. Otherwise, it appends the symbol to the end of the list.
Symbol *addSymbolToList(Symbol **list, Symbol *s) {
    Symbol *iter = *list;
    if (iter) {
        while (iter->next) iter = iter->next;
        iter->next = s;
    } else {
        *list = s;
    }
    return s;
}

// symbolsLen: This function returns the length (number of symbols) of a list of symbols by iterating through the list.
int symbolsLen(Symbol *list) {
    int n = 0;
    for (; list; list = list->next) n++;
    return n;
}

// freeSymbol: This function frees a single symbol from memory.
// It handles different kinds of symbols (variables, functions, structs) and frees associated memory accordingly.
void freeSymbol(Symbol *s) {
    switch (s->kind) {
        case SK_VAR:
            if (!s->owner) free(s->varMem);
            break;
        case SK_FN:
            freeSymbols(s->fn.params);
            freeSymbols(s->fn.locals);
            break;
        case SK_STRUCT:
            freeSymbols(s->structMembers);
            break;
    }
    free(s);
}

// pushDomain: This function creates a new domain, sets it as the current symbol table (symTable),
// and returns a pointer to the new domain. The new domain’s parent is set to the previous current domain.
Domain *pushDomain() {
    Domain *d = (Domain*)safeAlloc(sizeof(Domain));
    d->symbols = NULL;
    d->parent = symTable;
    symTable = d;
    return d;
}

// dropDomain: This function removes the current domain from the symbol table,
// frees its symbols, and sets the parent domain as the current domain.
void dropDomain() {
    Domain *d = symTable;
    symTable = d->parent;
    freeSymbols(d->symbols);
    free(d);
}

// showNamedType: This function prints a type with its name.
// It handles different base types and arrays, formatting the output accordingly.
void showNamedType(Type *t, const char *name) {
    switch (t->tb) {
        case TB_INT: printf("int"); break;
        case TB_DOUBLE: printf("double"); break;
        case TB_CHAR: printf("char"); break;
        case TB_VOID: printf("void"); break;
        default: // TB_STRUCT
            printf("struct %s", t->s->name);
    }
    if (name) printf(" %s", name);
    if (t->n == 0) printf("[]");
    else if (t->n > 0) printf("[%d]", t->n);
}

// showSymbol: This function prints detailed information about a symbol, including its type, name, size, and other relevant attributes.
// It handles different kinds of symbols (variables, parameters, functions, structs).
void showSymbol(Symbol *s) {
    switch (s->kind) {
        case SK_VAR:
            showNamedType(&s->type, s->name);
            if (s->owner) {
                printf(";\t// size=%d, idx=%d\n", typeSize(&s->type), s->varIdx);
            } else {
                printf(";\t// size=%d, mem=%p\n", typeSize(&s->type), s->varMem);
            }
            break;
        case SK_PARAM:
            showNamedType(&s->type, s->name);
            printf(" /*size=%d, idx=%d*/", typeSize(&s->type), s->paramIdx);
            break;
        case SK_FN:
            showNamedType(&s->type, s->name);
            printf("(");
            bool next = false;
            for (Symbol *param = s->fn.params; param; param = param->next) {
                if (next) printf(", ");
                showSymbol(param);
                next = true;
            }
            printf(") {\n");
            for (Symbol *local = s->fn.locals; local; local = local->next) {
                printf("\t");
                showSymbol(local);
            }
            printf("\t}\n");
            break;
        case SK_STRUCT:
            printf("struct %s {\n", s->name);
            for (Symbol *m = s->structMembers; m; m = m->next) {
                printf("\t");
                showSymbol(m);
            }
            printf("};\t// size=%d\n", typeSize(&s->type));
            break;
    }
}

// showDomain: This function prints all symbols in a given domain with a specified name.
// It iterates through the domain’s symbols and prints each one.
void showDomain(Domain *d, const char *name) {
    printf("// domain: %s\n", name);
    for (Symbol *s = d->symbols; s; s = s->next) {
        showSymbol(s);
    }
    puts("\n");
}

// findSymbolInDomain: This function searches for a symbol by name within a specific domain
// and returns a pointer to the symbol if found, or NULL if not found.
Symbol *findSymbolInDomain(Domain *d, const char *name) {
    for (Symbol *s = d->symbols; s; s = s->next) {
        if (!strcmp(s->name, name)) return s;
    }
    return NULL;
}

// findSymbol: This function searches for a symbol by name in the current domain and its parent domains.
// It returns a pointer to the symbol if found, or NULL if not found.
Symbol *findSymbol(const char *name) {
    for (Domain *d = symTable; d; d = d->parent) {
        Symbol *s = findSymbolInDomain(d, name);
        if (s) return s;
    }
    return NULL;
}

// addSymbolToDomain: This function adds a symbol to a specified domain.
// It calls addSymbolToList to add the symbol to the domain's symbol list.
Symbol *addSymbolToDomain(Domain *d, Symbol *s) {
    return addSymbolToList(&d->symbols, s);
}

// addExtFn: This function adds an external function to the current domain.
// It creates a new function symbol, sets its return type and external function pointer, and adds it to the current domain.
Symbol *addExtFn(const char *name, void (*extFnPtr)(), Type ret) {
    Symbol *fn = newSymbol(name, SK_FN);
    fn->fn.extFnPtr = extFnPtr;
    fn->type = ret;
    addSymbolToDomain(symTable, fn);
    return fn;
}

// addFnParam: This function adds a parameter to a function symbol.
// It creates a new parameter symbol, sets its type and index, duplicates it, and adds it to the function's parameter list.
Symbol *addFnParam(Symbol *fn, const char *name, Type type) {
    Symbol *param = newSymbol(name, SK_PARAM);
    param->type = type;
    param->paramIdx = symbolsLen(fn->fn.params);
    addSymbolToList(&fn->fn.params, dupSymbol(param));
    return param;
}
