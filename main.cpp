﻿#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <Windows.h>


typedef int Status;
#define OK 1
#define ERROR 0

#define USER_FILE "user.txt"

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

// 使用状态机模式
typedef enum
{
	MAIN_MENU,
	LOGIN,
	REGISTER,
	FORGOT_PASSWORD,
	USER_MENU,
	ADMIN_MENU,
	VIEW_PRODUCTS,
	SEARCH_PRODUCTS,
	SHOPPING_CART,
	CHECKOUT,
	SHOPPING_HISTORY,
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
} UserNode, * UserList;


void printSeparator(const char* separator_char, const char* color, int width);
void printCentered(const char* text, const char* color, int console_width);
void printHeader();

int getConsoleWidth();

Status initUserList(UserList& L);
Status appendUserList(UserList& L, char* username, char* password, int memberLevel, int points);
Status readUserFromFile(const char* filename, UserList& L);
Status saveUserToFile(const char* filename, UserList L);
Status userExist(UserList L, char* username);
Status updatePassword(UserList L, char* username, char* newPassword);

SystemState mainMenu();

SystemState userLogin(UserList &L);
SystemState userRegister(UserList& L);
SystemState forgotPassword(UserList& L);

SystemState userMenu();

// 测试链表
Status printUserList(UserList L)
{
	UserList p = L->next;
	while (p)
	{
		printf("%-20s %-20s %d %d\n", p->username, p->password, p->memberLevel, p->points);
		p = p->next;
	}
	return OK;
}

// 定义全局变量，指示当前用户
UserList currUser = NULL;

int main()
{
	// 初始化 UserList 并从文件中读取用户信息
	UserList User_L;
	initUserList(User_L);
	readUserFromFile(USER_FILE, User_L);

	// 使用状态机模式管理程序流程
	SystemState currState = MAIN_MENU;

	while (currState != EXIT)
	{
		switch (currState)
		{
			case MAIN_MENU:
				currState = mainMenu();
				break;
			case LOGIN:
				currState = userLogin(User_L);
				break;
			case REGISTER:
				currState = userRegister(User_L);
				break;
			case FORGOT_PASSWORD:
				currState = forgotPassword(User_L);
				break;
			case USER_MENU:
				currState = userMenu();
				break;
		}
	}

	return OK;
}


