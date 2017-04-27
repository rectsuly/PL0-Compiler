// pl0 compiler source code
//for Visual Studio
#pragma warning(disable:4996) 


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "pl0.h"
#include "set.h"

//////////////////////////////////////////////////////////////////////
// print error message.
void error(int n)
{
	int i;

	printf("      ");
	for (i = 1; i <= cc - 1; i++)
		printf(" ");
	printf("^\n");
	printf("Error %3d: %s\n", n, err_msg[n]);
	err++;
} // error

//////////////////////////////////////////////////////////////////////
void getch(void)
{
	if (cc == ll)
	{
		if (feof(infile))
		{
			printf("\nPROGRAM INCOMPLETE\n");
			exit(1);
		}
		ll = cc = 0;
		printf("%5d  ", cx);
		while ( (!feof(infile)) // added & modified by alex 01-02-09
			    && ((ch = getc(infile)) != '\n'))
		{
			printf("%c", ch);
			line[++ll] = ch;
		} // while
		printf("\n");
		line[++ll] = ' ';
	}
	ch = line[++cc];
} // getch

//////////////////////////////////////////////////////////////////////
// gets a symbol from input stream.
void getsym(void)
{
	int i, k;
	char a[MAXIDLEN + 1];
	if (sym_count > 0)
	{
		sym = sym_stack[--sym_count];
		return;
	}
	while (ch == ' '|| ch == '\t')
		getch();

	if (isalpha(ch))
	{ // symbol is a reserved word or an identifier.
		k = 0;
		do
		{
			if (k < MAXIDLEN)
				a[k++] = ch;
			getch();
		}
		while (isalpha(ch) || isdigit(ch));
		a[k] = 0;
		strcpy(id, a);
		word[0] = id;
		i = NRW;
		while (strcmp(id, word[i--]));
		if (++i)
			sym = wsym[i];				// symbol is a reserved word
		else {
			if (ch == '[')
				sym = SYM_ARRAY;		// symbol is an array
			else
				sym = SYM_IDENTIFIER;   // symbol is an identifier
		}
			
	}
	else if (isdigit(ch))
	{ // symbol is a number.
		k = num = 0;
		sym = SYM_NUMBER;
		do
		{
			num = num * 10 + ch - '0';
			k++;
			getch();
		}
		while (isdigit(ch));
		if (k > MAXNUMLEN)
			error(25);     // The number is too great.
	}
	else if (ch == '\n')
	{
		getch();
		getsym();
	}
	else if (ch == ':')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_BECOMES; // :=
			getch();
		}
		else
		{
			sym = SYM_NULL;       // illegal?
		}
	}
	else if (ch == '>')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_GEQ;     // >=
			getch();
		}
		else
		{
			sym = SYM_GTR;     // >
		}
	}
	else if (ch == '<')
	{
		getch();
		if (ch == '=')
		{
			sym = SYM_LEQ;     // <=
			getch();
		}
		else if (ch == '>')
		{
			sym = SYM_NEQ;     // <>
			getch();
		}
		else
		{
			sym = SYM_LES;     // <
		}
	}
	else if (ch == '/')     //添加注释
	{
		getch();
		if (ch == '*')
		{
			getch();
			while (1)
			{
				while (ch != '*')
					getch();
				getch();
				if (ch == '/')  //  注释/*... */
					break;
			}	
			getch();
			getsym();
		}
		else if(ch == '/')
		{
			cc = ll;
			ch = ' ';
			getsym();
		}
		else
		{
			sym = SYM_SLASH;
		}	
			
	}
	else if (ch == ']') {		// 表示数组的右中括号
		getch();
		sym = SYM_RSPAREN;
	}
	else if (ch == '+')
	{
		getch();
		if (ch == '=')
		{
			getch();
			sym = SYM_ADDPLUS;
		}
		else
		{
			sym = SYM_PLUS;
		}
	}
	else if (ch == '-')
	{
		getch();
		if (ch == '=')
		{
			getch();
			sym = SYM_SUBPLUS;
		}
		else
		{
			sym = SYM_MINUS;
		}
	}
	else
	{ // other tokens
		i = NSYM;
		csym[0] = ch;
		while (csym[i--] != ch);
		if (++i)
		{
			sym = ssym[i];
			getch();
		}
		else
		{
			printf("Fatal Error: Unknown character.\n");
			exit(1);
		}
	}
} // getsym

//////////////////////////////////////////////////////////////////////
// generates (assembles) an instruction.
void gen(int x, int y, int z)
{
	if (cx > CXMAX)
	{
		printf("Fatal Error: Program too long.\n");
		exit(1);
	}
	code[cx].f = x;
	code[cx].l = y;
	code[cx++].a = z;
//	printf("%s\t%d\t%d\n",  mnemonic[x], y, z);   //单步调试用
} // gen

