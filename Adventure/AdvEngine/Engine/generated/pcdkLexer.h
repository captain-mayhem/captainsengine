/** \file
 *  This C header file was generated by $ANTLR version 3.1.2
 *
 *     -  From the grammar source file : pcdk.g
 *     -                            On : 2016-05-13 22:43:08
 *     -                 for the lexer : pcdkLexerLexer *
 * Editing it, at least manually, is not wise. 
 *
 * C language generator and runtime by Jim Idle, jimi|hereisanat|idle|dotgoeshere|ws.
 *
 *
 * The lexer pcdkLexer has the callable functions (rules) shown below,
 * which will invoke the code for the associated rule in the source grammar
 * assuming that the input stream is pointing to a token/text stream that could begin
 * this rule.
 * 
 * For instance if you call the first (topmost) rule in a parser grammar, you will
 * get the results of a full parse, but calling a rule half way through the grammar will
 * allow you to pass part of a full token stream to the parser, such as for syntax checking
 * in editors and so on.
 *
 * The parser entry points are called indirectly (by function pointer to function) via
 * a parser context typedef ppcdkLexer, which is returned from a call to pcdkLexerNew().
 *
 * As this is a generated lexer, it is unlikely you will call it 'manually'. However
 * the methods are provided anyway.
 * * The methods in ppcdkLexer are  as follows:
 *
 *  -  void      ppcdkLexer->T__36(ppcdkLexer)
 *  -  void      ppcdkLexer->COMMENT(ppcdkLexer)
 *  -  void      ppcdkLexer->LPAREN(ppcdkLexer)
 *  -  void      ppcdkLexer->RPAREN(ppcdkLexer)
 *  -  void      ppcdkLexer->LBRACE(ppcdkLexer)
 *  -  void      ppcdkLexer->RBRACE(ppcdkLexer)
 *  -  void      ppcdkLexer->LBRACKET(ppcdkLexer)
 *  -  void      ppcdkLexer->RBRACKET(ppcdkLexer)
 *  -  void      ppcdkLexer->SEMICOLON(ppcdkLexer)
 *  -  void      ppcdkLexer->COMMA(ppcdkLexer)
 *  -  void      ppcdkLexer->UNDERSCORE(ppcdkLexer)
 *  -  void      ppcdkLexer->PLUS(ppcdkLexer)
 *  -  void      ppcdkLexer->MINUS(ppcdkLexer)
 *  -  void      ppcdkLexer->TIMES(ppcdkLexer)
 *  -  void      ppcdkLexer->DIVIDE(ppcdkLexer)
 *  -  void      ppcdkLexer->IDIV(ppcdkLexer)
 *  -  void      ppcdkLexer->GREATER(ppcdkLexer)
 *  -  void      ppcdkLexer->LESS(ppcdkLexer)
 *  -  void      ppcdkLexer->INFO_BEG(ppcdkLexer)
 *  -  void      ppcdkLexer->INFO_END(ppcdkLexer)
 *  -  void      ppcdkLexer->ON(ppcdkLexer)
 *  -  void      ppcdkLexer->IF(ppcdkLexer)
 *  -  void      ppcdkLexer->IFNOT(ppcdkLexer)
 *  -  void      ppcdkLexer->LEVEL(ppcdkLexer)
 *  -  void      ppcdkLexer->ROW(ppcdkLexer)
 *  -  void      ppcdkLexer->TIMER(ppcdkLexer)
 *  -  void      ppcdkLexer->INT(ppcdkLexer)
 *  -  void      ppcdkLexer->REAL(ppcdkLexer)
 *  -  void      ppcdkLexer->REAL_INT(ppcdkLexer)
 *  -  void      ppcdkLexer->IDENT_FRAG(ppcdkLexer)
 *  -  void      ppcdkLexer->IDENT_PART(ppcdkLexer)
 *  -  void      ppcdkLexer->NEWLINE(ppcdkLexer)
 *  -  void      ppcdkLexer->WS(ppcdkLexer)
 *  -  void      ppcdkLexer->Tokens(ppcdkLexer)
 *
 * The return type for any particular rule is of course determined by the source
 * grammar file.
 */
// [The "BSD licence"]
// Copyright (c) 2005-2009 Jim Idle, Temporal Wave LLC
// http://www.temporal-wave.com
// http://www.linkedin.com/in/jimidle
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
// 1. Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
// 2. Redistributions in binary form must reproduce the above copyright
//    notice, this list of conditions and the following disclaimer in the
//    documentation and/or other materials provided with the distribution.
// 3. The name of the author may not be used to endorse or promote products
//    derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef	_pcdkLexer_H
#define _pcdkLexer_H
/* =============================================================================
 * Standard antlr3 C runtime definitions
 */
#include    <antlr3.h>

/* End of standard antlr 3 runtime definitions
 * =============================================================================
 */
 
