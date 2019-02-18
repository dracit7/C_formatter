// 
// parser.c
// 
// The lexical and grammatical analysis part of the formatter.
// 

/* Grammar supported (Represented by BNF without left-recursion): 
 * program -> externalDef program | externalDef
 * externalDef -> globalVarDef | funcDef
 * globalVarDef -> type varSeq ;
 * type -> INT | FLOAT | CHAR
 * varSeq -> IDENT , varSeq | IDENT
 * funcDef -> type IDENT ( paramSeq ) compState
 * paramSeq -> param , paramSeq | EMPTY
 * param -> type IDENT
 * compState -> { statementSeq }
 * statementSeq -> statement statmentSeq | EMPTY
 * statement -> localVarDef | stat
 * localVarDef -> type varSeq ;
 * stat -> expr ; 
 *       | RETURN expr ;
 *       | IF ( expr ) compStateOrStat
 *       | IF ( expr ) compStateOrStat ELSE compStateOrStat
 *       | WHILE ( expr ) compStateOrStat
 * compStateOrStat -> compState | stat
 * expr -> prior0 morePrior0    // These steps removes left-recursion
 * prior0 -> prior1 morePrior1
 * prior1 -> prior2 morePrior2
 * prior2 -> prior3 morePrior3
 * prior3 -> prior4 morePrior4
 * prior4 -> NOT factor
 *         | factor
 * morePrior0 -> ASSGIN prior0 morePrior0
 *             | EMPTY
 * morePrior1 -> EQUAL prior1 morePrior1
 *             | NEQUAL prior1 morePrior1
 *             | EMPTY
 * moreProir2 -> GREATER prior2 morePrior2
 *             | LESS prior2 morePrior2
 *             | LEQUAL prior2 morePrior2
 *             | GEQUAL prior2 morePrior2
 *             | EMPTY
 * moreProir3 -> PLUS prior3 morePrior3
 *             | MINUS prior3 morePrior3
 *             | EMPTY
 * moreProir4 -> MULTI proir4 moreProir4
 *             | DIV proir4 moreProir4
 *             | MOD proir4 moreProir4
 *             | EMPTY
 * factor -> INT_CONST | FLOAT_CONST | CHAR_CONST
 *         | IDENT | IDENT ( argSeq )
 *         | ( expr )
 * argSeq -> expr , argSeq | expr
 * */


#ifndef _INC_GLOBAL
#include "global.h"
#endif