//////////////////////////////////////////////////////////////////////
// tests if error occurs and skips all symbols that do not belongs to s1 or s2.
void test(symset s1, symset s2, int n)
{
	symset s;

	if (! inset(sym, s1))
	{
		error(n);
		s = uniteset(s1, s2);
		while(! inset(sym, s))
			getsym();
		destroyset(s);
	}
} // test

//////////////////////////////////////////////////////////////////////
int dx;  // data allocation index

// enter object(constant, variable or procedre) into table.
void enter(int kind)
{
	mask* mk;

	tx++;
	strcpy(table[tx].name, id);
	table[tx].kind = kind;
	switch (kind)
	{
	case ID_CONSTANT:
		if (num > MAXADDRESS)
		{
			error(25); // The number is too great.
			num = 0;
		}
		table[tx].value = num;
		break;
	case ID_VARIABLE:
		mk = (mask*) &table[tx];
		mk->level = level;
		mk->address = dx++;
		break;
	case ID_PROCEDURE:
		mk = (mask*) &table[tx];
		mk->level = level;
		break;
	} // switch
} // enter


  /*
   *将数组类型填入数组的符号表中
   */
void array_enter()
{
	ax++;
	array_table[ax] = array_t;
	strcpy(array_table[ax].name, id);
	enter(ID_VARIABLE);
	array_table[ax].addr = tx;							//记录数组初始偏移地址
	for (int i = array_table[ax].sum - 1; i > 0; i--)	//将数组的所有存储空间填入符号表中
		enter(ID_VARIABLE);
} // array_enter

//////////////////////////////////////////////////////////////////////
// locates identifier in symbol table.
int position(char* id)
{
	int i;
	strcpy(table[0].name, id);
	i = tx + 1;
	while (strcmp(table[--i].name, id) != 0);
	return i;
} // position

 /*
  *定位标识符在数组符号表中的位置
  */
int array_position()
{
	int i = 0;
	while (strcmp(array_table[++i].name, id));
	if (i <= ax)
		return i;
	else
		return 0;
} // array_position

//////////////////////////////////////////////////////////////////////
void constdeclaration()
{
	if (sym == SYM_IDENTIFIER)
	{
		getsym();
		if (sym == SYM_EQU || sym == SYM_BECOMES)
		{
			if (sym == SYM_BECOMES)
				error(1); // Found ':=' when expecting '='.
			getsym();
			if (sym == SYM_NUMBER)
			{
				enter(ID_CONSTANT);
				getsym();
			}
			else
			{
				error(2); // There must be a number to follow '='.
			}
		}
		else
		{
			error(3); // There must be an '=' to follow the identifier.
		}
	} else	error(4);
	 // There must be an identifier to follow 'const', 'var', or 'procedure'.
} // constdeclaration
	
//////////////////////////////////////////////////////////////////////
void vardeclaration(void)
{
	int i, dim = 0;
	if (sym == SYM_IDENTIFIER)
	{
		enter(ID_VARIABLE);
		getsym();
	}
	else if (sym == SYM_ARRAY) {			//声明变量若为数组	
		while(ch == '[')
		{
			dim++;
			getch();						//读取'['后的下一个字符
			getsym();						//读取'['和']'之间的数字，其值num为当前维数的数组空间
		
			array_t.dim[dim - 1] = num;		//声明数组所在维数的数据大小
			getsym();						//读取']'后的下一个字符
		}
		array_t.n = dim;					//数组的总维数
		array_t.size[dim - 1] = 1;			//最外层的偏移量大小为1
		for (int i = dim - 1; i > 0; i--)
			array_t.size[i - 1] = array_t.size[i] * array_t.dim[i];		//计算数组元素的偏移地址
		array_t.sum = array_t.size[0] * array_t.dim[0];					//计算出数组的所有元素占用空间
		array_enter();						//填入数组符号表
		getsym();
	//	else
	//		enter(ID_VARIABLE);
	}
	else
	{
		error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
	}
} // vardeclaration

//////////////////////////////////////////////////////////////////////
void listcode(int from, int to)
{
	int i;
	
	printf("\n");
	for (i = from; i < to; i++)
	{
		printf("%5d %s\t%d\t%d\n", i, mnemonic[code[i].f], code[i].l, code[i].a);
	}
	printf("\n");
} // listcode