#ifdef __cplusplus
extern "C" {
#endif

// Forward declare the context typedef so that we can use it before it is
// properly defined. Delegators and delegates (from import statements) are
// interdependent and their context structures contain pointers to each other
// C only allows such things to be declared if you pre-declare the typedef.
//
typedef struct pcdkLexer_Ctx_struct pcdkLexer, * ppcdkLexer;



#ifdef	ANTLR3_WINDOWS
// Disable: Unreferenced parameter,							- Rules with parameters that are not used
//          constant conditional,							- ANTLR realizes that a prediction is always true (synpred usually)
//          initialized but unused variable					- tree rewrite variables declared but not needed
//          Unreferenced local variable						- lexer rule declares but does not always use _type
//          potentially unitialized variable used			- retval always returned from a rule 
//			unreferenced local function has been removed	- susually getTokenNames or freeScope, they can go without warnigns
//
// These are only really displayed at warning level /W4 but that is the code ideal I am aiming at
// and the codegen must generate some of these warnings by necessity, apart from 4100, which is
// usually generated when a parser rule is given a parameter that it does not use. Mostly though
// this is a matter of orthogonality hence I disable that one.
//
#pragma warning( disable : 4100 )
#pragma warning( disable : 4101 )
#pragma warning( disable : 4127 )
#pragma warning( disable : 4189 )
#pragma warning( disable : 4505 )
#pragma warning( disable : 4701 )
#endif

/** Context tracking structure for pcdkLexer
 */
struct pcdkLexer_Ctx_struct
{
    /** Built in ANTLR3 context tracker contains all the generic elements
     *  required for context tracking.
     */
    pANTLR3_LEXER    pLexer;


     void (*mT__36)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mCOMMENT)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mLPAREN)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mRPAREN)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mLBRACE)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mRBRACE)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mLBRACKET)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mRBRACKET)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mSEMICOLON)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mCOMMA)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mUNDERSCORE)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mPLUS)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mMINUS)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mTIMES)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mDIVIDE)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mIDIV)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mGREATER)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mLESS)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mINFO_BEG)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mINFO_END)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mON)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mIF)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mIFNOT)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mLEVEL)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mROW)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mTIMER)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mINT)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mREAL)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mREAL_INT)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mIDENT_FRAG)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mIDENT_PART)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mNEWLINE)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mWS)	(struct pcdkLexer_Ctx_struct * ctx);
     void (*mTokens)	(struct pcdkLexer_Ctx_struct * ctx);    const char * (*getGrammarFileName)();
    void	    (*free)   (struct pcdkLexer_Ctx_struct * ctx);
        
};

// Function protoypes for the constructor functions that external translation units
// such as delegators and delegates may wish to call.
//
ANTLR3_API ppcdkLexer pcdkLexerNew         (pANTLR3_INPUT_STREAM instream);
ANTLR3_API ppcdkLexer pcdkLexerNewSSD      (pANTLR3_INPUT_STREAM instream, pANTLR3_RECOGNIZER_SHARED_STATE state);

/** Symbolic definitions of all the tokens that the lexer will work with.
 * \{
 *
 * Antlr will define EOF, but we can't use that as it it is too common in
 * in C header files and that would be confusing. There is no way to filter this out at the moment
 * so we just undef it here for now. That isn't the value we get back from C recognizers
 * anyway. We are looking for ANTLR3_TOKEN_EOF.
 */
#ifdef	EOF
#undef	EOF
#endif
#ifdef	Tokens
#undef	Tokens
#endif 
#define UNDERSCORE      31
#define INFO_BEG      16
#define INT      9
#define COMMENT      32
#define MINUS      23
#define DIVIDE      26
#define IFNOT      13
#define SEMICOLON      6
#define REAL_INT      29
#define T__36      36
#define WS      35
#define EOF      -1
#define IF      12
#define ON      7
#define COMMA      27
#define TIMES      25
#define NEWLINE      34
#define TIMER      11
#define LPAREN      4
#define GREATER      20
#define LBRACKET      18
#define IDENT_FRAG      33
#define RBRACKET      21
#define IDIV      22
#define RPAREN      5
#define LEVEL      8
#define IDENT_PART      30
#define RBRACE      14
#define REAL      28
#define LESS      19
#define ROW      10
#define INFO_END      17
#define LBRACE      15
#define PLUS      24
#ifdef	EOF
#undef	EOF
#define	EOF	ANTLR3_TOKEN_EOF
#endif

#ifndef TOKENSOURCE
#define TOKENSOURCE(lxr) lxr->pLexer->rec->state->tokSource
#endif

/* End of token definitions for pcdkLexer
 * =============================================================================
 */
/** \} */

#ifdef __cplusplus
}
#endif

#endif

/* END - Note:Keep extra line feed to satisfy UNIX systems */
