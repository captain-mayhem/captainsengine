#include "Script.h"

#include <string>

PcdkScript::PcdkScript(){

}

PcdkScript::~PcdkScript(){

}

void PcdkScript::parseProgram(){
  pANTLR3_INPUT_STREAM input;
  ppcdkLexer lexer;
  pANTLR3_COMMON_TOKEN_STREAM tokStream;
  ppcdkParser parser;
  pcdkParser_prog_return langAST;

  std::string prog("a(2)\n b(abc;6)");
  input = antlr3NewAsciiStringInPlaceStream(((pANTLR3_UINT8)prog.c_str()), (ANTLR3_UINT32)prog.size(), (pANTLR3_UINT8)"Script");
  lexer = pcdkLexerNew(input);
  tokStream = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
  parser = pcdkParserNew(tokStream);
  langAST = parser->prog(parser);
  //if (parser->pParser->rec->error)
  parser->free(parser);
  tokStream->free(tokStream);
  lexer->free(lexer);
  input->free(input);
}