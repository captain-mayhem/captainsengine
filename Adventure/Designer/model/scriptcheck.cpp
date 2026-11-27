#include "scriptcheck.h"

#include <cstdlib>

//Lua is built as C
extern "C"{
#include <lua.h>
#include <lauxlib.h>
}

#include <pcdkLexer.h>
#include <pcdkParser.h>

// The game parses a script in PcdkScript::parseProgram (Script.cpp); this
// runs the same parsers without an engine, and keeps the errors that the game
// only logs.

namespace sd{

namespace{

//! the errors of the parse running; the recognizers' callbacks have no context
thread_local std::vector<ScriptError>* errors = nullptr;

std::string tokenText(pANTLR3_COMMON_TOKEN token){
  if (token == nullptr)
    return std::string();
  if (token->type == ANTLR3_TOKEN_EOF)
    return std::string();
  pANTLR3_STRING str = token->getText(token);
  if (str == nullptr || str->chars == nullptr)
    return std::string();
  return std::string((const char*)str->chars, str->len);
}

void reportError(pANTLR3_BASE_RECOGNIZER recognizer){
  //as in the game: one error until the parser has recovered
  if (recognizer->state->errorRecovery == ANTLR3_TRUE)
    return;
  recognizer->state->errorRecovery = ANTLR3_TRUE;
  recognizer->state->errorCount++;
  pANTLR3_EXCEPTION ex = recognizer->state->exception;
  if (ex == nullptr || errors == nullptr)
    return;
  ScriptError err;
  err.line = (int)ex->line;
  err.column = (int)ex->charPositionInLine;
  if (recognizer->type == ANTLR3_TYPE_LEXER){
    unsigned char c = (unsigned char)ex->c;
    err.message = c >= 32 && c != 0xFF ? std::string("unexpected character '") + (char)c + "'" : "unexpected character";
  }
  else{
    std::string text = tokenText((pANTLR3_COMMON_TOKEN)ex->token);
    if (text.empty()){
      err.message = "unexpected end of the script";
      //placed on the last line with text afterwards
      err.line = 0;
    }
    else
      err.message = "unexpected '" + text + "'";
  }
  errors->push_back(err);
}

void* recoverFromMismatchedToken(pANTLR3_BASE_RECOGNIZER recognizer, ANTLR3_UINT32 ttype, pANTLR3_BITSET_LIST follow){
  recognizer->mismatch(recognizer, ttype, follow);
  return nullptr;
}

std::vector<ScriptError> checkPcdk(const std::string& text){
  std::vector<ScriptError> result;
  if (text.empty())
    return result;
  errors = &result;
  //the stream reads the bytes in place without changing them
  pANTLR3_INPUT_STREAM input = antlr3NewAsciiStringInPlaceStream((pANTLR3_UINT8)text.c_str(), (ANTLR3_UINT32)text.size(), (pANTLR3_UINT8)"Script");
  ppcdkLexer lexer = pcdkLexerNew(input);
  lexer->pLexer->rec->reportError = reportError;
  pANTLR3_COMMON_TOKEN_STREAM tokens = antlr3CommonTokenStreamSourceNew(ANTLR3_SIZE_HINT, TOKENSOURCE(lexer));
  ppcdkParser parser = pcdkParserNew(tokens);
  parser->pParser->super = parser;
  parser->pParser->rec->reportError = reportError;
  parser->pParser->rec->recoverFromMismatchedToken = recoverFromMismatchedToken;
  pcdkParser_prog_return ast = parser->prog(parser);
  delete ast.nodes;
  //the engine's allocation header defines free for the debug heap on Windows
#undef free
  parser->free(parser);
  tokens->free(tokens);
  lexer->free(lexer);
  input->free(input);
  errors = nullptr;
  //an error without a position (the end of the script) goes to the last line with text
  int lines = 1;
  int lastText = 1;
  for (char c : text){
    if (c == '\n')
      ++lines;
    else if (c != '\r' && c != ' ' && c != '\t')
      lastText = lines;
  }
  for (ScriptError& err : result){
    if (err.line <= 0)
      err.line = lastText;
  }
  return result;
}

std::vector<ScriptError> checkLua(const std::string& text){
  std::vector<ScriptError> result;
  lua_State* L = luaL_newstate();
  if (L == nullptr)
    return result;
  if (luaL_loadbuffer(L, text.data(), text.size(), "=script") != LUA_OK){
    //"script:<line>: <message>"
    std::string msg = lua_tostring(L, -1);
    ScriptError err;
    err.line = 1;
    err.message = msg;
    size_t colon = msg.find(':');
    if (colon != std::string::npos){
      size_t colon2 = msg.find(':', colon+1);
      if (colon2 != std::string::npos){
        err.line = std::atoi(msg.substr(colon+1, colon2-colon-1).c_str());
        err.message = msg.substr(colon2+1);
        if (!err.message.empty() && err.message[0] == ' ')
          err.message.erase(0, 1);
      }
    }
    result.push_back(err);
  }
  lua_close(L);
  return result;
}

}

std::vector<ScriptError> checkScript(const std::string& text, ScriptLang language){
  if (language == ScriptLang::Lua)
    return checkLua(text);
  return checkPcdk(text);
}

}
