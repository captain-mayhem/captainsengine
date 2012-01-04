grammar pcdk;

options{
	language=C;
	output=AST;
}

@parser::includes{
#include "AST.h"
}

@parser::postinclude{
extern ASTNode* stringify(ASTNode* tree);
//extern ASTNode* concatinateStr(ASTNode* left, const char* right);
//extern ASTNode* concatinateVar(ASTNode* left, VariableNode* right);
extern ASTNode* parseLangArg(const char* funcname, int argnum, int strindex);
std::string currentFunc;
int currentArg;
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
@init{ $func = NULL;}
	:	id=ident {currentFunc = id.id->value();}
	LPAREN args=arg_list RPAREN 
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
	:	ROW {currentFunc = "row";}
	LPAREN INT SEMICOLON txt=arg SEMICOLON vis=stdarg RPAREN row_blk=block
	{
		int rownum = atoi((char*)$INT.text->chars);
		bool visible = vis.value->value() == "true" ? true : false;
		ASTNode* arg = txt.value;
		IdentNode* text = NULL;
		if (arg->getType() == ASTNode::IDENTIFIER)
			text = (IdentNode*)arg;
		else{
			text = (IdentNode*)stringify((ASTNode*)txt.value);
		}
		$row = new RowNode(rownum, text->value(), visible, row_blk.nodes);
		if (arg->getType() != ASTNode::IDENTIFIER)
			delete text;
		delete txt.value;
		delete vis.value;
	}
	;
	
timer_stmt returns [TimerNode* timer]
	:	TIMER LPAREN (
	REAL { char tmp[64]; strcpy(tmp, (char*)$REAL.text->chars); char* tst = strchr(tmp, ','); if (tst != NULL) *tst = '.'; float time = (float)atof(tmp); $timer = new TimerNode(time);}
	| INT { float time = (float)atoi((char*)$INT.text->chars); $timer = new TimerNode(time); }
	) RPAREN exec=block
	{
		//float time = (float)atoi((char*)$INT.text->chars);
		$timer->setCommands(exec.nodes);
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
@init {currentArg = 0;}
	:	node=arg {++currentArg; $nodes = new NodeList(); $nodes->addNode(node.value);}
		(SEMICOLON 
		node=arg {++currentArg; $nodes->addNode(node.value);}
		)*
	|	{$nodes = new NodeList();}
;

arg	returns [ASTNode* value]
@init{ bool internalArgument = true;}
	:
	(ahdr=arg_header {internalArgument = false; $value = parseLangArg(currentFunc.c_str(), currentArg, ahdr.number->value()); delete ahdr.number;})?
	((rel_expr) => (exp=rel_expr {if (internalArgument){$value = exp.exp;}else{delete exp.exp;}}
		(vari=variable {if (internalArgument){ConcatenationNode* concat = new ConcatenationNode(); concat->left() = $value; concat->right() = vari.var; $value = concat;}else{delete vari.var;}}
		 | ca2=complex_arg {if (internalArgument){ConcatenationNode* concat = new ConcatenationNode(); concat->left() = $value; concat->right() = ca2.value; $value = concat;}else{delete ca2.value;}}
		)*
		)
	| (ca=complex_arg {if (internalArgument){$value = ca.value;}else{delete ca.value;}}
		(vari=variable {if (internalArgument){ConcatenationNode* concat = new ConcatenationNode(); concat->left() = $value; concat->right() = vari.var; $value = concat;}else{delete vari.var;}}
		| ca2=complex_arg {if (internalArgument){ConcatenationNode* concat = new ConcatenationNode(); concat->left() = $value; concat->right() = ca2.value; $value = concat;}else{delete ca2.value;}}
		)*
	  )
	)
;

arg_header returns [IntNode* number]
	: INFO_BEG 
	INT {$number = new IntNode(atoi((char*)$INT.text->chars));}
	INFO_END;
	
complex_arg returns [ASTNode* value]
	:
	({$value = new IdentNode("");}
	(
		first=stdarg {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append(first.value->value().c_str()); delete first.value;}
	)
	(
		second=stdarg {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append(second.value->value().c_str()); delete second.value;}
		| MINUS {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$MINUS.text->chars);}
		| REAL  {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$REAL.text->chars);}
		| GREATER {((IdentNode*)$value)->append(" "); ((IdentNode*)$value)->append((char*)$GREATER.text->chars);}
	)*)
	;

stdarg returns [IdentNode* value]
	:
	id=ident {$value = id.id;}
	| ON {$value = new IdentNode(""); $value->append((char*)$ON.text->chars);}
	| IF {$value = new IdentNode(""); $value->append((char*)$IF.text->chars);}
	| LESS {$value = new IdentNode(""); $value->append((char*)$LESS.text->chars);}
	| RBRACKET {$value = new IdentNode(""); $value->append((char*)$RBRACKET.text->chars);}
	| DIVIDE {$value = new IdentNode(""); $value->append((char*)$DIVIDE.text->chars);}
	| INT {$value = new IdentNode(""); $value->append((char*)$INT.text->chars);}
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
	: lt=term {$exp = lt.trm;}
	((
	PLUS {ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_PLUS; an->left() = $exp; $exp = an;}
	| MINUS {ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_MINUS; an->left() = $exp; $exp = an;}
	) 
	rt=term {((ArithmeticNode*)$exp)->right() = rt.trm;}
	)*
;

term returns [ASTNode* trm]
	: lf=factor {$trm = lf.fac;}
	((TIMES | DIVIDE) => (
	TIMES {ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_TIMES; an->left() = $trm; $trm = an;}
	| DIVIDE {ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_DIV; an->left() = $trm; $trm = an;}
	) 
	rf=factor {((ArithmeticNode*)$trm)->right() = rf.fac;}
	)*
;

factor returns [ASTNode* fac]
	: var=variable {$fac = var.var;} (',' var2=variable {
		ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_PLUS; an->left() = $fac;
		ArithmeticNode* mul = new ArithmeticNode(); mul->type() = ArithmeticNode::AR_TIMES; mul->left() = var2.var; an->right() = mul;
		mul->right() = new RealNode(0.1f); //TODO how to make generic
		$fac = an;
	}
	)?
	| REAL {char tmp[64]; strcpy(tmp, (char*)$REAL.text->chars); char* tst = strchr(tmp, ','); if (tst != NULL) *tst = '.'; $fac = new RealNode((float)atof(tmp));}
	| INT  {$fac = new IntNode(atoi((char*)$INT.text->chars));}
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
	)*
	;

COMMENT	:	'(*'
		(~(TIMES))*
		'*)'
{$channel=HIDDEN;}
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
TIMES	:	'*';
DIVIDE	:	'/';
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
IDENT_PART	:	('a'..'z'|'A'..'Z'|'\u00fc'|'\u00dc'|'\u00f6'|'\u00d6'|'\u00e4'|'\u00c4'|'\u00df'|'0'..'9'|':'|'\''|'\.'|'&'|TIMES)
				('a'..'z'|'A'..'Z'|'\u00fc'|'\u00dc'|'\u00f6'|'\u00d6'|'\u00e4'|'\u00c4'|'\u00df'|'0'..'9'|'\?'|'\''|'\.'|'!'|','|'&'|TIMES|':')*;
NEWLINE	:	('\r'|'\n')+ {$channel=HIDDEN;}
	;
WS	:	(' '|'\t'|'"')+ {$channel=HIDDEN;}
	;
	
