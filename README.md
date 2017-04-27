# PL0-Compiler
编译原理大作业

#### 一.扩展功能
1. 注释；		
2. 扩展条件及短路计算； 
3. else语句实现;		
4. for语句实现;		
5. exit及break语句实现;	
6. 多维数组实现；		
7. 输入输出语句实现。

#### 二. PL0.h修改
1. 指令添加：(10条)
> (1)opcode:
+ JPNC、LDA、STA、RDA、WTA、READ、WRITE
> (2)oprcode:
+ OPR_NOT、OPR_WEN、OPR_WRITE

2. 关键字添加:  (9个)
+ and、or、 not、 else、 for、 break
+ exit、 read、 print 

3. 添加数组相关参数：
+ (1) symtype和idtype类型：SYM_ARRAY、SYM_LSPAREN、SYMRSPAREN、ID_ARRAY ；
+ (2) 数组结构:
```
typedef struct
{
char name[MAXIDLEN + 1];//数组变量名称
int  sum;//数组
int  n;//数组总维数
int  dim[MAXDIM];//数组对应维数的存储空间
int  size[MAXDIM];//数组对应维数的地址偏移量大小
int  addr;//数组首地址
}arr;
```

4. 短路计算相关参数:
+ (1) true_out[4][10] :  真值链
+ (2) false_out[4][10]:  假值链
+ (3) true_count[4]、false_count[4]:   当前层要跳到真、假出口的指令数目
+ (4) condition_level:   记录当前条件嵌套层数

5. break语句相关参数：
+ break_count:	break的层数
+ break_cx[4]:	当前层break后应该跳转到的指令

#### 三. PL0.c代码分析

 各函数修改情况分析：
+ getsym()函数：遇‘[’即判定为数组类型，处理‘/*…*/’和‘//’注释语句，处理‘]’符号；
+ array_enter()函数：记录数组初始地址，并调用enter()函数将数组的所有存储空间填入符号表中；
+ array_position()函数：定位数组变量在数组符号表中的位置；
+ vardeclaration()函数：增加了对数组声明的处理；
+ factor()函数：增加了对数组情况的处理，并在factor、term和expression的FIRST集合中添加了数组类型；
+ ex_condition()函数：处理短路计算，判断各真假值链出口，并调用condition()函数；
+ statement()函数：增加了对数组情况、if、for、break、while、read、write语句的处理
+ interpret()函数：增加了对所增加的指令语句的处理；



