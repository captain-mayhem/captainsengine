grammar pcdk;

options{
	language=C;
	output=AST;
	}

@parser::includes{
#include "AST.h"
#include <system/allocation.h>
}

@parser::postinclude{
extern ASTNode* stringify(ASTNode* tree);
extern ASTNode* parseLangArg(const char* funcname, int argnum, int strindex);
std::string currentFunc;
int currentArg;
ASTNode* incompleteAlloc = NULL;
}

prog returns [NodeList* nodes]
	:	{$nodes = new NodeList();}
		(
		statement=stmt {$nodes->addNode(statement.stmt);}
		| blk=braced_block {$nodes->addNodes(blk.nodes); delete blk.nodes;}
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
	|	fc=func_call {if (incompleteAlloc) delete incompleteAlloc; $stmt = fc.func;}
	;

func_call returns [FuncCallNode* func]
@init{ $func = NULL;}
	:	id=ident {currentFunc = id.id->value(); incompleteAlloc=id.id;}
	LPAREN args=arg_list RPAREN SEMICOLON?
	{
		std::string fname = id.id->value(); delete id.id;
		$func = new FuncCallNode(fname,args.nodes);
		incompleteAlloc = NULL;
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
	fa = factor {$timer = new TimerNode(fa.fac); }
	) RPAREN exec=block
	{
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

braced_block returns [NodeList* nodes]
	:	{$nodes = new NodeList();}
		LBRACE
		(
			stmtnode=nested_stmt {$nodes->addNode(stmtnode.stmt);}
			|   evth=event_handler {delete evth.evt;/* ignore nested events */}
			|   blk=braced_block {$nodes->addNodes(blk.nodes); delete blk.nodes;}
		)*
		RBRACE
	;
	
block returns [NodeList* nodes]
@init{ $nodes = NULL;}
	:
	bb=braced_block {$nodes = bb.nodes;}
	|	stmtnode=nested_stmt {$nodes = new NodeList(); $nodes->addNode(stmtnode.stmt);}
	;
	
arg_list returns [NodeList* nodes]
@init {currentArg = 0;}
	:	node=arg {++currentArg; $nodes = new NodeList(); $nodes->addNode(node.value);}
		(SEMICOLON 
		(node=arg {++currentArg; $nodes->addNode(node.value);}
		)?
		)*
	|	{$nodes = new NodeList();}
;

arg	returns [ASTNode* value]
@init{ bool internalArgument = true; $value = NULL;}
	:
	(ahdr=arg_header {internalArgument = false; $value = parseLangArg(currentFunc.c_str(), currentArg, ahdr.number->value()); delete ahdr.number;})?
	((rel_expr (SEMICOLON|RPAREN)) => (exp=rel_expr {if (internalArgument){$value = exp.exp;}else{delete exp.exp;}}
	 )
	| (ca=complex_arg {
			if (internalArgument){
				if ($value == NULL)
					$value = ca.value;
				else{
					ConcatenationNode* concat = new ConcatenationNode();
					concat->left() = $value;
					concat->right() = ca.value;
					$value = concat;
				}
			}
			else{
				delete ca.value;
			}
		}
	  )
	)
;

arg_header returns [IntNode* number]
	: INFO_BEG 
	INT {$number = new IntNode(atoi((char*)$INT.text->chars));}
	INFO_END;
	
complex_arg returns [ASTNode* value]
	:
	(ls=long_string {$value = ls.ident;} 
	 | vari=variable  { $value = vari.var;}
	)
	(ca=complex_arg { ConcatenationNode* concat = new ConcatenationNode(); concat->left() = $value; concat->right() = ca.value; $value = concat;}
	)?
;
	
long_string returns [IdentNode* ident]
@init {$ident = new IdentNode("");}
	:
	(str=~(LBRACKET|SEMICOLON|RPAREN) {
		char* txt = (char*)$str.text->chars;
		if (txt[0] != '_' && $ident->lastChar() != '_')
			$ident->append(" ");
		$ident->append(txt);
	 } )+
;
	
stdarg returns [IdentNode* value]
	:
	id=ident {$value = id.id;}
	| ON {$value = new IdentNode(""); $value->append((char*)$ON.text->chars);}
	| IF {$value = new IdentNode(""); $value->append((char*)$IF.text->chars);}
	| LESS {$value = new IdentNode(""); $value->append((char*)$LESS.text->chars);}
	| GREATER {$value = new IdentNode(""); $value->append((char*)$GREATER.text->chars);}
	| RBRACKET {$value = new IdentNode(""); $value->append((char*)$RBRACKET.text->chars);}
	| IDIV {$value = new IdentNode(""); $value->append((char*)$IDIV.text->chars);}
	| INT {$value = new IdentNode(""); $value->append((char*)$INT.text->chars);}
	| ' , ' {$value = new IdentNode(",");}
;

rel_expr returns [ASTNode* exp]
	: {$exp = new RelationalNode();}
	( (LESS {((RelationalNode*)$exp)->type() = RelationalNode::REL_LESS;} (MINUS {((RelationalNode*)$exp)->negated() = true;})?)
	| (GREATER {((RelationalNode*)$exp)->type() = RelationalNode::REL_GREATER;} (MINUS {((RelationalNode*)$exp)->negated() = true;})?)
	| PLUS {((RelationalNode*)$exp)->type() = RelationalNode::REL_PLUS;}
	| MINUS {((RelationalNode*)$exp)->type() = RelationalNode::REL_MINUS;}
	| (TIMES {((RelationalNode*)$exp)->type() = RelationalNode::REL_TIMES;} (MINUS {((RelationalNode*)$exp)->negated() = true;})?)
	)?
	ex=expr {((RelationalNode*)$exp)->child() = ex.exp;}
;

expr returns [ASTNode* exp]
	: lt=term {$exp = lt.trm;}
	((PLUS | MINUS) => (
	PLUS {ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_PLUS; an->left() = $exp; $exp = an;}
	| MINUS {ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_MINUS; an->left() = $exp; $exp = an;}
	) 
	rt=term {((ArithmeticNode*)$exp)->right() = rt.trm;}
	)*
;

term returns [ASTNode* trm]
	: lf=factor {$trm = lf.fac;}
	((TIMES | DIVIDE | IDIV) => (
	TIMES {ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_TIMES; an->left() = $trm; $trm = an;}
	| DIVIDE {ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_DIV; an->left() = $trm; $trm = an;}
	| IDIV {ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_IDIV; an->left() = $trm; $trm = an;}
	) 
	rf=factor {((ArithmeticNode*)$trm)->right() = rf.fac;}
	)*
;

factor returns [ASTNode* fac]
	: var=variable {$fac = var.var;} (COMMA var2=variable {
		UnaryArithNode* i2r = new UnaryArithNode(); i2r->type() = UnaryArithNode::UA_I2R; i2r->node() = $fac;
		ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_PLUS; an->left() = i2r;
		UnaryArithNode* shift = new UnaryArithNode(); shift->type() = UnaryArithNode::UA_DEC_SHIFT; shift->node() = var2.var; an->right() = shift;
		$fac = an;
	}
	)?
	| REAL {char tmp[64]; strcpy(tmp, (char*)$REAL.text->chars); char* tst = strchr(tmp, ','); if (tst != NULL) *tst = '.'; $fac = new RealNode((float)atof(tmp));}
	| INT {$fac = new IntNode(atoi((char*)$INT.text->chars));}
	| (REAL_INT  {$fac = new IntNode(atoi((char*)$REAL_INT.text->chars));} var2=variable{
		UnaryArithNode* i2r = new UnaryArithNode(); i2r->type() = UnaryArithNode::UA_I2R; i2r->node() = $fac;
		ArithmeticNode* an = new ArithmeticNode(); an->type() = ArithmeticNode::AR_PLUS; an->left() = i2r;
		UnaryArithNode* shift = new UnaryArithNode(); shift->type() = UnaryArithNode::UA_DEC_SHIFT; shift->node() = var2.var; an->right() = shift;
		$fac = an;
	}
	)
;

variable returns [ASTNode* var]
	:
	LBRACKET
	id=ident {$var = new VariableNode(id.id->value()); delete id.id;}
	/* names with : are allowed here */
	(IDIV (id2=ident {VariableNode* node = (VariableNode*)$var; node->name().append(":"+id2.id->value()); delete id2.id;} 
	   |  var2=variable {VariableNode* node = (VariableNode*)$var; node->name().append(":"); node->setChild((VariableNode*)var2.var);} )
	 )?
	 (var3=variable {VariableNode* node = (VariableNode*)$var; node->setChild((VariableNode*)var3.var);} 
	 | INT {VariableNode* node = (VariableNode*)$var; node->name().append((char*)$INT.text->chars);}
	 | id3=ident {VariableNode* node = (VariableNode*)$var; node->name().append(id3.id->value()); delete id3.id;}
	 )*
	RBRACKET
;

ident returns [IdentNode* id]	
	:	
	IDENT_PART {$id = new IdentNode((char*)$IDENT_PART.text->chars);}
	(UNDERSCORE (secid=ident {$id->append("_"); $id->append(secid.id->value().c_str()); delete secid.id;}
		| INT {$id->append("_"); $id->append((char*)$INT.text->chars);}
		| LEVEL {$id->append("_"); $id->append((char*)$LEVEL.text->chars);}
		| ON {$id->append("_"); $id->append((char*)$ON.text->chars);}
		| TIMER {$id->append("_"); $id->append((char*)$TIMER.text->chars);}
		| ROW {$id->append("_"); $id->append((char*)$ROW.text->chars);}
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
COMMA : ',';
UNDERSCORE
	:	'_';
PLUS	:	'+';
MINUS	:	'-';
TIMES	:	'*';
DIVIDE	:	'/';
IDIV    :   ':';
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
REAL:	'0'..'9'+('\.'|COMMA)'0'..'9'+;
REAL_INT:	INT COMMA;
fragment IDENT_FRAG: ('a'..'z'|'A'..'Z'|'\?'|'\''|'\.'|'!'|COMMA|'&'|'|'|'%'|'\\'|'\u0080'..'\u00ff');
IDENT_PART	:	(IDENT_FRAG|'0'..'9')+;
NEWLINE	:	('\r'|'\n')+ {$channel=HIDDEN;}
	;
WS	:	(' '|'\t'|'"')+ {$channel=HIDDEN;}
	;
	
