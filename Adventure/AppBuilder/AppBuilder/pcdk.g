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
	:	evth=event_handler {$stmt = evth.evt;}
	|	ns=nested_stmt {$stmt = ns.stmt;}
;
	
nested_stmt returns [StmtNode* stmt]
	:	cond=conditional {$stmt = cond.cond;}
	|	fc=func_call {$stmt = fc.func;}
	;

func_call returns [FuncCallNode* func]
	:	IDENT LPAREN args=arg_list RPAREN 
	{
		std::string fname = std::string((char*)$IDENT.text->chars);
		$func = new FuncCallNode(fname,args.nodes);
	}
;
	
event_handler returns [EventNode* evt]
	:	ON LPAREN IDENT RPAREN evtblock=block
	{
		std::string eventname = std::string((char*)$IDENT.text->chars);
		$evt = new EventNode(eventname, evtblock.nodes);
	}
	;
	
conditional returns [CondNode* cond]
	:	
		(IF {$cond = new CondNode(false);}
		| 
		IFNOT {$cond = new CondNode(true);}
		) 
		IDENT LPAREN args=arg_list RPAREN ifblock=block
	{
		$cond->setCondition(std::string((char*)$IDENT.text->chars));
		$cond->setCondArgs(args.nodes);
		$cond->setIfBlock(ifblock.nodes);
	}
	;
	
block returns [NodeList* nodes]
	:	{$nodes = new NodeList();}
		LBRACE
		(
			stmtnode=nested_stmt {$nodes->addNode(stmtnode.stmt);}
		)*
		RBRACE
	|	stmtnode=nested_stmt {$nodes = new NodeList(); $nodes->addNode(stmtnode.stmt);}
	;
	
arg_list returns [NodeList* nodes]
	:	node=arg {$nodes = new NodeList(); $nodes->addNode(node.value);}
		(SEMICOLON 
		node=arg {$nodes->addNode(node.value);}
		)*
	|	{$nodes = new NodeList();}
;

arg	returns [ASTNode* value]
	: {$value = new IdentNode("");}
	(INFO_BEG INT INFO_END )? 
	(
		IDENT {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$IDENT.text->chars);}
		| ON {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$ON.text->chars);}
		| IF {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$IF.text->chars);}
	)+
	/*|	IDENT  {$value = new IdentNode((char*)$IDENT.text->chars);}*/
	/*|	STRING {$value = new IdentNode((char*)$STRING.text->chars);}*/
	|	INT {$value = new IntNode(atoi((char*)$INT.text->chars));}
;


LPAREN	:	'(';
RPAREN	:	')';
LBRACE  :	'{';
RBRACE	:	'}';
SEMICOLON
	:	';';
UNDERSCORE
	:	'_';
INFO_BEG	:	'|''#';
INFO_END	:	'#''|';
ON	:	('O'|'o')('N'|'n');
IF	:	('I'|'i')('F'|'f')UNDERSCORE;
IFNOT   :	('I'|'i')('F'|'f')('N'|'n')('O'|'o')('T'|'t')UNDERSCORE;
INT	:	'0'..'9'+;
IDENT	:	('a'..'z'|'A'..'Z')('a'..'z'|'A'..'Z'|'0'..'9')*;
/*STRING	:*/	/*WS*('a'..'z'|'A'..'Z')('a'..'z'|'A'..'Z'|'0'..'9'|WS)*//*('a'..'z'|'A'..'Z'|'0'..'9'|WS)**/
	/*|	WS+('a'..'z'|'A'..'Z')('a'..'z'|'A'..'Z'|'0'..'9')*WS('a'..'z'|'A'..'Z'|'0'..'9'|WS)*	*/
	//;
NEWLINE	:	('\r'|'\n')+ {$channel=HIDDEN;}
	;
WS	:	(' '|'\t')+ {$channel=HIDDEN;}
	;
