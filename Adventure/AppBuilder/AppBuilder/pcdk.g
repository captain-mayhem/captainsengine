grammar pcdk;

options{
	language=C;
	output=AST;
}

@parser::includes{
#include "AST.h"
}

prog returns [NodeList* nodes]
	:	{$nodes = new NodeList();}
		(
		statement=stmt {$nodes->addNode(statement.stmt);}
		)*;

stmt returns [StmtNode* stmt]
	:	event_handler
	|	ns=nested_stmt {$stmt = ns.stmt;}
;
	
nested_stmt returns [StmtNode* stmt]
	:	fc=func_call {$stmt = fc.func;}
	|	conditional;

func_call returns [FuncCallNode* func]
	:	IDENT LPAREN args=arg_list RPAREN 
	{
		std::string fname = std::string((char*)$IDENT.text->chars);
		$func = new FuncCallNode(fname,args.nodes);
	}
;
	
event_handler
	:	ON LPAREN IDENT RPAREN block;
	
conditional
	:	IF IDENT LPAREN arg_list RPAREN block
;
	
block	:	'{' (nested_stmt)* '}'
	|	nested_stmt;
	
arg_list returns [NodeList* nodes]
	:	node=arg {$nodes = new NodeList(); $nodes->addNode(node.value);}
		(SEMICOLON 
		node=arg {$nodes->addNode(node.value);}
		)*
;

arg	returns [ASTNode* value]
	:	IDENT {$value = new IdentNode((char*)$IDENT.text->chars);}
	|	INT {$value = new IntNode(atoi((char*)$INT.text->chars));}
;


LPAREN	:	'(';
RPAREN	:	')';
SEMICOLON
	:	';';
ON	:	('O'|'o')('N'|'n');
IF	:	('I'|'i')('F'|'f')'_';
INT	:	'0'..'9'+;
IDENT	:	('a'..'z'|'A'..'Z')('a'..'z'|'A'..'Z'|'0'..'9')*;
NEWLINE	:	('\r'|'\n')+ {$channel=HIDDEN;}
	;
WS	:	(' '|'\t')+ {$channel=HIDDEN;}
	;
