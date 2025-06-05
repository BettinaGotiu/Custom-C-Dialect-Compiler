#pragma once

enum
{
	ID, INT, DOUBLE, STRING, CHAR, 
	TYPE_CHAR, TYPE_INT, TYPE_DOUBLE, STRUCT, VOID,
	COMMA, END, SEMICOLON,RETURN,
	ASSIGN, EQUAL, LESS, DIV, ADD, AND, MUL,FOR ,BREAK , SUB, OR, NOT, NOTEQ, LESSEQ, GREATER, GREATEREQ, LPAR, RPAR, LACC, RACC, LBRACKET, RBRACKET, WHILE, IF, ELSE, DOT
};

typedef struct Token
{
	int code; // ID, TYPE_CHAR, ...
	int line; // the line from the input file
	union
	{
		char *text; // the chars for ID, STRING (dynamically allocated)
		int i;		// the value for INT
		char c;		// the value for CHAR
		double d;	// the value for DOUBLE
	};
	struct Token *next; // next token in a simple linked list
} Token;

Token *tokenize(const char *pch);
void showTokens(const Token *tokens);