//////////////////////////////////////////////////////////////////////
void factor(symset fsys)
{
	void expression(symset fsys);
	int i,j;
	int dim = 0;	//数组当前维数
	symset set;
	mask* mk;
	
//	test(facbegsys, fsys, 24); // The symbol can not be as the beginning of an expression.

	while (inset(sym, facbegsys))
	{
		if (sym == SYM_IDENTIFIER)
		{
			if ((i = position(id)) == 0)
			{
				error(11); // Undeclared identifier.
			}
			else
			{
				switch (table[i].kind)
				{
				case ID_CONSTANT:
					gen(LIT, 0, table[i].value);
					break;
				case ID_VARIABLE:
					mk = (mask*) &table[i];
					gen(LOD, level - mk->level, mk->address);
				//	printf("level=%d,mk->level=%d,mk->address=%d\n", level, mk->level, mk->address);	//调试用
					break;
				case ID_PROCEDURE:
					error(21); // Procedure identifier can not be in an expression.
					break;
				} // switch
			}
			getsym();
		}
		else if (sym == SYM_ARRAY)					//因子为数组类型
		{
			if (!(i = array_position()))			//定位数组变量在符号表中的位置
				error(11);
			else {
				j = array_table[i].addr;			//记录数组首地址
				mk = (mask*) &table[j];
				gen(LIT, 0, 0);						//通过累加数组偏移量来确定元素位置，初始为0
				while (ch == '[') {
					dim++;							//进入左中括号后维数+1，
					getch();
					getsym();						//读取数组括号中的数值
					set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
					expression(set);				//数组中的表达式
					destroyset(set);
					gen(LIT, 0, array_table[i].size[dim - 1]);		//取当前维数偏移大小到栈顶
					gen(OPR, 0, OPR_MUL);			//该维数的值乘以该维数偏移大小
					gen(OPR, 0, OPR_ADD);			//累加到总偏移
				}
				gen(LDA, level - mk->level, mk->address);			//生成加载数组指令，记录层差和偏移
			}
			getsym();
		}
		else if (sym == SYM_NUMBER)
		{
			if (num > MAXADDRESS)
			{
				error(25); // The number is too great.
				num = 0;
			}
			gen(LIT, 0, num);
			getsym();
		}
		else if (sym == SYM_LPAREN)
		{
			getsym();
			set = uniteset(createset(SYM_RPAREN, SYM_NULL), fsys);
			expression(set);
			destroyset(set);
			if (sym == SYM_RPAREN)
			{
				getsym();
			}
			else
			{
				error(22); // Missing ')'.
			}
		}
		test(fsys, createset(SYM_LPAREN, SYM_NULL), 23);
	} // while
} // factor

//////////////////////////////////////////////////////////////////////
void term(symset fsys)
{
	int mulop;
	symset set;
	//fsys中添加中括号的symtype值
	set = uniteset(fsys, createset(SYM_TIMES, SYM_SLASH, SYM_LSPAREN, SYM_RSPAREN, SYM_NULL));
	factor(set);
	while (sym == SYM_TIMES || sym == SYM_SLASH)
	{
		mulop = sym;
		getsym();
		factor(set);
		if (mulop == SYM_TIMES)
		{
			gen(OPR, 0, OPR_MUL);
		}
		else
		{
			gen(OPR, 0, OPR_DIV);
		}
	} // while
	destroyset(set);
} // term

//////////////////////////////////////////////////////////////////////
void expression(symset fsys)
{
	int addop;
	symset set;
	//fsys中添加中括号的symtype值
	set = uniteset(fsys, createset(SYM_PLUS, SYM_MINUS,SYM_MOD, SYM_LSPAREN, SYM_RSPAREN, SYM_NULL));
	if (sym == SYM_PLUS || sym == SYM_MINUS)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_MINUS)
		{
			gen(OPR, 0, OPR_NEG);
		}
	}
	else
	{
		term(set);
	}

	while (sym == SYM_PLUS || sym == SYM_MINUS||sym == SYM_MOD)
	{
		addop = sym;
		getsym();
		term(set);
		if (addop == SYM_PLUS)
		{
			gen(OPR, 0, OPR_ADD);
		}
		else if(sym == SYM_MINUS)
		{
			gen(OPR, 0, OPR_MIN);
		}
		else
		{
			gen(OPR, 0, OPR_MOD);
		}
	} // while

	destroyset(set);
} // expression

//////////////////////////////////////////////////////////////////////
void condition(symset fsys)
{
	int relop;
	symset set;

	if (sym == SYM_ODD)
	{
		getsym();
		expression(fsys);
		gen(OPR, 0, 6);
	}
	else
	{
		set = uniteset(relset, fsys);
		expression(set);
		destroyset(set);
		if (! inset(sym, relset))
		{
			error(20);
		}
		else
		{
			relop = sym;
			getsym();
			expression(fsys);
			switch (relop)
			{
			case SYM_EQU:
				gen(OPR, 0, OPR_EQU);
				break;
			case SYM_NEQ:
				gen(OPR, 0, OPR_NEQ);
				break;
			case SYM_LES:
				gen(OPR, 0, OPR_LES);
				break;
			case SYM_GEQ:
				gen(OPR, 0, OPR_GEQ);
				break;
			case SYM_GTR:
				gen(OPR, 0, OPR_GTR);
				break;
			case SYM_LEQ:
				gen(OPR, 0, OPR_LEQ);
				break;
			} // switch
		} // else
	} // else
} // condition
/////////////////////////////////////////////////////////////////////

