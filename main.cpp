﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>


typedef int Status;
#define OK 1
#define ERROR 0


// ANSI颜色代码
#define RESET   "\033[0m"
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

// 文本样式
#define BOLD      "\033[1m"
#define UNDERLINE "\033[4m"

// 清屏
#define CLEAR_SCREEN "\033[2J\033[H"

// 使用状态机模式
typedef enum
{
	MAIN_MENU,
	LOGIN,
	REGISTER,
	FORGOT_PASSWORD,
	USER_MENU,
	ADMIN_MENU,
	EXIT
}SystemState;

// 用户结构体
typedef struct User
{
	char username[20];
	char password[20];
	int memberLevel;	// 会员等级，0 表示管理员
	int points;			// 积分
	struct User* next;
} UserNode, * UserList, * CurrUser;


void printSeparator(const char* separator_char, const char* color, int width);
void printCentered(const char* text, const char* color, int console_width);
void printHeader();
void cls();

int getConsoleWidth();

Status initUserList(UserList& L);
Status appendUserList(UserList& L, char* username, char* password, int memberLevel, int points);
Status readUserFromFile(const char* filename, UserList& L);

SystemState mainMenu();
SystemState userLogin(UserList &L);
Status userExist(UserList L, char* username);
SystemState userRegister(UserList& L);

// 测试链表
//Status printUserList(UserList L)
//{
//	UserList p = L->next;
//	while (p)
//	{
//		printf("%-20s %-20s %d %d\n", p->username, p->password, p->memberLevel, p->points);
//		p = p->next;
//	}
//	return OK;
//}


int main()
{
	// 初始化 UserList 并从文件中读取用户信息
	UserList L;
	initUserList(L);
	readUserFromFile("user.txt", L);

	// 使用状态机模式管理程序流程
	SystemState currState = MAIN_MENU;
	UserList currUser = NULL;

	while (currState != EXIT)
	{
		switch (currState)
		{
			case MAIN_MENU:
				currState = mainMenu();
				break;
			case LOGIN:
				currState = userLogin(L);
				break;
		}
	}

	return OK;
}


void printHeader()
{
	int width = getConsoleWidth();

	printSeparator("=", CYAN, width);
	printCentered("无人超市管理系统", WHITE, width);
	printSeparator("=", CYAN, width);
}

void cls()
{
	printf("%s", CLEAR_SCREEN);
}

// 打印一行分隔符
void printSeparator(const char* separator_char, const char* color, int width)
{
	int i;

	printf("%s", color);
	for (i = 0; i < width; i++)
	{
		printf("%c", *separator_char);
	}
	printf("%s\n", RESET);
}

// 打印居中字符串
void printCentered(const char* text, const char* color, int console_width)
{
	size_t text_len = strlen(text);
	int padding = (console_width - text_len) / 2;
	int i;

	for (i = 0; i < padding; i++)
	{
		printf(" ");
	}
	printf("%s%s%s\n", color, text, RESET);
}

// 使用 Windows.h 中的 GetConsoleScreenBufferInfo 获取终端宽度
int getConsoleWidth() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int width = 80; // 默认宽度

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }

    return width;
}

// 初始化 UserList
Status initUserList(UserList& L)
{
	L = new UserNode;
	L->next = NULL;
	return OK;
}

// 向 UserList 的末尾添加一个节点
Status appendUserList(UserList& L, char* username, char* password, int memberLevel, int points)
{
	UserList p, newNode;

	// 新建一个节点
	newNode = new UserNode;
	strcpy(newNode->username, username);
	strcpy(newNode->password, password);
	newNode->memberLevel = memberLevel;
	newNode->points = points;
	newNode->next = NULL;

	// 将新节点添加到链表末尾
	p = L;
	while (p->next)
	{
		p = p->next;
	}
	p->next = newNode;

	return OK;
}

// 从文件读取用户数据并存储到 UserList 中
Status readUserFromFile(const char* filename, UserList& L)
{
	FILE* fp;
	if ((fp = fopen(filename, "r")) == NULL)
	{
		return ERROR;
	}

	char username[20], password[20];
	int memberLevel, points;

	while (fscanf(fp, "%s %s %d %d", username, password, &memberLevel, &points) != EOF)
	{
		appendUserList(L, username, password, memberLevel, points);
	}

	fclose(fp);
	return OK;
}

// 主菜单
SystemState mainMenu()
{
	int console_width = getConsoleWidth();
	int choice = -1;

	// 清屏  
	cls();

	// 打印标题  
	printSeparator("=", CYAN, console_width);
	// ASCII Art  
	printf("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗██╗\n");
	printf("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝██║\n");
	printf("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗  ██║\n");
	printf("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝  ╚═╝\n");
	printf("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗██╗\n");
	printf(" ╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝╚═╝\n");

	printSeparator("=", CYAN, console_width);
	printCentered("欢迎光临无人超市！", BOLD WHITE, console_width);
	printSeparator("-", CYAN, console_width);

	printf("%s您当前未登录！%s\n", BOLD YELLOW, RESET);

	// 操作选择  
	printf("请选择要进行的操作：\n");
	printf("1. 用户登录\n");
	printf("2. 用户注册\n");
	printf("3. 找回密码\n");
	printf("0. 退出系统\n");
	printf("%s输入序号进行对应操作：%s", YELLOW, RESET);
	scanf("%d", &choice);

	// 根据用户选择返回对应状态  
	switch (choice)
	{
	case 1:
		return LOGIN;
	case 2:
		return REGISTER;
	case 3:
		return FORGOT_PASSWORD;
	case 0:
		return EXIT;
	default:
		printf("%s输入的序号不存在，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return MAIN_MENU;
	}
}

// 用户登录
SystemState userLogin(UserList& L)
{
	cls();
	printHeader();

	char username[20] = { 0 }, password[20] = { 0 };
	printf("请输入用户名: ");
	scanf("%s", username);
	printf("请输入密码: ");
	scanf("%s", password);

	UserList CurrUser = L->next;
	while (CurrUser)
	{
		if (strcmp(CurrUser->username, username) == 0 && strcmp(CurrUser->password, password) == 0)
		{
			if (CurrUser->memberLevel)
			{
				printf("%s用户登录成功！欢迎您，%s！\n您的会员等级：%d\n剩余积分：%d%s\n", BOLD GREEN, CurrUser->username, CurrUser->memberLevel, CurrUser->points, RESET);
				return USER_MENU;
			}
			else
			{
				printf("%s管理员登录成功！欢迎您，%s！%s\n", BOLD GREEN, CurrUser->username, RESET);
				return ADMIN_MENU;
			}
			Sleep(1000);
		}
		CurrUser = CurrUser->next;
	}

	printf("%s用户名或密码错误，请重试。%s\n", BOLD RED, RESET);
	Sleep(1000);
	return LOGIN;
}

// 判断用户是否存在
Status userExist(UserList L, char* username)
{
	UserList p = L->next;
	while (p)
	{
		if (strcmp(p->username, username) == 0)
		{
			return OK;
		}
		p = p->next;
	}
	return ERROR;
}

// 用户注册
SystemState userRegister(UserList& L)
{
	cls();
	printHeader();
	char username[20] = { 0 }, password[20] = { 0 };
	printf("请输入用户名: ");
	scanf("%s", username);
	// 验证用户名是否已经存在
	if (userExist(L, username))
	{
		printf("%s用户名已存在，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return REGISTER;
	}
	
	printf("请输入密码: ");
	scanf("%s", password);

}
