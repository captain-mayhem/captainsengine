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
	|       lvl=level_stmt {$stmt = lvl.lvl;}
	|	row=row_stmt {$stmt = row.row;}
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
	
level_stmt returns [LevelNode* lvl]
	:	LEVEL LPAREN INT RPAREN lvlblock=block
	{
		int level = atoi((char*)$INT.text->chars);
		$lvl = new LevelNode(level, lvlblock.nodes);
	}
	;

row_stmt returns [RowNode* row]	
	:	ROW LPAREN INT SEMICOLON txt=arg SEMICOLON vis=stdarg RPAREN row_blk=block
	{
		int rownum = atoi((char*)$INT.text->chars);
		bool visible = vis.value->value() == "true" ? true : false;
		$row = new RowNode(rownum, ((IdentNode*)txt.value)->value(), visible, row_blk.nodes);
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
	arg_header? 
	(
		first=stdarg {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append(first.value->value().c_str()); delete first.value;}
	)
	(
		second=stdarg {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append(second.value->value().c_str()); delete second.value;}
		| MINUS {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$MINUS.text->chars);}
		| INT {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$INT.text->chars);}
	)*
	|	exp=rel_expr {$value = exp.exp;}
;

arg_header
	: INFO_BEG INT INFO_END;

stdarg returns [IdentNode* value]
	:
	IDENT {$value = new IdentNode(""); $value->append((char*)$IDENT.text->chars);}
	| ON {$value = new IdentNode(""); $value->append((char*)$ON.text->chars);}
	| IF {$value = new IdentNode(""); $value->append((char*)$IF.text->chars);}
;

rel_expr returns [ASTNode* exp]
	: {$exp = new RelationalNode();}
	( LESS {((RelationalNode*)$exp)->type() = RelationalNode::REL_LESS;}
	| GREATER {((RelationalNode*)$exp)->type() = RelationalNode::REL_GREATER;}
	| PLUS {((RelationalNode*)$exp)->type() = RelationalNode::REL_PLUS;}
	| MINUS {((RelationalNode*)$exp)->type() = RelationalNode::REL_MINUS;}
	)?
	ex=expr {((RelationalNode*)$exp)->child() = ex.exp;}
;

expr returns [ASTNode* exp]
	: var=variable {$exp = var.var;}
	| INT  {$exp = new IntNode(atoi((char*)$INT.text->chars));}
	| REAL {$exp = new RealNode((float)atof((char*)$REAL.text->chars));}
;

variable returns [ASTNode* var]
	:
	LBRACKET
	IDENT {$var = new VariableNode((char*)$IDENT.text->chars);}
	RBRACKET
;


LPAREN	:	'(';
RPAREN	:	')';
LBRACE  :	'{';
RBRACE	:	'}';
LBRACKET:	'[';
RBRACKET:	']';
SEMICOLON
	:	';';
UNDERSCORE
	:	'_';
PLUS	:	'+';
MINUS	:	'-';
GREATER	:	'>';
LESS	:	'<';
INFO_BEG	:	'|''#';
INFO_END	:	'#''|';
ON	:	('O'|'o')('N'|'n');
IF	:	('I'|'i')('F'|'f')UNDERSCORE;
IFNOT   :	('I'|'i')('F'|'f')('N'|'n')('O'|'o')('T'|'t')UNDERSCORE;
LEVEL	:	'l''e''v''e''l';
ROW	:	'r''o''w';
INT	:	'0'..'9'+;
REAL:	'0'..'9'+'.''0'..'9'+;
IDENT	:	('a'..'z'|'A'..'Z')('a'..'z'|'A'..'Z'|'0'..'9'|'\?'|'\''|'\.'|'!'|',')*;
/*STRING	:*/	/*WS*('a'..'z'|'A'..'Z')('a'..'z'|'A'..'Z'|'0'..'9'|WS)*//*('a'..'z'|'A'..'Z'|'0'..'9'|WS)**/
	/*|	WS+('a'..'z'|'A'..'Z')('a'..'z'|'A'..'Z'|'0'..'9')*WS('a'..'z'|'A'..'Z'|'0'..'9'|WS)*	*/
	//;
NEWLINE	:	('\r'|'\n')+ {$channel=HIDDEN;}
	;
WS	:	(' '|'\t')+ {$channel=HIDDEN;}
	;
