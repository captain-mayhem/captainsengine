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
	|	timer=timer_stmt {$stmt = timer.timer;}
	|	fc=func_call {$stmt = fc.func;}
	;

func_call returns [FuncCallNode* func]
	:	id=ident LPAREN args=arg_list RPAREN 
	{
		std::string fname = id.id->value(); delete id.id;
		$func = new FuncCallNode(fname,args.nodes);
	}
;
	
event_handler returns [EventNode* evt]
	:	ON LPAREN id=ident RPAREN evtblock=block
	{
		std::string eventname = id.id->value(); delete id.id;
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
		delete txt.value;
		delete vis.value;
	}
	;
	
timer_stmt returns [TimerNode* timer]
	:	TIMER LPAREN INT RPAREN exec=block
	{
		float time = (float)atoi((char*)$INT.text->chars);
		$timer = new TimerNode(time, exec.nodes);
	}
	;
	
conditional returns [CondNode* cond]
	:	
		(IF {$cond = new CondNode(false);}
		| 
		IFNOT {$cond = new CondNode(true);}
		) 
		id=ident LPAREN args=arg_list RPAREN ifblock=block
	{
		$cond->setCondition(id.id->value()); delete id.id;
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
	:
	exp=rel_expr {$value = exp.exp;}
	| {$value = new IdentNode("");}
	arg_header? 
	(
		first=stdarg {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append(first.value->value().c_str()); delete first.value;}
	)
	(
		second=stdarg {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append(second.value->value().c_str()); delete second.value;}
		| MINUS {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$MINUS.text->chars);}
		| INT {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$INT.text->chars);}
		| REAL  {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$REAL.text->chars);}
		| GREATER {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$GREATER.text->chars);}
	)*
;

arg_header
	: INFO_BEG INT INFO_END;

stdarg returns [IdentNode* value]
	:
	id=ident {$value = id.id;}
	| ON {$value = new IdentNode(""); $value->append((char*)$ON.text->chars);}
	| IF {$value = new IdentNode(""); $value->append((char*)$IF.text->chars);}
	| LESS {$value = new IdentNode(""); $value->append((char*)$LESS.text->chars);}
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
	| REAL {char tmp[64]; strcpy(tmp, (char*)$REAL.text->chars); char* tst = strchr(tmp, ','); if (tst != NULL) *tst = '.'; $exp = new RealNode((float)atof(tmp));}
	| INT  {$exp = new IntNode(atoi((char*)$INT.text->chars));}
;

variable returns [ASTNode* var]
	:
	LBRACKET
	id=ident {$var = new VariableNode(id.id->value()); delete id.id;}
	RBRACKET
;

ident returns [IdentNode* id]	
	:	
	IDENT_PART {$id = new IdentNode((char*)$IDENT_PART.text->chars);}
	(UNDERSCORE (secid=ident {$id->append("_"); $id->append(secid.id->value().c_str()); delete secid.id;}
		| INT {$id->append("_"); $id->append((char*)$INT.text->chars);}
		)
	)?
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
//TIMES	:	'*';
//DIVIDE	:	'/';
GREATER	:	'>';
LESS	:	'<';
INFO_BEG	:	'|''#';
INFO_END	:	'#''|';
ON	:	('O'|'o')('N'|'n');
IF	:	('I'|'i')('F'|'f')UNDERSCORE;
IFNOT   :	('I'|'i')('F'|'f')('N'|'n')('O'|'o')('T'|'t')UNDERSCORE;
LEVEL	:	'l''e''v''e''l';
ROW	:	'r''o''w';
TIMER:	't''i''m''e''r';
INT	:	'0'..'9'+;
REAL:	'0'..'9'+('.'|',')'0'..'9'+;
IDENT_PART	:	('a'..'z'|'A'..'Z'|'ü'|'Ü'|'ö'|'Ö'|'ä'|'Ä'|'ß')('a'..'z'|'A'..'Z'|'ü'|'Ü'|'ö'|'Ö'|'ä'|'Ä'|'ß'|'0'..'9'|'\?'|'\''|'\.'|'!'|','|'*')*;
NEWLINE	:	('\r'|'\n')+ {$channel=HIDDEN;}
	;
WS	:	(' '|'\t')+ {$channel=HIDDEN;}
	;