// Lexical Analysis
int getToken() {
  int lookAhead;
  while (1) {
    lookAhead = fgetc(filePtr);
    // Case 1: space/tab/enter
    if (lookAhead == ' ' || lookAhead == '\t' || lookAhead == '\r') continue;
    else if (lookAhead == '\n') {
      lineSerial++;
      continue;
    } else if (isDigit(lookAhead)) {
    // Case 2: int or float constants
      int isHex = 0;
      if (lookAhead == '0') {
        if ((lookAhead = fgetc(filePtr)) != 'x') {
          if (isDigit(lookAhead)) error("Number can't start with 0.");
          else {
            ungetc(lookAhead,filePtr);
            tokenValue = 0;
            return INT_CONST;
          }
        }
        else {
          isHex = 1;
          lookAhead = fgetc(filePtr);
        }
      }
      float num = 0;
      // Store the integral part
      while (isDigit(lookAhead)) {
        num = num*10 + lookAhead - '0';
        lookAhead = fgetc(filePtr);
      } 
      // Judge if this is an integer or float
      if (lookAhead == '.') {
        if (isHex) error("Invalid constant.");
        float dec = 0;
        lookAhead = fgetc(filePtr);
        // Get the decimal part of the number
        while (isDigit(lookAhead)) {
          dec = dec*10 + lookAhead - '0';
          lookAhead = fgetc(filePtr);
        }
        while (dec > 1.0) dec /= 10.0;
        ungetc(lookAhead, filePtr);
        // This step may cause the loss of precision !
        // But not solved yet
        tokenValue = num + dec;
        return FLOAT_CONST;
      } else if (lookAhead == 'L') {
        tokenValue = (long)num;
        return LONG_CONST;
      } else {
        ungetc(lookAhead,filePtr);
        tokenValue = (int)num;
        return INT_CONST;
      }
    } else if (isAlpha(lookAhead)) {
    // Case 3: Identifiers or Keywords
      int head = bufferPtr;
      // Store name of the identifier into a buffer
      while(isAlpha(lookAhead) || isDigit(lookAhead) || lookAhead == '_') {
        buffer[bufferPtr++] = lookAhead;
        lookAhead = fgetc(filePtr);
        if (bufferPtr >= BUFFERSIZE)
          exception("Buffer overflow.");
      }
      buffer[bufferPtr++] = '\0';
      if (lookAhead != EOF)
        ungetc(lookAhead,filePtr);
      // Store the identifier into the tokenTable
      int tokenPtr = find(buffer+head);
      if (tokenPtr == -1)
        tokenPtr = insert(buffer+head,IDENT);
      // If it turns out that this is a keyword, return its type.
      else if (tokenPtr < KEYWORD_NUM)
        return tokenTable[tokenPtr].type;
      else ;
      tokenValue = tokenPtr;
      return IDENT;
    } else if (lookAhead == '=') {
    // Case 4: Special operators
      if ((lookAhead = fgetc(filePtr)) == '=') return EQUAL;
      else {
        ungetc(lookAhead,filePtr);
        return ASSIGN;
      }
    } else if (lookAhead == '+') {
      if ((lookAhead = fgetc(filePtr)) == '+') return INCRE;
      else {
        ungetc(lookAhead,filePtr);
        return PLUS;
      }
    } else if (lookAhead == '-') {
      if ((lookAhead = fgetc(filePtr)) == '-') return DECRE;
      else if (isDigit(lookAhead)) {
      // Negative Numbers
        float num = 0;
        while (isDigit(lookAhead)) {
          num = num*10 + lookAhead - '0';
          lookAhead = fgetc(filePtr);
        } 
        if (lookAhead == '.') {
          float dec = 0;
          lookAhead = fgetc(filePtr);
          while (isDigit(lookAhead)) {
            dec = dec*10 + lookAhead - '0';
            lookAhead = fgetc(filePtr);
          }
          while (dec > 1.0) dec /= 10.0;
          ungetc(lookAhead, filePtr);
          tokenValue = -(num + dec);
          return FLOAT_CONST;
        } else {
          ungetc(lookAhead,filePtr);
          tokenValue = -((int)num);
          return INT_CONST;
        }
      } else {
        ungetc(lookAhead,filePtr);
        return MINUS;
      }
    } else if (lookAhead == '&') {
      if ((lookAhead = fgetc(filePtr)) == '&') return AND;
      else error("Invalid token.");
    } else if (lookAhead == '|') {
      if ((lookAhead = fgetc(filePtr)) == '|') return OR;
      else error("Invalid token.");
    } else if (lookAhead == '>') {
      if ((lookAhead = fgetc(filePtr)) == '=') return GEQUAL;
      else {
        ungetc(lookAhead,filePtr);
        return GREATER;
      }
    } else if (lookAhead == '<') {
      if ((lookAhead = fgetc(filePtr)) == '=') return LEQUAL;
      else {
        ungetc(lookAhead,filePtr);
        return LESS;
      }
    } else if (lookAhead == '!') {
      if ((lookAhead = fgetc(filePtr)) == '=') return NEQUAL;
      else {
        ungetc(lookAhead,filePtr);
        return NOT;
      }
    } else if (lookAhead == '\'') {
    // Case 5: characters 
      lookAhead = fgetc(filePtr);
      tokenValue = lookAhead;
      if ((lookAhead = fgetc(filePtr)) == '\'') return CHAR_CONST;
      else error("illegal constChar declaration.");
    } else if (lookAhead == '/') {
    // Case 6: Identify comments
    // I identify comments here instead of in preprocess period,
    // because it would be more convenient to report errors.
      lookAhead = fgetc(filePtr);
      if (lookAhead == '/') {
        readLine(filePtr);
        ungetc('\n',filePtr);
      }
      else if (lookAhead == '*') {
        char c = 0;
        while (1) {
          while ((c = fgetc(filePtr)) != '*') 
            if (c == '\n') lineSerial++;
          if (fgetc(filePtr) == '/') break;
        }
      } else error("Syntax error: invalid comment.");
    } else {
    // Other cases
      char buf[128];
      switch (lookAhead) {
        case '*': return MULTI;
        case '/': return DIV;
        case '%': return MOD;
        case ';': return SEMI;
        case ',': return COMMA;
        case ':': return COLON;
        case '(': return LPAREN;
        case ')': return RPAREN;
        case '{': return LBRACE;
        case '}': return RBRACE;
        case '[': return LBRACK;
        case ']': return RBRACK;
        case EOF: return EOF;
        default: 
          sprintf(buf,"unregonized character: %d",lookAhead);
          exception(buf);
      }
    }
  }
}


