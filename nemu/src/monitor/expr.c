#include "nemu.h"
#include "cpu/reg.h"
#include "memory/memory.h"
#include "cpu/cpu.h"
#include <elf.h>
#include <stdlib.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <sys/types.h>
#include <regex.h>

enum
{
	NOTYPE = 256,
	LEQ,// less or equal
	MEQ,// more or equal
	EQ,// equal
	NEQ,// not equal
	DEC,//dec number
	HEX,//hex number
	REG,
	AND,// &&
	OR,// ||
	
	DEREF,//指针解引用
	NEG,//取负
	POS,//正号
	
	SYMB// symbol
	/* TODO: Add more token types */
};

static struct rule//自定义一套token的集合，一开始进行init_regex()初始化时编译到re[NR_REGEX]里存储
{
	char *regex;
	int token_type;
} rules[] = {

	/* TODO: Add more rules.
	 * Pay attention to the precedence level of different rules.
	 */
	{" +", NOTYPE}, // white space
	
	{"\\+", '+'},
	{"-", '-'},//////////////////需要进行再判断，减法还是取负
	{"\\*", '*'},///////////////需要进行再判断，乘法还是指针解引用
	{"/", '/'},
	
	{"\\(", '('},
	{"\\)", ')'},
	
	{"==", EQ},//equal
	{"!=", NEQ},//not equal
	{"<=", LEQ},//less or equal
	{">=", MEQ},//more or equal
	{">", '>'},
	{"<", '<'},
	{"!", '!'},
	
	{"&&", AND},
	{"\\|\\|", OR},
	
	{"0[xX][0-9a-fA-F]+", HEX},// hex number
	{"[0-9]+", DEC},// dec number
	
	{"\\$[a-zA-Z]+", REG},
	{"[a-zA-Z_][a-zA-Z0-9_]*", SYMB}//symbol
};

#define NR_REGEX (sizeof(rules) / sizeof(rules[0]))

static regex_t re[NR_REGEX];//存储编译初始化后的正则表达式

/* Rules are used for more times.
 * Therefore we compile them only once before any usage.
 */
void init_regex()//初始化正则表达式
{
	int i;
	char error_msg[128];
	int ret;

	for (i = 0; i < NR_REGEX; i++)
	{
		ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);//将正则表达式字符串编译为内部格式，以便后续匹配操作
		if (ret != 0)
		{
			regerror(ret, &re[i], error_msg, 128);//返回错误信息
			assert(ret != 0);
		}
	}
}

typedef struct token
{
	int type;
	char str[32];
} Token;

Token tokens[32];//按顺序存储已经被识别出的token信息
int nr_token;//标记已经被识别出的token数目

static bool make_token(char *e)//把识别出的token信息 “写入” Token tokens[32]中存储
{
	int position = 0;
	int i;
	regmatch_t pmatch;

	nr_token = 0;

	while (e[position] != '\0')
	{
		/* Try all rules one by one. */
		for (i = 0; i < NR_REGEX; i++)
		{
			if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0)//把token的类型匹配到位（仅使得i和rules里面的次序相同)
			{
				char *substr_start = e + position;
				int substr_len = pmatch.rm_eo;

				printf("match regex[%d] at position %d with len %d: %.*s\n", i, position, substr_len, substr_len, substr_start);
				position += substr_len;

				/* TODO: Now a new token is recognized with rules[i]. 
				 * Add codes to perform some actions with this token.
				 */

				switch (rules[i].token_type)//对token的type进行填充，内容的str字符串也手动匹配到位
				{
				    case NOTYPE:
				        break;
				    case HEX:
				        tokens[nr_token].type = rules[i].token_type;
				        for(int j = 2; j < substr_len; j++)
				        {
				            tokens[nr_token].str[j - 2] = substr_start[j];
				        }
				        tokens[nr_token].str[substr_len - 2] = '\0';
				        nr_token++;
				        break;
				    case DEC:
				    case REG:
				    case SYMB:
				        tokens[nr_token].type = rules[i].token_type;
				        for(int j = 0; j < substr_len; j++)
				        {
				            tokens[nr_token].str[j] = substr_start[j];
				        }
				        tokens[nr_token].str[substr_len] = '\0';
				        nr_token++;
				        break;
				    default:
					    tokens[nr_token].type = rules[i].token_type;
					    nr_token++;
				}

				break;
			}
		}

		if (i == NR_REGEX)
		{
			printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
			return false;
		}
	}

	return true;
}

static bool check_parentheses(int p, int q) //判断子串是否完全由一个大括号包裹
{
    if (tokens[p].type != '(' || tokens[q].type != ')') {
        // 如果子表达式的起始和结束不是括号，直接返回 false
        return false;
    }

    int temp = 0;
    int i = 0;
    for (i = p; i <= q; i++) {
        if (tokens[i].type == '(') {
            temp++;
        } else if (tokens[i].type == ')') {
            temp--;
        }
    }
    // 如果 temp 不为 0，说明括号不匹配
    if (temp != 0) {
        assert(0);
    }

    // 检查括号是否完全包裹整个子表达式
    int cnt = 0;
    for (i = p + 1; i <= q - 1; i++) {
        if (tokens[i].type == '(') {
            cnt++;
        } else if (tokens[i].type == ')') {
            cnt--;
        }
        if (cnt < 0) {
            // 如果 cnt 小于 0，说明括号不匹配
            return false;
        }
    }

    return true;
}