void printHeader()
{
	int width = getConsoleWidth();

	printSeparator("=", CYAN, width);
	printCentered("欢迎光临一次买够！！！！！", WHITE, width);
	printSeparator("=", CYAN, width);
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

// 将 UserList 中的用户数据保存到文件
Status saveUserToFile(const char* filename, UserList L)
{
	FILE* fp;
	UserList p = L->next;

	if ((fp = fopen(filename, "w")) == NULL)
	{
		return ERROR;
	}

	while (p)
	{
		fprintf(fp, "%s %s %d %d\n", p->username, p->password, p->memberLevel, p->points);
		p = p->next;
	}
	fclose(fp);

	return OK;
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

// 修改用户密码
Status updatePassword(UserList L, char* username, char* newPassword)
{
	UserList p = L->next;
	while (p)
	{
		if (strcmp(p->username, username) == 0)
		{
			strcpy(p->password, newPassword);
			return OK;
		}
		p = p->next;
	}
	return ERROR;
}

// 主菜单
SystemState mainMenu()
{
	int console_width = getConsoleWidth();
	int choice = -1;

	// 清屏  
	system("cls");

	// 打印标题  
	// printSeparator("=", CYAN, console_width);
	// ASCII Art  
	printf("██╗    ██╗███████╗██╗      ██████╗ ██████╗ ███╗   ███╗███████╗\n");
	printf("██║    ██║██╔════╝██║     ██╔════╝██╔═══██╗████╗ ████║██╔════╝\n");
	printf("██║ █╗ ██║█████╗  ██║     ██║     ██║   ██║██╔████╔██║█████╗  \n");
	printf("██║███╗██║██╔══╝  ██║     ██║     ██║   ██║██║╚██╔╝██║██╔══╝  \n");
	printf("╚███╔███╔╝███████╗███████╗╚██████╗╚██████╔╝██║ ╚═╝ ██║███████╗\n");
	printf(" ╚══╝╚══╝ ╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚══════╝\n");

	//printSeparator("=", CYAN, console_width);
	printHeader();

	printf("%s您当前未登录！登录以开始购物%s\n", BOLD YELLOW, RESET);

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
	case 100:
		return USER_MENU;  // 仅供测试使用
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
	system("cls");
	printHeader();

	char c;
	char username[20] = { 0 }, password[20] = { 0 };
	printf("请输入用户名（输入 0 返回主菜单）: ");
	scanf("%19s", username);
	if (strcmp(username, "0") == 0)
	{
		return MAIN_MENU;
	}
	printf("请输入密码: ");
	while (1)
	{
		c = _getch();
		if (c == '\r')
		{
			break;
		}
		else if (c == '\b')
		{
			printf("\b \b");
			if (strlen(password) > 0)
			{
				password[strlen(password) - 1] = '\0';
			}
		}
		else
		{
			password[strlen(password)] = c;
			printf("*");
		}
	}
	printf("\n");

	UserList CurrUser = L->next;
	while (CurrUser)
	{
		if (strcmp(CurrUser->username, username) == 0 && strcmp(CurrUser->password, password) == 0)
		{
			currUser = CurrUser;
			if (CurrUser->memberLevel)
			{
				printf("%s用户登录成功！欢迎您，%s！%s\n", BOLD GREEN, CurrUser->username, RESET);
				Sleep(1000);
				return USER_MENU;
			}
			else
			{
				printf("%s管理员登录成功！欢迎您，%s！%s\n", BOLD GREEN, CurrUser->username, RESET);
				Sleep(1000);
				return ADMIN_MENU;
			}

		}
		CurrUser = CurrUser->next;
	}

	printf("%s用户名或密码错误，请重试。%s\n", BOLD RED, RESET);
	Sleep(1000);
	return LOGIN;
}

// 用户注册
SystemState userRegister(UserList& L)
{
	char c;
	char username[20] = { 0 }, password[20] = { 0 }, confirm_password[20] = { 0 };

	system("cls");
	printHeader();
	printCentered("用户注册", BOLD YELLOW, getConsoleWidth());
	printf("%s用户名及密码均不得超过 19 个字符！%s\n", BOLD YELLOW, RESET);
	printf("请输入用户名（输入 0 返回主菜单）: ");
	scanf("%19s", username);
	if (strcmp(username, "0") == 0)
	{
		return MAIN_MENU;
	}

	// 验证用户名是否已经存在
	if (userExist(L, username))
	{
		printf("%s用户名已存在，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return REGISTER;
	}
	
	printf("请输入密码: "); 
	while (1)
	{
		c = _getch();
		if (c == '\r')
		{
			break;
		}
		else if (c == '\b')
		{
			if (strlen(password) > 0)
			{
				printf("\b \b");
				password[strlen(password) - 1] = '\0';
			}
		}
		else
		{
			password[strlen(password)] = c;
			printf("*");
		}
	}
	printf("\n");
	printf("请确认密码: ");
	while (1)
	{
		c = _getch();
		if (c == '\r')
		{
			break;
		}
		else if (c == '\b')
		{
			if (strlen(confirm_password) > 0)
			{
				printf("\b \b");
				confirm_password[strlen(confirm_password) - 1] = '\0';
			}
		}
		else
		{
			confirm_password[strlen(confirm_password)] = c;
			printf("*");
		}
	}
	printf("\n");
	
	// 判断两次密码是否一致
	if (strcmp(password, confirm_password) == 0)
	{
		appendUserList(L, username, password, 1, 0);
		saveUserToFile(USER_FILE, L);
		printf("%s密码更改成功！正在跳转登录%s\n", BOLD GREEN, RESET);
		Sleep(1000);
		return LOGIN;
	}
	else
	{
		printf("%s两次输入的密码不一致，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return FORGOT_PASSWORD;
	}
}

// 找回密码
SystemState forgotPassword(UserList& L)
{
	char c;
	char username[20] = { 0 }, password[20] = { 0 }, confirm_password[20] = { 0 };
	char verification_code[7] = { 0 }, input_veri_code[7] = { 0 };
	int i;

	system("cls");
	printHeader();
	printCentered("找回密码", BOLD YELLOW, getConsoleWidth());
	printf("请输入用户名（输入 0 返回主菜单）: ");
	scanf("%19s", username);
	if (strcmp(username, "0") == 0)
	{
		return MAIN_MENU;
	}

	// 验证用户名是否已经存在
	if (!userExist(L, username))
	{
		printf("%s用户不存在，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return FORGOT_PASSWORD;
	}

	// 随机生成 6 位验证码模拟短信验证码
	srand((unsigned)time(NULL));
	for (i = 0; i < 6; i++)
	{
		verification_code[i] = '0' + rand() % 10;
	}
	printf("%s验证码已发送，请输入验证码（%s）: %s", YELLOW, verification_code, RESET);
	scanf("%6s", input_veri_code);
	if (strcmp(verification_code, input_veri_code) != 0)
	{
		printf("%s验证码错误，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return FORGOT_PASSWORD;
	}
	else
	{
		printf("%s验证码正确，请设置新密码：%s", BOLD GREEN, RESET);
	}

	while (1)
	{
		c = _getch();
		if (c == '\r')
		{
			break;
		}
		else if (c == '\b')
		{
			if (strlen(password) > 0)
			{
				printf("\b \b");
				password[strlen(password) - 1] = '\0';
			}
		}
		else
		{
			password[strlen(password)] = c;
			printf("*");
		}
	}
	printf("\n");
	printf("请确认新密码: ");
	while (1)
	{
		c = _getch();
		if (c == '\r')
		{
			break;
		}
		else if (c == '\b')
		{
			if (strlen(confirm_password) > 0)
			{
				printf("\b \b");
				confirm_password[strlen(confirm_password) - 1] = '\0';
			}
		}
		else
		{
			confirm_password[strlen(confirm_password)] = c;
			printf("*");
		}
	}
	printf("\n");

	// 判断两次密码是否一致
	if (strcmp(password, confirm_password) == 0)
	{
		updatePassword(L, username, password);
		saveUserToFile(USER_FILE, L);
		printf("%s注册成功！正在跳转登录%s\n", BOLD GREEN, RESET);
		Sleep(1000);
		return LOGIN;
	}
	else
	{
		printf("%s两次输入的密码不一致，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return FORGOT_PASSWORD;
	}
}

// 显示用户菜单
SystemState userMenu()
{
	UserList CurrUser = currUser;
	int choice = -1;

	// 显示欢迎信息
	system("cls");
	printHeader();
	printf("欢迎回来，%s！\n您的会员等级：%d，购物积分：%d\n", CurrUser->username, CurrUser->memberLevel, CurrUser->points);
	printSeparator("-", WHITE, getConsoleWidth());

	// 显示菜单选项
	printf("请选择要进行的操作：\n");
	printf("1. 浏览商品\n");
	printf("2. 搜索商品\n");
	printf("3. 查看购物车\n");
	printf("4. 自助结账\n");
	printf("5. 购物记录\n");
	printf("0. 退出登录\n");
	printf("%s输入序号进行对应操作：%s", YELLOW, RESET);
	scanf("%d", &choice);

	switch (choice)
	{
	case 1:
		return VIEW_PRODUCTS;
		break;
	case 2:
		return SEARCH_PRODUCTS;
		break;
	case 3:
		return SHOPPING_CART;
		break;
	case 4:
		return CHECKOUT;
		break;
	case 5:
		return SHOPPING_HISTORY;
		break;
	case 0:
		currUser = NULL;
		return MAIN_MENU;
		break;
	default:
		printf("%s输入的序号不存在，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return USER_MENU;
		break;
	}
}
