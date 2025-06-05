# Custom-C-Dialect-Compiler
Built a compiler for a custom C-inspired language, implementing lexical analysis, parsing, type checking, and code generation

---
## Compiler Code Structure

This project implements a full compilation pipeline for a custom C-inspired language. The major components and their roles are as follows:

### 1. Lexical Analysis (`lexer.c`)

**Purpose:**  
Transforms raw source code into a stream of tokens (lexical units), each representing keywords, identifiers, literals, or operators.

**Main Structures:**
- `Token`: Represents a single token, with fields for type (`code`), value (`i`, `d`, `c`, `text`), line number (`line`), and a pointer to the next token (`next`).

**Key Functions:**
- `tokenize(const char*)`: Main entry point. Scans the input string, matches patterns, and produces a linked list of tokens.
- `addTk(int code)`: Allocates a new token and appends it to the list.
- `extract(const char*, const char*)`: Utility to extract substrings for identifiers/strings.
- `showTokens(const Token*)`: Debug function to print all tokens for inspection.

**Process:**  
The lexer reads the input character-by-character, identifies token boundaries based on whitespace and punctuation, and classifies each token (e.g., keywords, operators, numbers, strings, identifiers). It handles comments, newlines, and error reporting.

---

### 2. Parsing (`parser.c`)

**Purpose:**  
Converts the token stream into an Abstract Syntax Tree (AST) that represents the program’s syntactic structure according to the grammar of the language.

**Main Structures:**  
- `ASTNode`: Represents nodes in the syntax tree (expressions, statements, blocks, etc.).

**Key Functions :**
- `parseProgram()`, `parseStatement()`, `parseExpression()`, etc.: Recursive descent functions for grammar rules.
- AST construction and error handling routines.

**Process:**  
The parser consumes the token list and builds a tree structure reflecting program logic (e.g., expressions, control flow, function definitions). Syntax errors are reported here.

---

### 3. Attribute and Type Checking (`at.c`)

**Purpose:**  
 Ensures all operations and assignments use compatible types, and that expressions have valid types.

**Main Structures:**
- `Type`: Represents a type, with fields for type base (`tb`), struct info, and array info (`n`).
- `Ret`: Used to hold type information for expressions
- `Symbol`: Represents variables, functions, structs, etc.

**Key Functions:**
- `canBeScalar(Ret*)`: Checks if a value can be treated as a scalar (not array/pointer, not void).
- `convTo(Type*, Type*)`: Determines if one type can be converted to another (handles numeric types, pointers, and structs).
- `arithTypeTo(Type*, Type*, Type*)`: Determines result type for arithmetic operations, enforcing correct operand types.
- `findSymbolInList(Symbol*, const char*)`: Searches a symbol list for a name.

**Process:**  
Enforces rules like: only compatible types can be assigned or operated on; numeric types are convertible among each other; struct conversions require same struct type.

---

### 4. Symbol Table and Domain Management (`ad.c`)

**Purpose:**  
Implements symbol table logic (scoping, variable/function/struct tracking), type size calculations, and symbol memory management.

**Main Structures:**
- `Symbol`: Represents symbols in the program—variables, functions, structs, parameters.
- `Domain`: Represents a scope (block, function, global), holds a list of symbols and a parent pointer.
- `Type`: Used for type information on symbols.

**Key Functions:**
- **Symbol/Type Management:**
  - `typeBaseSize(Type*)`, `typeSize(Type*)`: Compute sizes for various types, including structs and arrays.
  - `newSymbol`, `dupSymbol`, `addSymbolToList`, `addSymbolToDomain`: Create, duplicate, and manage lists of symbols.
  - `freeSymbol`, `freeSymbols`: Memory management for symbols.
  - `addExtFn`, `addFnParam`: Add external functions and parameters to domains.
- **Symbol Table (Scope) Management:**
  - `pushDomain()`, `dropDomain()`: Enter/exit new scopes (blocks/functions).
  - `findSymbolInDomain`, `findSymbol`: Look up symbols in current or nested scopes.
- **Debug/Utility:**
  - `showNamedType`, `showSymbol`, `showDomain`: Pretty-print types, symbols, and domains for debugging.

**Process:**  
Whenever entering a new scope, a new `Domain` is pushed. Variables and functions are added to the current domain. When leaving a scope, that domain is popped and its symbols are freed. This supports proper variable/function scoping and lifetime. Symbols contain all relevant info for semantic checks and code generation.

---

## How the Compiler Was Built

1. **Lexical Analysis**:  
   - Implemented as a hand-written scanner in `lexer.c` using a loop and switch-case logic.
   - Produces linked-token streams for parsing.

2. **Parsing**:  
   - (Expected in `parser.c`) Implements recursive-descent parsing, building AST using token streams and grammar rules.
   - Handles syntax errors and constructs the tree for the next stage.

3. **Type/Attribute Checking**:  
   - In `at.c`, provides logic to enforce type safety and correct attribute usage for any expression or statement.

4. **Symbol Table and Scoping**:  
   - In `ad.c`, manages all identifiers, their lifetimes, scopes, and types, supporting variables, functions, structs and their parameters/members.

5. **Memory and Utility Functions**:  
   - Error reporting and safe allocation can be found in `utils.c`.
---

**Project Note:**  
This compiler was developed as a lab project for the course "Compiler Construction" during February to May 2024.