void ex_condition(symset fsys)
{
	int relop;
	condition_level++;
	true_count[condition_level] = 0; //复位0
	false_count[condition_level] = 0;//复位0
	if (sym == SYM_NOT) //条件第一个是NOT
	{
		getsym();
		condition(fsys);
		gen(OPR, 0, OPR_NOT);
	}
	else
	{
		condition(fsys);
	}
	
	if (sym == SYM_THEN||sym == SYM_SEMICOLON) //只有一个条件,即没有not and or计算。
	{
		false_out[condition_level][false_count[condition_level]++] = cx;
		gen(JPC, 0, 0);   //条件不成立
		true_out[condition_level][true_count[condition_level]++] = cx;
		gen(JMP, 0, 0);  //条件成立
		return;
	}
	else if (sym == SYM_AND || sym == SYM_OR || sym == SYM_NOT)
	{
		while (sym == SYM_AND || sym == SYM_OR || sym == SYM_NOT)
		{
			relop = sym;
			getsym();
			switch (relop)
			{
			case SYM_OR:
				true_out[condition_level][true_count[condition_level]++] = cx;
				gen(JPNC, 0, 0); //如果成立，跳到true 的出口,等待回填
				if (sym == SYM_NOT)  //or 的后面是not
				{
					getsym();
					condition(fsys);
					gen(OPR, 0, OPR_NOT); //布尔值取反
				}
				else
				{
					condition(fsys);
				}
				if (sym == SYM_THEN || sym == SYM_SEMICOLON)
				{
					false_out[condition_level][false_count[condition_level]++] = cx; //条件不成立
					gen(JPC, 0, 0);
					true_out[condition_level][true_count[condition_level]++] = cx;
					gen(JMP, 0, 0);  //条件成立
					return;
				}
				break;

			case SYM_AND:
				false_out[condition_level][false_count[condition_level]++] = cx;
				gen(JPC, 0, 0);  //跳到false出口
				if (sym == SYM_NOT)  //and 后面是 not
				{
					getsym();
					condition(fsys);
					gen(OPR, 0, OPR_NOT);
				}
				else
				{
					condition(fsys);
				}
				if (sym == SYM_THEN || sym == SYM_SEMICOLON)
				{
					false_out[condition_level][false_count[condition_level]++] = cx; //条件不成立
					gen(JPC, 0, 0);
					true_out[condition_level][true_count[condition_level]++] = cx;
					gen(JMP, 0, 0);  //条件成立
					return;
				}
				break;

			/*case SYM_NOT:
				condition(fsys);
				gen(OPR, 0, OPR_NEG);
				if (sym == SYM_THEN)
				{
					false_out[false_count++] = cx;
					gen(JPC, 0, 0);
					return;
				}
				break;*/
			}

		}
		error(16);
	}
	else
	{
		error(16);
	}
	

}
//////////////////////////////////////////////////////////////////////
void statement(symset fsys)
{
	int i, cx1, cx2;
	symset set1, set;

	//for array
	int dim = 0, j;
	mask* mk;
	// for for-loop
	int before_update;
	int before_condition;

	if (sym == SYM_IDENTIFIER)
	{ // variable assignment
		if (! (i = position(id)))
		{
			error(11); // Undeclared identifier.
		}
		else if (table[i].kind != ID_VARIABLE)
		{
			error(12); // Illegal assignment.
			i = 0;
		}
		mk = (mask*)&table[i];
		getsym();
		if (sym == SYM_BECOMES)
		{
			//if (sym == SYM_BECOMES)
			//{
				getsym();
			//}
			/*else
			{
				error(13); // ':=' expected.
			}*/

			expression(fsys);
		}
		else if (sym == SYM_ADDPLUS)
		{
			getsym();
			gen(LOD, level - mk->level, mk->address);
			expression(fsys);
			gen(OPR, 0, OPR_ADD);
		}
		else if(sym == SYM_SUBPLUS)
		{
			getsym();
			gen(LOD, level - mk->level, mk->address);
			expression(fsys);
			gen(OPR, 0, OPR_MIN);
		}
		else
		{
			error(13);
		}
		//mk = (mask*) &table[i];
		if (i)
		{
			gen(STO, level - mk->level, mk->address);
		}
	}
	else if (sym == SYM_ARRAY)		//statment语句起始为数组类型
	{
		if (!(i = array_position()))
			error(11);
		else {
			j = array_table[i].addr;
			mk = (mask*)&table[j];
			gen(LIT, 0, 0);
			while (ch == '[') {
				dim++;
				getch();
				getsym();
				set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
				expression(set);
				destroyset(set);
				gen(LIT, 0, array_table[i].size[dim - 1]);
				gen(OPR, 0, OPR_MUL);
				gen(OPR, 0, OPR_ADD);
			}
		}
		getsym();
		if (sym == SYM_BECOMES)
			getsym();
		else
			error(13);		// ':=' expected.
		set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
		expression(set);
		destroyset(set);
		j = array_table[i].addr;
		mk = (mask*)&table[j];
		if (j)
			gen(STA, level - mk->level, mk->address);		//存储数组值到特定层和特定偏移地址
	}
	else if (sym == SYM_CALL)
	{ // procedure call
		getsym();
		if (sym != SYM_IDENTIFIER)
		{
			error(14); // There must be an identifier to follow the 'call'.
		}
		else
		{
			if (! (i = position(id)))
			{
				error(11); // Undeclared identifier.
			}
			else if (table[i].kind == ID_PROCEDURE)
			{
				mask* mk;
				mk = (mask*) &table[i];
				gen(CAL, level - mk->level, mk->address);
			}
			else
			{
				error(15); // A constant or variable can not be called. 
			}
			getsym();
		}
	} 
	else if (sym == SYM_IF)
	{ // if statement
		getsym();
		set1 = createset(SYM_THEN, SYM_DO, SYM_AND,SYM_NOT,SYM_OR,SYM_NULL);
		set = uniteset(set1, fsys);
		ex_condition(set);
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_THEN)
		{
			getsym();
		}
		else
		{
			error(16); // 'then' expected.
		}
		
		//cx1 = cx;  //(原)
		//gen(JPC, 0, 0);//（原）
		
		for (int k = 0; k < true_count[condition_level];k++) /*所有真出口都跳到statement的前面*/
			code[true_out[condition_level][k]].a = cx;
		
		//条件成立的执行语句
		statement(fsys);
		mid_cx = cx;
		gen(JMP, 0, 0);  //成立语句执行完之后 要么跳到else-statment之后，要么继续执行

		if (sym == SYM_SEMICOLON)
		{ 
			getsym();
		}
		else
		{
			error(10);
		}
		if (sym == SYM_ELSE)  //如果有else字句，则false出口到else字句
		{
			getsym();
			for (int i = 0;i < false_count[condition_level];i++) //所有假出口都跳到else字后面
				code[false_out[condition_level][i]].a = cx;
			statement(fsys);
		}
		else
		{
			sym_stack[sym_count++] = sym;
			sym = SYM_SEMICOLON;
			for (int k = 0;k < false_count[condition_level];k++) /*所有假出口都跳到if-statement 后面*/
				code[false_out[condition_level][k]].a = cx;
		}
		code[mid_cx].a = cx;
		condition_level--;
		//code[cx1].a = cx;	
	}
	else if (sym == SYM_FOR)
	{
		getsym();
		break_count++; //表示是否允许break
		if (sym = SYM_LPAREN)
		{
			getsym();
		}
		else
		{
			error(26); //expected '('
		}
		statement(fsys);  //循环变量初始值
		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else
		{
			error(17);
		}
		//for循环条件
		before_condition = cx; //记录条件前的指令序号，执行完变量更新后需要跳转到这
		set1 = createset(SYM_THEN, SYM_DO, SYM_AND, SYM_NOT, SYM_OR, SYM_NULL);
		set = uniteset(set1, fsys);
		ex_condition(set); //条件
		destroyset(set1);
		destroyset(set);

		if (sym == SYM_SEMICOLON)
		{
			getsym();
		}
		else
		{
			error(17);
		}
		
		before_update = cx; //记录更新变量的指令序号，当循环体执行完后需要跳转到这
		//循环变量自增自减
		statement(fsys);

		gen(JMP, 0, before_condition);//更新完后跳到条件处

		if (sym == SYM_RPAREN)
		{
			getsym();
		}
		else
		{
			error(26);
		}
		/*所有真出口都跳到loop-statement的前面*/
		for (int k = 0; k < true_count[condition_level];k++) 
			code[true_out[condition_level][k]].a = cx;
		
		statement(fsys); //循环体 loop-statement
		gen(JMP, 0, before_update); //执行完后跳到自增自减语句

		/*所有假出口都跳到loop-statement 后面*/
		for (int k = 0;k < false_count[condition_level];k++)
			code[false_out[condition_level][k]].a = cx;
		condition_level--; //减少条件层次

		//如果存在break，则跳到该处
		if (break_cx[break_count] > 0) 
		{
			code[break_cx[break_count]].a = cx;
			break_count--;
		}
	}
	else if (sym == SYM_BEGIN)
	{ // block
		getsym();
		set1 = createset(SYM_SEMICOLON, SYM_END, SYM_NULL);
		set = uniteset(set1, fsys);
		statement(set);
		while (sym == SYM_SEMICOLON || inset(sym, statbegsys))
		{
			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(10);
			}
			statement(set);
		} // while
		destroyset(set1);
		destroyset(set);
		if (sym == SYM_END)
		{
			getsym();
		}
		else
		{
			error(17); // ';' or 'end' expected.
		}
	}
	else if (sym == SYM_BREAK)
	{
		getsym();
		if (break_count <=0)
		{
			error(27); //can't break
		}
		else
		{
			break_cx[break_count] = cx;
			gen(JMP, 0, 0); //直接跳到循环体的后面；
		}								
	
	}
	else if(sym == SYM_EXIT)
	{
		getsym();
		gen(OPR, 0, OPR_RET);
	}
	else if (sym == SYM_WHILE)
	{ // while statement
		break_count++; //是否可以break
		cx1 = cx;
		getsym();
		set1 = createset(SYM_DO,SYM_AND,SYM_NOT,SYM_OR ,SYM_NULL);
		set = uniteset(set1, fsys);
		//condition(set);
		ex_condition(set);
		destroyset(set1);
		destroyset(set);

		//cx2 = cx;
		//gen(JPC, 0, 0);
		if (sym == SYM_DO)
		{
			getsym();
		}
		else
		{
			error(18); // 'do' expected.
		}
		/*所有真出口都跳到while-statement的前面*/
		for (int k = 0; k < true_count[condition_level];k++)
			code[true_out[condition_level][k]].a = cx;

		statement(fsys);
		gen(JMP, 0, cx1); //执行完循环体跳回条件前面判断
		
		/*所有假出口都跳到while-statement 后面*/
		for (int k = 0;k < false_count[condition_level];k++) 
			code[false_out[condition_level][k]].a = cx;
		condition_level--; //减小条件层次

		//如果存在break，则跳到该处
		if (break_cx[break_count] > 0)
		{
			code[break_cx[break_count]].a = cx;
			break_count--;
		}
	
		//code[cx2].a = cx;
	}
	else if (sym == SYM_READ) {			//处理读入数据
		getsym();

		if (sym != SYM_LPAREN)
			error(26);					//Missing '['.
		do {
			mask* mk;
			getsym();
			if(sym == SYM_IDENTIFIER){	//读取的数据是变量值
				if (!(i = position(id)))
					error(11);			//Undeclared identifier.
				else{
					mk = (mask*)& table[i];
					gen(READ, level - mk->level, mk->address);		//生成读取变量的READ指令
				}
			}
			else if (sym == SYM_ARRAY) {	//读取的数据是数组变量值
				dim = 0;
				if (!(i = array_position()))
					error(11);
				else {
					j = array_table[i].addr;
					mk = (mask*)&table[j];
					gen(LIT, 0, 0);
					while (ch == '[') {
						dim++;
						getch();
						getsym();
						set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
						expression(set);
						destroyset(set);
						gen(LIT, 0, array_table[i].size[dim - 1]);
						gen(OPR, 0, OPR_MUL);
						gen(OPR, 0, OPR_ADD);
					}
					gen(RDA, level - mk->level, mk->address);		//生成读取数组的RDA指令
				}
			}
			getsym();
		} while (sym == SYM_COMMA);		//如果变量之间仍有逗号表示未读完，继续读
		if (sym == SYM_RPAREN)
			getsym();
		else
			error(22);
	}
	else if (sym == SYM_WRITE) {		//处理打印数据
		getsym();
		if (sym != SYM_LPAREN)
			error(26);
		do {
			getsym();
			if (sym == SYM_RPAREN) {	//如果读取完左括号，下一个字符读到的是右括号，表示该过程为打印回车
				gen(OPR, 0, OPR_WEN);	//生成OPR,0,14打印回车符，并break退出
				break;
			}
			else if (sym == SYM_IDENTIFIER) {	//打印的内容为const常量或变量
				if (!(i = position(id)))
					error(11);
			/*	else if (table[i].kind != ID_VARIABLE) {		//注释原来的错误提示以使得print过程可以输出常量值
					error(12);
					i = 0;
				} */
				mk = (mask *)&table[i];
				if (table[i].kind == ID_CONSTANT) {				//输出constant常量值
					gen(LIT, 0, table[i].value);
					gen(OPR, 0, OPR_WRITE);						//生成OPR,0,15打印常量值
				}
				else if (i)										//若为变量，用WRITE指令
					gen(WRITE, level - mk->level, mk->address);	
			}
			else if (sym == SYM_ARRAY) {						//若为数组
				dim = 0;
				if (!(i = array_position()))
					error(11);
				else {
					j = array_table[i].addr;
					mk = (mask *)&table[j];
					gen(LIT, 0, 0);
					while (ch == '[') {
						dim++;
						getch();
						getsym();
						set = uniteset(createset(SYM_RSPAREN, SYM_NULL), fsys);
						expression(set);
						destroyset(set);
						gen(LIT, 0, array_table[i].size[dim - 1]);
						gen(OPR, 0, OPR_MUL);
						gen(OPR, 0, OPR_ADD);
					}
					gen(WTA, level - mk->level, mk->address);	//生成WTA指令打印所在层及偏移地址的数组值
				}
			}
			else if (sym == SYM_NUMBER) {						//如果是数字，直接取数到栈顶
				gen(LIT, 0, num);
				gen(OPR, 0, OPR_WRITE);							//生成OPR,0,15指令打印数字
			}
			getsym();
		} while (sym == SYM_COMMA);								//若中间为逗号，则继续打印下一个数
		if (sym == SYM_RPAREN)
			getsym();
		else
			error(22);
	}
	test(fsys, phi, 19);
} // statement
			