// Grammatical Analysis

void match(int token) {
  if (tokenPtr == token) {
    tokenPtr = getToken();
    // emit(token, tokenValue);
  }
  else error("Syntax error: token doesn't match the grammar."); 
}

link parse() {
  tokenPtr = getToken();
  link ptr = mkNode(PROGREM,mkNumProp(0));
  while(tokenPtr != EOF) {
    addChild(ptr,externalDef());
  }
  return ptr;
}


/* Structure of Function Definition: 
 * - (FUNCDEF,0)
 *    + (PARAMLIST,0)
 *      * (PARAM,Type) * N
 *        - (VAR,name)
 *    + Compstate
 *    + (RETURNTYPE,Type)
 * Structure of Varaiable Definition:
 * - (VARDEF,0)
 *    + (Type,0)
 *    + (VARSEQ,name) / (VAR,name) * N
 *    For varseq: * (INT_CONST,size)
 *                * [ (Type,value) ]
 *    For var:    * [ (Type,value) ] 
 * */

link externalDef() {
  if (isVar(tokenPtr) || tokenPtr == VOID) {
    int Type = tokenPtr;
    match(tokenPtr);
    char* name = tokenTable[(int)tokenValue].content;
    match(IDENT);
    if (tokenPtr == LPAREN) {
    // funcDef
      link ptr = mkNode(FUNCDEF,mkStrProp(name));
      link paramList = mkNode(PARAMLIST,mkNumProp(0));
      link returnType = mkNode(RETURNTYPE,mkNumProp(Type));
      match(LPAREN);
      while (tokenPtr != RPAREN) {
        if (isVar(tokenPtr)) {
          link param = mkNode(PARAM, mkNumProp(tokenPtr));
          match(tokenPtr);
          addChild(param, mkNode(VAR, mkStrProp(tokenTable[(int)tokenValue].content)));
          match(IDENT);
          addChild(paramList, param);
          if (tokenPtr != RPAREN) match(COMMA);
        } else error("Syntax error: invalid type for parameters.");
      }
      match(RPAREN);
      addChild(ptr, paramList);
      addChild(ptr, compState());
      addChild(ptr, returnType);
      return ptr;
    } else {
    // globalVarDef
      link ptr = mkNode(VARDEF,mkNumProp(0));
      addChild(ptr, mkNode(Type,mkNumProp(0)));
      // Arrays
      link ident = mkNode(VAR,mkStrProp(name));
      if (tokenPtr == LBRACK) {
        match(LBRACK);
        ident->type = VARSEQ;
        if (tokenPtr == INT_CONST)
          addChild(ident, mkNode(INT_CONST,mkNumProp(tokenValue)));
        else error("size of array must be int constant.");
        match(tokenPtr);
        match(RBRACK);
      }
      addChild(ptr, ident); 
      // Initial value
      if (tokenPtr == ASSIGN) {
        match(ASSIGN);
        link value = mkNode(Type,mkNumProp(tokenValue));
        if (isConst(tokenPtr))
          match(tokenPtr);
        else error("Syntax error: initial value for global variables must be constant.");
        addChild(ident, value);
      }
      // Multi declarations
      while (tokenPtr == COMMA) {
        match(COMMA);
        link ident = mkNode(VAR,mkStrProp(tokenTable[(int)tokenValue].content));
        match(IDENT);
        if (tokenPtr == LBRACK) {
          match(LBRACK);
          ident->type = VARSEQ;
          if (tokenPtr == INT_CONST)
            addChild(ident, mkNode(INT_CONST,mkNumProp(tokenValue)));
          else error("size of array must be int constant.");
          match(tokenPtr);
          match(RBRACK);
        }
        addChild(ptr, ident); 
        if (tokenPtr == ASSIGN) {
          match(ASSIGN);
          link value = mkNode(tokenPtr,mkNumProp(tokenValue));
          if (isConst(tokenPtr))
            match(tokenPtr);
          else error("Syntax error: initial value for global variables must be constant.");
          addChild(ident, value);
        }
      }
      match(SEMI);
      return ptr;
    }
  } else error("Syntax error: external definition must start with a valid datatype.");
}

