
#ifndef _INC_GLOBAL
#include "global.h"
#endif

void preProcess(const char* fileName, const char* tempFile) {
  FILE* input = fopen(fileName,"r");
  if (!input) exception("Cannot open target file.");
  filePtr = input;
  FILE* output = fopen(tempFile,"w");
  int ptr = 0;
  while((ptr = fgetc(input)) != EOF) {
    if (ptr == '#') {
      char* ident = readUntilSpace(input);
      // Identify macro definitions
      if (strCmp("include",ident) == 0) {
        ifClean = 0;
        // Include instructions
        char tempBuffer[BUFFERSIZE] = {0};
        readUntilChar(input);
        // Get include paths
        ptr = fgetc(input);
        if (ptr == '<') {
          // Case 1: #include <xxx>
          char* path = getenv("C_INCLUDE_PATH");
          if (!path) path = "/usr/include/";
          char* p = strCpy(tempBuffer, path);
          while ((ptr = fgetc(input)) != '>') {
            *p = ptr;
            p++;
          }
          *p = 0;
        } else if (ptr == '\"') {
          // Case 2: #include "xxx"
          char *p = tempBuffer;
          while ((ptr = fgetc(input)) != '\"') {
            *p = ptr;
            p++;
          }
        } else error("Include path should be between either <> or \"\".");
        // Copy source file's content to target file.
        FILE* source = fopen(tempBuffer,"r");
        if (!source) error("Include error. Cannot find target file.");
        copyFile(source,output);
      } else if (strCmp("define",ident) == 0) {
      // Store defined identifiers
        readUntilChar(input);
        if ((ptr = getToken()) == IDENT) {
          defineIndex[definePtr] = (int)tokenValue;
          readUntilChar(input);
          strIndex[definePtr++] = defineBuffer + defineBufferPtr;
          while ((ptr = fgetc(input)) != '\n' && ptr != '\r' && ptr != ' ')
            defineBuffer[defineBufferPtr++] = ptr;
          defineBuffer[defineBufferPtr++] = 0;
        } else error("Syntax error: define must be followed by identifiers.");
      } else error("Cannot identify target macro instruction.");
    } else if (ptr == '/') {
    // Identify comments
      ptr = fgetc(input);
      if (ptr == '/') {
        readLine(input);
        ungetc('\n',input);
      }
      else if (ptr == '*') {
        while (1) {
          while (fgetc(input) != '*') ;
          if (fgetc(input) == '/') break;
        }
      } else error("Syntax error: invalid comment.");
    } else if (isAlpha(ptr) || ptr == '_') {
    // Replace defined identifiers
      ungetc(ptr, input);
      ptr = getToken();
      if (ptr != IDENT) {
        int flag = 0;
        for (int i=0; i<symPtr; i++)
          if (ptr == tokenTable[i].type) {
            fprintf(output,"%s",tokenTable[i].content);
            flag = 1;
          }
        if (!flag) exception("Identifier doesn't start with alpha or _.");
        else continue;
      }
      char* ident = tokenTable[(int)tokenValue].content;
      int defined = 0;
      for (int i=0; i<definePtr; i++)
        if (strCmp(tokenTable[defineIndex[i]].content,ident) == 0) {
          fprintf(output,"%s",strIndex[i]);
          defined = 1;
          break;
        }
      if (!defined)
        fprintf(output,"%s",ident);
    } else fputc(ptr,output);
  }
  fclose(input);
  fclose(output);
}
