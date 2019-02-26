// 
// preprocess.c
// 
// The preprocess part of the program.
// 


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
    if (ptr == '\n') {
      lineSerial++;
      fputc(ptr,output);
      continue;
    } else if (ptr == '#') {
      char* ident = readUntilSpace(input);
      // Identify macro definitions
      if (strCmp("include",ident) == 0) {
        int nameLen = 0;
        ifClean = 0;
        // Include instructions
        readUntilChar(input);
        // Get include paths
        ptr = fgetc(input);
        if (ptr == '<') {
          // Case 1: #include <xxx>
          char* path = getenv("C_INCLUDE_PATH");
          if (!path) error("Include error. Can not find target header, please update your environment variables.");
          char* p = strCpy(filenameBuffer + filenamePtr, path);
          while ((ptr = fgetc(input)) != '>') {
            if (ptr == '\n') error("Include error: wrong syntax.");
            *p = ptr;
            p++;
            nameLen++;
          }
          *p = 0;
        } else if (ptr == '\"') {
          // Case 2: #include "xxx"
          char *p = filenameBuffer + filenamePtr;
          while ((ptr = fgetc(input)) != '\"') {
            if (ptr == '\n') error("Include error: wrong syntax.");
            *p = ptr;
            p++;
            nameLen++;
          }
          *p = 0;
        } else error("Include path should be between either <> or \"\".");
        // Copy source file's content to target file.
        FILE* source = fopen(filenameBuffer + filenamePtr,"r");
        if (!source) error("Include error. Cannot find target file.");
        int length = copyFile(source,output);
        addRange(filenameBuffer + filenamePtr, lineSerial, lineSerial + length);
        filenamePtr += nameLen;
      } else if (strCmp("define",ident) == 0) {
      // Store defined identifiers
        readUntilChar(input);
        if ((ptr = getToken()) == IDENT) {
          defineIndex[definePtr] = (int)tokenValue;
          readUntilChar(input);
          strIndex[definePtr++] = defineBuffer + defineBufferPtr;
          while ((ptr = fgetc(input)) != '\n' && ptr != '\r' && ptr != ' ')
            defineBuffer[defineBufferPtr++] = ptr;
          if (ptr == '\n') {
            lineSerial++;
            ungetc('\n',input);
          }
          defineBuffer[defineBufferPtr++] = 0;
        } else error("Syntax error: define must be followed by identifiers.");
      } else error("Cannot identify target macro instruction.");
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
  // Reset the line serial.
  lineSerial = 1;
  fclose(input);
  fclose(output);
}