link compState() {
  link ptr = mkNode(COMPSTATE,mkNumProp(0));
  match(LBRACE);
  while (tokenPtr != RBRACE) {
    addChild(ptr,statement());
  } 
  match(RBRACE);
  return ptr;
}

link statement() {
  if (isVar(tokenPtr)) {
  // localVarDef
    link ptr = mkNode(VARDEF,mkNumProp(0));
    int Type = tokenPtr;
    match(tokenPtr);
    char* name = tokenTable[(int)tokenValue].content;
    match(IDENT);
    addChild(ptr, mkNode(Type,mkNumProp(0)));
    if (tokenPtr == LBRACK) {
      match(LBRACK);
      addChild(ptr, mkNode(VARSEQ,mkStrProp(name)));
      if (tokenPtr == INT_CONST)
        addChild(ptr->head->next->location, mkNode(INT_CONST,mkNumProp(tokenValue)));
      else error("size of array must be int constant.");
      match(tokenPtr);
      match(RBRACK);
    } else addChild(ptr, mkNode(VAR,mkStrProp(name))); 
    if (tokenPtr == ASSIGN) {
      match(ASSIGN);
      link value = mkNode(Type,mkNumProp(tokenValue));
      if (isConst(tokenPtr))
        match(tokenPtr);
      else error("Syntax error: initial value for local variables must be constant.");
      addChild((ptr->head->next->location), value);
    }
    while (tokenPtr == COMMA) {
      match(COMMA);
      char* identName = tokenTable[(int)tokenValue].content;
      link ident = mkNode(VAR,mkStrProp(identName));
      match(IDENT);
      if (tokenPtr == LBRACK) {
        match(LBRACK);
        ident->type = VARSEQ;
        if (tokenPtr == INT_CONST)
          addChild(ident, mkNode(INT_CONST,mkNumProp(tokenValue)));
        else error("size of array must be int constant.");
        match(tokenPtr);
        match(RBRACK);
      }
      addChild(ptr, ident); 
      if (tokenPtr == ASSIGN) {
        match(ASSIGN);
        link value = mkNode(tokenPtr,mkNumProp(tokenValue));
        if (isConst(tokenPtr))
          match(tokenPtr);
        else error("Syntax error: initial value for local variables must be constant.");
        addChild(ident, value);
      }
    }
    match(SEMI);
    return ptr;
  // Statement
  } else return stat();
}

link stat() {
  link ptr;
  switch(tokenPtr) {
    case RETURN:
      ptr = mkNode(RETURN,mkNumProp(0));
      match(RETURN);
      addChild(ptr, expr());
      match(SEMI);
      return ptr;
    case IF:
      match(IF);
      match(LPAREN);
      link condition = expr();
      match(RPAREN);
      link ifexpr = compStateOrStat();
      if (tokenPtr == ELSE) {
        match(ELSE);
        ptr = mkNode(IF,mkStrProp("IF_ELSE"));
        addChild(ptr,condition);
        addChild(ptr,ifexpr);
        addChild(ptr,compStateOrStat());
        return ptr;
      } else {
        ptr = mkNode(IF,mkStrProp("IF"));
        addChild(ptr,condition);
        addChild(ptr,ifexpr);
        return ptr;
      }
    case WHILE:
      ptr = mkNode(WHILE,mkNumProp(0));
      match(WHILE);
      match(LPAREN);
      addChild(ptr,expr());
      match(RPAREN);
      addChild(ptr,compStateOrStat());
      return ptr;
    case FOR:
      ptr = mkNode(FOR,mkNumProp(0));
      match(FOR);
      match(LPAREN);
      addChild(ptr,statement());
      addChild(ptr,expr());
      match(SEMI);
      addChild(ptr,expr());
      match(RPAREN);
      addChild(ptr,compStateOrStat());
      return ptr;
    case BREAK:
      ptr = mkNode(BREAK,mkNumProp(0));
      match(BREAK);
      match(SEMI);
      return ptr;
    case CONTINUE:
      ptr = mkNode(CONTINUE,mkNumProp(0));
      match(CONTINUE);
      match(SEMI);
      return ptr;
    default:
      ptr = expr();
      match(SEMI);
      return ptr;
  }
}