static int dec_trans(char str[]) //把DEC类型的字符转换为数字
{
    int i = 0, a = 0;
    
    while(str[i] != '\0')
    {
        a *= 10;
        a += str[i] - '0';
        i++;
    }
    return a;
}
static int hex_trans(char str[]) //把HEX类型的字符转换为数字
{
    int i = 0, a = 0;
    
    while(str[i] != '\0')
    {
        a *= 16;
        if(str[i] >= '0' && str[i] <= '9')
        {
            a += str[i] - '0';
            i++;
        }
        else if(str[i] >= 'a' && str[i] <= 'f')
        {
            a += str[i] - 'a' + 10;
            i++;
        }
    }
    return a;
}

static uint32_t reg_trans(char *str)//取出reg的内容
{
    if(!strcmp(str, "$eax")) return cpu.eax;
    else if(!strcmp(str, "$ebx")) return cpu.ebx;
    else if(!strcmp(str, "$ecx")) return cpu.ecx;
    else if(!strcmp(str, "$edx")) return cpu.edx;
    else if(!strcmp(str, "$esi")) return cpu.esi;
    else if(!strcmp(str, "$edi")) return cpu.edi;
    else if(!strcmp(str, "$ebp")) return cpu.ebp;
    else if(!strcmp(str, "$esp")) return cpu.esp;
    else if(!strcmp(str, "$eip")) return cpu.eip;
    else assert(0);
}

extern char *strtab;
extern Elf32_Sym *symtab;
extern int nr_symtab_entry;

static uint32_t symb_trans(char *str)//取出符号的地址
{
    int i = 0;
    for(i = 0; i < nr_symtab_entry; i++)
    {
        if(!strcmp(strtab + symtab[i].st_name, str)) break;
    }
    return symtab[i].st_value;
}

static int type_priority(int type)//只含有最繁杂且普通的运算符
{
    switch(type)
    {
		case '!':
		case NEG:
		case POS:
		case DEREF: return 6;
		case '*':
		case '/': return 5;
		case '+':
		case '-': return 4;
		case '<':
		case '>':
		case LEQ:
		case MEQ: return 3;
		case EQ:
		case NEQ: return 2;
		case AND: return 1;
		case OR: return 0;
		default: return -1;
    }
}


static uint32_t eval(int p, int q)
{
    if(p > q) {
        /* Bad expression */
        return 0;
    }
    else if(p == q) {
        /* Single token.
         * For now this token should be a number.
         * Return the value of the number.
         */ 
        if(tokens[p].type == DEC) return dec_trans(tokens[p].str);
        else if(tokens[p].type == HEX) return hex_trans(tokens[p].str);
        else if(tokens[p].type == REG) return reg_trans(tokens[p].str);
        else if (tokens[p].type == SYMB) return symb_trans(tokens[p].str);
        else assert(0);
    }
    else if(check_parentheses(p, q) == true) {
        /* The expression is surrounded by a matched pair of parentheses. 
         * If that is the case, just throw away the parentheses.
         */
        return eval(p + 1, q - 1); 
    }
    else {
        //op = the position of dominant operator in the token expression;
        int op = -1;
        int min_priority = -1;
        int stack = 0; // Stack to handle parentheses

        // Find the dominant operator
        for (int i = q; i >= p; i--) {
            if (tokens[i].type == '(') {
                stack++;
            } else if (tokens[i].type == ')') {
                stack--;
            } else if (stack == 0) {
                int priority = type_priority(tokens[i].type);
                if (priority >= 0 && (op == -1 || priority < min_priority)) {
                    op = i;
                    min_priority = priority;
                }
            }
        }

        if (op == -1) {
            // No valid operator found
            assert(0);
        }
        
        // printf("\nop:%d\n", op);
        uint32_t val1 = eval(p, op - 1);
        uint32_t val2 = eval(op + 1, q);
        switch(tokens[op].type)
        {
            case '+':   return val1 + val2;
            case '-':   return val1 - val2;
            case '*':   return val1 * val2;
            case '/':   return val1 / val2;
            case '!':   return !val2;
            case DEREF: return paddr_read(val2, 4);
            case NEG:   return -val2;
            case POS:   return val2;
            case EQ:    return val1 == val2;
            case NEQ:   return val1 != val2;
            case LEQ:   return val1 <= val2;
            case MEQ:   return val1 >= val2;
            case '>':   return val1 > val2;
            case '<':   return val1 < val2;
            case AND:   return val1 && val2;
            case OR:    return val1 || val2;

            default:
            printf("bug: \nop_type:%d\n", tokens[op].type);
            return -1;
        }
    }
}


uint32_t expr(char *e, bool *success)
{
	if (!make_token(e))
	{
		*success = false;
		return 0;
	}
    
    //对*和-和+进行再判断，确定真实类型
    for(int i = 0; i < nr_token; i++)
    {
        if(tokens[i].type == '*')
        {
            if(i == 0) tokens[i].type = DEREF;
            else if(tokens[i - 1].type == HEX || tokens[i - 1].type == DEC ||
            tokens[i - 1].type == REG || tokens[i - 1].type == SYMB || tokens[i - 1].type == ')' ) ;
            else tokens[i].type = DEREF;
        }
        if(tokens[i].type == '-')
        {
            if(i == 0) tokens[i].type = NEG;
            else if(tokens[i - 1].type == HEX || tokens[i - 1].type == DEC ||
            tokens[i - 1].type == REG || tokens[i - 1].type == SYMB || tokens[i - 1].type == ')' ) ;
            else tokens[i].type = NEG;
        }
        if(tokens[i].type == '+')
        {
            if(i == 0) tokens[i].type = POS;
            else if(tokens[i - 1].type == HEX || tokens[i - 1].type == DEC ||
            tokens[i - 1].type == REG || tokens[i - 1].type == SYMB || tokens[i - 1].type == ')' ) ;
            else tokens[i].type = POS;
        }
    }
    
    *success = true;
    return eval(0, nr_token - 1);
}