//////////////////////////////////////////////////////////////////////
void block(symset fsys)
{
	int cx0; // initial code index
	int tx0; // initial table index
	int dx1; // save data allocation index
	mask* mk;
	int block_dx;
	int savedTx;
	symset set1, set;

	dx = 3;
	tx0 = tx;
	block_dx = dx;
	mk = (mask*) &table[tx];
	mk->address = cx;
	gen(JMP, 0, 0);
	if (level > MAXLEVEL)
	{
		error(32); // There are too many levels.
	}
	do
	{
		if (sym == SYM_CONST)
		{ // constant declarations
			getsym();
			do
			{
				constdeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					constdeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
		} // if

		if (sym == SYM_VAR)
		{ // variable declarations
			getsym();
			do
			{
				vardeclaration();
				while (sym == SYM_COMMA)
				{
					getsym();
					vardeclaration();
				}
				if (sym == SYM_SEMICOLON)
				{
					getsym();
				}
				else
				{
					error(5); // Missing ',' or ';'.
				}
			}
			while (sym == SYM_IDENTIFIER);
//			block = dx;
		} // if

		while (sym == SYM_PROCEDURE)
		{ // procedure declarations
			getsym();
			if (sym == SYM_IDENTIFIER)
			{
				enter(ID_PROCEDURE);
				getsym();
			}
			else
			{
				error(4); // There must be an identifier to follow 'const', 'var', or 'procedure'.
			}


			if (sym == SYM_SEMICOLON)
			{
				getsym();
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}

			level++;
			savedTx = tx;
			dx1 = dx;
			set1 = createset(SYM_SEMICOLON, SYM_NULL);
			set = uniteset(set1, fsys);
			block(set);
			destroyset(set1);
			destroyset(set);
			tx = savedTx;
			dx = dx1;
			level--;		

			if (sym == SYM_SEMICOLON)
			{
				getsym();
				set1 = createset(SYM_IDENTIFIER, SYM_PROCEDURE, SYM_NULL);
				set = uniteset(statbegsys, set1);
				test(set, fsys, 6);
				destroyset(set1);
				destroyset(set);
			}
			else
			{
				error(5); // Missing ',' or ';'.
			}
		 } // while
		set1 = createset(SYM_IDENTIFIER, SYM_NULL);
		set = uniteset(statbegsys, set1);
		test(set, declbegsys, 7);
		destroyset(set1);
		destroyset(set);
	}
	while (inset(sym, declbegsys));

	code[mk->address].a = cx;
	mk->address = cx;
	cx0 = cx;
	gen(INT, 0, block_dx);		//dx1?
	set1 = createset(SYM_SEMICOLON, SYM_END, SYM_ELSE,SYM_RPAREN, SYM_NULL);
	set = uniteset(set1, fsys);
	statement(set);				//调用statement函数
	destroyset(set1);
	destroyset(set);
	gen(OPR, 0, OPR_RET); // return
	test(fsys, phi, 8); // test for error: Follow the statement is an incorrect symbol.
//	listcode(cx0, cx);
} // block

//////////////////////////////////////////////////////////////////////
int base(int stack[], int currentLevel, int levelDiff)
{
	int b = currentLevel;
	
	while (levelDiff--)
		b = stack[b];
	return b;
} // base

//////////////////////////////////////////////////////////////////////
// interprets and executes codes.
void interpret()
{
	int pc;        // program counter
	int stack[STACKSIZE];
	int top;       // top of stack
	int b;         // program, base, and top-stack register
	instruction i; // instruction register

	int input, output;
	printf("Begin executing PL/0 program.\n");

	pc = 0;
	b = 1;
	top = 3;
	stack[1] = stack[2] = stack[3] = 0;
	do
	{
		i = code[pc++];
		switch (i.f)
		{
		case LIT:
			stack[++top] = i.a;
			break;
		case OPR:
			switch (i.a) // operator
			{
			case OPR_RET:
				top = b - 1;
				pc = stack[top + 3];
				b = stack[top + 2];
				break;
			case OPR_NEG:
				stack[top] = -stack[top];
				break;
			case OPR_NOT:
				stack[top] = !stack[top];
				break;
			case OPR_ADD:
				top--;
				stack[top] += stack[top + 1];
				break;
			case OPR_MIN:
				top--;
				stack[top] -= stack[top + 1];
				break;
			case OPR_MOD:
				top--;
				stack[top] %= stack[top + 1];
				break;
			case OPR_MUL:
				top--;
				stack[top] *= stack[top + 1];
				break;
			case OPR_DIV:
				top--;
				if (stack[top + 1] == 0)
				{
					fprintf(stderr, "Runtime Error: Divided by zero.\n");
					fprintf(stderr, "Program terminated.\n");
					continue;
				}
				stack[top] /= stack[top + 1];
				break;
			case OPR_ODD:
				stack[top] %= 2;
				break;
			case OPR_EQU:
				top--;
				stack[top] = stack[top] == stack[top + 1];
				break;
			case OPR_NEQ:
				top--;
				stack[top] = stack[top] != stack[top + 1];
			case OPR_LES:
				top--;
				stack[top] = stack[top] < stack[top + 1];
				break;
			case OPR_GEQ:
				top--;
				stack[top] = stack[top] >= stack[top + 1];
			case OPR_GTR:
				top--;
				stack[top] = stack[top] > stack[top + 1];
				break;
			case OPR_LEQ:
				top--;
				stack[top] = stack[top] <= stack[top + 1];
			case OPR_WEN:		//打印回车
				printf("\n");
				break;
			case OPR_WRITE:
				printf("%d\t", stack[top]);		//打印数字或常量，后加制表符
				top--;
				break;
			} // switch
			break;
		case LOD:
			stack[++top] = stack[base(stack, b, i.l) + i.a];
			break;
		case STO:
			stack[base(stack, b, i.l) + i.a] = stack[top];
			//printf("%d\n", stack[top]);
			top--;
			break;
		case CAL:
			stack[top + 1] = base(stack, b, i.l);
			// generate new block mark
			stack[top + 2] = b;
			stack[top + 3] = pc;
			b = top + 1;
			pc = i.a;
			break;
		case INT:
			top += i.a;
			break;
		case JMP:
			pc = i.a;
			break;
		case JPC:
			if (stack[top] <= 0)
				pc = i.a;
			top--;
			break;
		case JPNC:
			if (stack[top] > 0)
				pc = i.a;
			top--;
			break;
		case READ:					//读取变量
			scanf("%d", &input);
			stack[base(stack, b, i.l) + i.a] = input;
			break;
		case WRITE:					//打印变量
			printf("%d\t", stack[base(stack, b, i.l) + i.a]);
			break;
		case LDA:					//加载数组
			stack[top] = stack[base(stack, b, i.l) + i.a + stack[top]];
			break;
		case STA:					//存储数组
			stack[base(stack, b, i.l) + i.a + stack[top - 1]] = stack[top];
			top--;
			break;
		case WTA:					//打印数组值，后加制表符
			printf("%d\t", stack[base(stack, b, i.l) + i.a + stack[top]]);
			break;
		case RDA:					//读取数组变量值
			scanf("%d", &input);
			stack[base(stack, b, i.l) + i.a + stack[top]] = input;
			break;
		defalt:
			fprintf(stderr, "Runtime error: unexpected instruction.\n");
		} // switch
	}
	while (pc);

	printf("End executing PL/0 program.\n");
} // interpret

//////////////////////////////////////////////////////////////////////
void main ()
{
	FILE* hbin;
	char s[80];
	int i;
	symset set, set1, set2;

	printf("Please input source file name: "); // get file name to be compiled
	scanf("%s", s);
	if ((infile = fopen(s, "r")) == NULL)
	{
		printf("File %s can't be opened.\n", s);
		exit(1);
	}

	phi = createset(SYM_NULL);
	relset = createset(SYM_EQU, SYM_NEQ, SYM_LES, SYM_LEQ, SYM_GTR, SYM_GEQ, SYM_NULL);
	
	// create begin symbol sets
	declbegsys = createset(SYM_CONST, SYM_VAR, SYM_PROCEDURE, SYM_NULL);
	statbegsys = createset(SYM_BEGIN, SYM_CALL, SYM_IF, SYM_WHILE,SYM_FOR ,SYM_IDENTIFIER,SYM_BREAK,SYM_EXIT,SYM_NULL);
	//因子的FIST集中增加数组类型
	facbegsys = createset(SYM_IDENTIFIER, SYM_NUMBER, SYM_LPAREN, SYM_ARRAY, SYM_NULL);

	err = cc = cx = ll = 0; // initialize global variables
	ch = ' ';
	kk = MAXIDLEN;

	getsym();

	set1 = createset(SYM_PERIOD, SYM_NULL);
	set2 = uniteset(declbegsys, statbegsys);
	set = uniteset(set1, set2);
	block(set);
	destroyset(set1);
	destroyset(set2);
	destroyset(set);
	destroyset(phi);
	destroyset(relset);
	destroyset(declbegsys);
	destroyset(statbegsys);
	destroyset(facbegsys);

	if (sym != SYM_PERIOD)
		error(9); // '.' expected.
	if (err == 0)
	{
		hbin = fopen("hbin.txt", "w");
		for (i = 0; i < cx; i++)
			fwrite(&code[i], sizeof(instruction), 1, hbin);
		fclose(hbin);
	}
	if (err == 0)
		interpret();
	else
		printf("There are %d error(s) in PL/0 program.\n", err);
	listcode(0, cx);
	getchar();
} // main

//////////////////////////////////////////////////////////////////////
// eof pl0.c