link compStateOrStat() {
  if (tokenPtr == LBRACE) return compState();
  else return stat();
}

link expr() {
  link oper1 = priorN2();
  while (1) {
    if (tokenPtr == ASSIGN) {
      match(tokenPtr);
      link oper2 = priorN2();
      link ptr = mkNode(ASSIGN,mkNumProp(0));
      addChild(ptr,oper1);
      addChild(ptr,oper2);
      oper1 = ptr;
    } else return oper1;
  }
}

link priorN2() {
  link oper1 = priorN1();
  while (1) {
    if (tokenPtr == OR) {
      match(tokenPtr);
      link oper2 = priorN1();
      link ptr = mkNode(OR,mkNumProp(0));
      addChild(ptr,oper1);
      addChild(ptr,oper2);
      oper1 = ptr;
    } else return oper1;
  }
}

link priorN1() {
  link oper1 = prior0();
  while (1) {
    if (tokenPtr == AND) {
      match(tokenPtr);
      link oper2 = prior0();
      link ptr = mkNode(AND,mkNumProp(0));
      addChild(ptr,oper1);
      addChild(ptr,oper2);
      oper1 = ptr;
    } else return oper1;
  }
}

link prior0() {
  link oper1 = prior1();
  while (1) {
    if (tokenPtr == EQUAL || tokenPtr == NEQUAL) {
      int Type = tokenPtr;
      match(tokenPtr);
      link oper2 = prior1();
      link ptr = mkNode(Type,mkNumProp(0));
      addChild(ptr,oper1);
      addChild(ptr,oper2);
      oper1 = ptr;
    } else return oper1;
  }
}

link prior1() {
  link oper1 = prior2();
  while (1) {
    if (tokenPtr == GREATER || tokenPtr == LESS || tokenPtr == GEQUAL || tokenPtr == LEQUAL) {
      int Type = tokenPtr;
      match(tokenPtr);
      link oper2 = prior2();
      link ptr = mkNode(Type,mkNumProp(0));
      addChild(ptr,oper1);
      addChild(ptr,oper2);
      oper1 = ptr;
    } else return oper1;
  }
}

link prior2() {
  link oper1 = prior3();
  while (1) {
    if (tokenPtr == PLUS || tokenPtr == MINUS) {
      int Type = tokenPtr;
      match(tokenPtr);
      link oper2 = prior3();
      link ptr = mkNode(Type,mkNumProp(0));
      addChild(ptr,oper1);
      addChild(ptr,oper2);
      oper1 = ptr;
    } else return oper1;
  }
}

link prior3() {
  link oper1 = prior4();
  while (1) {
    if (tokenPtr == MULTI || tokenPtr == DIV || tokenPtr == MOD) {
      int Type = tokenPtr;
      match(tokenPtr);
      link oper2 = prior4();
      link ptr = mkNode(Type,mkNumProp(0));
      addChild(ptr,oper1);
      addChild(ptr,oper2);
      oper1 = ptr;
    } else return oper1;
  }
}

link prior4() {
  if (tokenPtr == NOT) {
    match(NOT);
    link ptr = mkNode(tokenPtr,mkNumProp(0));
    addChild(ptr,factor());
    return ptr;
  } else return factor();
}

link factor() {
  link ptr;
  switch(tokenPtr) {
    case INT_CONST: case FLOAT_CONST: case CHAR_CONST:
    case LONG_CONST: case SHORT_CONST: case DOUBLE_CONST:
      ptr = mkNode(tokenPtr,mkNumProp(tokenValue));
      match(tokenPtr);
      return ptr;
    case IDENT:
      ptr = mkNode(VAR,mkStrProp(tokenTable[(int)tokenValue].content));
      match(IDENT);
      if (tokenPtr == LPAREN) {
        ptr->type = FUNC;
        match(LPAREN);
        addChild(ptr,argSeq());
        match(RPAREN);
      }
      return ptr;
    case LPAREN:
      match(LPAREN);
      ptr = expr();
      match(RPAREN);
      return ptr;
    default: return 0;
  }
}

link argSeq() {
  link ptr = mkNode(ARGSEQ,mkNumProp(0));
  addChild(ptr,expr());
  while (tokenPtr == COMMA) {
    match(COMMA);
    addChild(ptr,expr());
  }
  return ptr;
}
