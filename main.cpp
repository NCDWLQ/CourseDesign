#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <time.h>
#include <Windows.h>

typedef int Status;

#define OK 1
#define ERROR 0

#define USER_FILE "user.txt"
#define PRODUCT_FILE "product.txt"
#define DISCOUNT_FILE "discount.txt"
#define LOG_FILE "sales_log.txt"

#define MAX_CATEGORIES_LENTH 100

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
	DISCOUNT_INFO,
	PRODUCT_MANAGEMENT,
	USER_MANAGEMENT,
	DISCOUNT_MANAGEMENT,
	SALES_REPORT,
	EXIT,
	TEST
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

// 商品结构体
typedef struct Product
{
	int id;
	char name[50];
	char category[2][15];
	int stock;
	double price;
	double discount;	// 优惠价（若无优惠价设为 -1）
	struct Product* next;
}ProductNode, * ProductList;

// 购物车结构体
typedef struct ShoppingCart
{
	int id;
	int quantity;	// 数量
	struct ShoppingCart* next;
} CartNode, * CartList;

// 优惠结构体
typedef struct Discount
{
	int id;
	char category[2][15];       // 优惠商品类别
	double discountRate;        // 折扣百分比
	double minAmount;           // 最低消费金额
	time_t endDate;             // 结束日期
	int minimumLevel;           // 会员等级限制，用户等级 >= minimumLevel 才能享受优惠
	struct Discount* next;
} DiscountNode, * DiscountList;

// 分类结构体（临时存储）
typedef struct Category
{
	char category1[15];
	char category2[15];
	int productCount;	// 商品数量
} CategoryNode, * CategoryList;

// 定义一个临时结构体来存储适用于折扣的商品
typedef struct DiscountItem
{
	int productId;        // 商品ID
	int quantity;         // 商品数量
	double originalPrice; // 商品原价
	char category[2][15]; // 商品分类
} DiscountItem;

void printSeparator(const char* separator_char, const char* color, int width);
void printCentered(const char* text, const char* color, int console_width);
void printHeader();
void printAligned(const char* str, int target_width);

int getDisplayWidth(const char* str);
int getConsoleWidth();

Status initUserList(UserList& L);
Status appendUserList(UserList& L, char* username, char* password, int memberLevel, int points);
Status readUserFromFile(const char* filename, UserList& L);
Status saveUserToFile(const char* filename, UserList L);
Status userExist(UserList L, char* username);
Status updatePassword(UserList L, char* username, char* newPassword);
Status printUserList(UserList L);

Status initProductList(ProductList& L);
Status appendProductList(ProductList& L, int id, char* name, char* category1, char* category2, int stock, double price, double discount);
Status deleteProductNode(ProductList& L, int id);
Status readProductFromFile(const char* filename, ProductList& L);
Status saveProductToFile(const char* filename, ProductList L);
Status productIDExist(ProductList L, int id);
Status getProductInfo(ProductList L, int id, ProductList& info);
Status searchProduct(ProductList L, const char* keyword, int& count);
Status printProductList(ProductList L);
Status addProduct(ProductList& L);
Status updateProduct(ProductList& L);
Status deleteProduct(ProductList& L);

Status generateCategories(ProductList L, CategoryList& categories, int& count);

Status initCartList(CartList& L);
Status appendCartList(CartList L, int id, int quantity);
Status findItemInCart(CartList L, int id, CartList& result);
Status deleteFromCart(CartList& L, int id);

Status initDiscountList(DiscountList& L);
Status addDiscount(DiscountList& L, int id, char category1[15], char category2[15], double discountRate, double minAmount, time_t endDate, int minimumLevel);
Status deleteDiscount(DiscountList& L, int id);
Status deleteExpiredDiscount(DiscountList& L);
Status addDiscountUI(ProductList Product_L, DiscountList& L);
Status readDiscountFromFile(const char* filename, DiscountList& L);
Status saveDiscountToFile(const char* filename, DiscountList L);
Status printDiscountList(DiscountList L);

Status salesLog(int pointsUsed, double totalPay, int pointsEarned);

bool isToday(time_t timestamp);

SystemState mainMenu();

SystemState userLogin(UserList& L);
SystemState userRegister(UserList& L);
SystemState forgotPassword(UserList& L);

SystemState userMenu();
SystemState viewProducts(ProductList& L);
SystemState searchProductUI(ProductList& L);
SystemState shoppingCart(ProductList Product_L);
SystemState checkout(ProductList Product_L, DiscountList Discount_L, UserList& User_L);
SystemState discountInfo(ProductList Product_L, DiscountList Discount_L, UserList& User_L);

SystemState adminMenu();
SystemState productManagement(ProductList& Product_L);
SystemState userManagement(UserList& User_L);
SystemState discountManagement(ProductList Product_L, DiscountList& Discount_L);
SystemState salesReport(ProductList Product_L);

// 定义全局变量，指示当前用户
UserList currUser = NULL;
// 定义全局指针变量，指向购物车链表
CartList Cart_L = NULL;

int main()
{
	// 初始化 UserList 并从文件中读取用户信息
	UserList User_L;
	initUserList(User_L);
	readUserFromFile(USER_FILE, User_L);

	// 初始化 ProductList
	ProductList Product_L;
	initProductList(Product_L);
	readProductFromFile(PRODUCT_FILE, Product_L);

	// 初始化 DiscountList
	DiscountList Discount_L;
	initDiscountList(Discount_L);
	readDiscountFromFile(DISCOUNT_FILE, Discount_L);
	deleteExpiredDiscount(Discount_L); // 删除过期的折扣

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
		case ADMIN_MENU:
			currState = adminMenu();
			break;
		case VIEW_PRODUCTS:
			currState = viewProducts(Product_L);
			break;
		case SEARCH_PRODUCTS:
			currState = searchProductUI(Product_L);
			break;
		case SHOPPING_CART:
			currState = shoppingCart(Product_L);
			break;
		case CHECKOUT:
			currState = checkout(Product_L, Discount_L, User_L);
			break;
		case DISCOUNT_INFO:
			currState = discountInfo(Product_L, Discount_L, User_L);
			break;
		case PRODUCT_MANAGEMENT:
			currState = productManagement(Product_L);
			break;
		case USER_MANAGEMENT:
			currState = userManagement(User_L);
			break;
		case DISCOUNT_MANAGEMENT:
			currState = discountManagement(Product_L, Discount_L);
			break;
		case SALES_REPORT:
			currState = salesReport(Product_L);
			break;
		case TEST:
			return OK;
		}
	}

	return OK;
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

void printHeader()
{
	int width = getConsoleWidth();

	printSeparator("=", CYAN, width);
	printCentered("欢迎光临一次买够！！！！！", WHITE, width);
	printSeparator("=", CYAN, width);
}

// 打印对齐的字符串
void printAligned(const char* str, int target_width) {
	printf("%s", str);
	int actual_width = getDisplayWidth(str);
	for (int i = 0; i < target_width - actual_width; ++i) {
		putchar(' ');
	}
}

// 获取字符串的显示宽度（考虑中文字符）
int getDisplayWidth(const char* str) {
	int width = 0;
	while (*str) {
		unsigned char ch = (unsigned char)*str;		// 用 unsigned char 来处理字符，是因为有可能是负数
		if (ch == '\n' || ch == '\r' || ch == '\t' || ch < 32)
		{
			str++;		// 跳过控制字符
			continue;
		}
		if (ch >= 0x80) {
			width += 2;
			str += 2;
		}
		else {
			width += 1;
			str++;
		}
	}
	return width;
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

// 打印用户列表
Status printUserList(UserList L)
{
	UserList p = L->next;
	if (p == NULL)
	{
		printf("用户列表为空\n");
		return ERROR;
	}
	printAligned("用户名", 20);
	printAligned("会员等级", 10);
	printAligned("积分", 10);
	printf("\n");
	printSeparator("-", WHITE, getConsoleWidth());

	int count = 0;
	while (p)
	{
		if (p->memberLevel == 0)	// 管理员不显示
		{
			p = p->next;
			continue;
		}
		printf("%-20s", p->username);
		printf("%-10d", p->memberLevel);
		printf("%-10d", p->points);
		printf("\n");
		p = p->next;
		count++;
	}
	printSeparator("-", WHITE, getConsoleWidth());
	printf("共 %d 位用户\n", count);

	return OK;
}

// 初始化 ProductList
Status initProductList(ProductList& L)
{
	L = new ProductNode;
	L->next = NULL;
	return OK;
}

// 向 ProductList 的末尾添加一个节点
Status appendProductList(ProductList& L, int id, char* name, char* category1, char* category2, int stock, double price, double discount)
{
	ProductList p, newNode;

	// 新建一个节点
	newNode = new ProductNode;

	newNode->id = id;
	strcpy(newNode->name, name);
	strcpy(newNode->category[0], category1);
	strcpy(newNode->category[1], category2);
	newNode->stock = stock;
	newNode->price = price;
	newNode->discount = discount;
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

// 从 ProductList 中删除一个节点
Status deleteProductNode(ProductList& L, int id)
{
	ProductList p = L->next;
	ProductList prev = L;
	while (p)
	{
		if (p->id == id)
		{
			prev->next = p->next;
			delete p;
			return OK;
		}
		prev = p;
		p = p->next;
	}
	return ERROR; // 商品 ID 不存在
}

// 从文件中读取 ProductList
Status readProductFromFile(const char* filename, ProductList& L)
{
	FILE* fp;
	if ((fp = fopen(filename, "r")) == NULL)
	{
		return ERROR;
	}

	int id, stock;
	char name[50] = { 0 };
	double price, discount;
	char category_t[31], category1[15], category2[15];

	while (fscanf(fp, "%d %s %s %d %lf %lf", &id, name, category_t, &stock, &price, &discount) != EOF)
	{
		// 将商品名称中的逗号替换为空格
		for (int i = 0; name[i] != '\0'; i++)
		{
			if (name[i] == ',')
			{
				name[i] = ' ';
			}
		}
		// 处理商品分类
		char* token;
		token = strtok(category_t, "@");
		strcpy(category1, token);
		token = strtok(NULL, "@");
		if (token != NULL)
		{
			strcpy(category2, token);
		}
		else
		{
			category2[0] = '\0';
		}

		appendProductList(L, id, name, category1, category2, stock, price, discount);
	}

	fclose(fp);
	return OK;
}

Status saveProductToFile(const char* filename, ProductList L)
{
	FILE* fp;
	ProductList p = L->next;

	if ((fp = fopen(filename, "w")) == NULL)
	{
		return ERROR;
	}

	while (p)	// 遍历商品列表，逐个写入文件
	{
		// 将商品名称中的空格替换为逗号
		char name[50];
		strcpy(name, p->name);
		for (int i = 0; name[i] != '\0'; i++)
		{
			if (name[i] == ' ')
			{
				name[i] = ',';
			}
		}

		// 拼接商品分类，格式为 "一级分类@二级分类"（如果二级分类为空，则只写一级分类）
		char category_t[31];
		strcpy(category_t, p->category[0]);
		if (p->category[1][0] != '\0')
		{
			strcat(category_t, "@");
			strcat(category_t, p->category[1]);
		}

		// 写入文件，格式：id name category_t stock price discount
		fprintf(fp, "%d %s %s %d %.2f %.2f\n", p->id, name, category_t, p->stock, p->price, p->discount);
		p = p->next;
	}
	fclose(fp);
	return OK;
}

// 判断商品 ID 是否存在
Status productIDExist(ProductList L, int id)
{
	ProductList p = L->next;
	while (p)
	{
		if (p->id == id)
		{
			return OK;
		}
		p = p->next;
	}
	return ERROR;
}

// 根据商品 ID 返回商品信息
Status getProductInfo(ProductList L, int id, ProductList& info)
{
	if (L->next == NULL)
	{
		return ERROR;
	}
	ProductList p = L->next;
	while (p)
	{
		if (p->id == id)
		{
			info = p;
			return OK;
		}
		p = p->next;
	}
	return ERROR;
}

Status searchProduct(ProductList L, const char* keyword, int& count)
{
	ProductList p = L->next;
	int found = 0;
	count = 0;

	while (p)
	{
		if (strstr(p->name, keyword) != NULL || strstr(p->category[0], keyword) != NULL || strstr(p->category[1], keyword) != NULL)
		{
			if (currUser != 0 && p->stock == 0)		// 普通用户不显示库存为 0 的商品
			{
				p = p->next;
				continue;
			}
			else if (found == 0)		// 首次找到匹配商品时打印表头
			{
				printAligned("ID", 4);
				printAligned("商品名称", 50);
				printAligned("商品分类", 30);
				printAligned("库存", 10);
				printAligned("价格", 10);
				printAligned("优惠价", 10);
				printf("\n");
				printSeparator("-", WHITE, getConsoleWidth());
				found = 1;
			}
			char category[31];
			strcpy(category, p->category[0]);
			if (p->category[1][0] != '\0')	// 拼接商品分类
			{
				strcat(category, "->");
				strcat(category, p->category[1]);
			}
			// 打印商品信息
			printf("%-4d", p->id);
			printAligned(p->name, 50);
			printAligned(category, 30);
			printf("%-10d", p->stock);
			printf("%-10.2f", p->price);
			if (p->discount != -1)
			{
				printf("%-10.2f", p->discount);
			}
			else
			{
				printf("%-10s", "无");
			}
			printf("\n");
			count++;
		}
		p = p->next;
	}

	if (!found)
	{
		printf("%s没有找到匹配的商品%s\n\n", RED, RESET);
		return ERROR;
	}
	else
	{
		printSeparator("-", WHITE, getConsoleWidth());
		printf("%s共找到 %d 件商品%s\n\n", GREEN, count, RESET);
		return OK;
	}
}

// 打印商品列表
Status printProductList(ProductList L)
{
	ProductList p = L->next;
	char category[31];
	int count = 0, sold_out = 0;
	
	printAligned("ID", 4);
	printAligned("商品名称", 50);
	printAligned("商品分类", 30);
	printAligned("库存", 10);
	printAligned("价格", 10);
	printAligned("优惠价", 10);
	printf("\n");
	printSeparator("-", WHITE, getConsoleWidth());

	while (p)
	{
		if (p->price == -1 && currUser->memberLevel != 0)	// 普通用户不显示赠品
		{
			p = p->next;
			continue;
		}
		if (p->stock == 0)		// 普通用户不显示库存为 0 的商品
		{
			sold_out++;
			if (currUser->memberLevel != 0)
			{
				p = p->next;
				continue;
			}
			else
			{
				printf("%s", YELLOW);
			}
		}
		strcpy(category, p->category[0]);
		if (p->category[1][0] != '\0')
		{
			strcat(category, "->");
			strcat(category, p->category[1]);
		}
		
		printf("%-4d", p->id);
		printAligned(p->name, 50);
		printAligned(category, 30);
		printf("%-10d", p->stock);
		if (p->price != -1)
		{
			printf("%-10.2f", p->price);
			if (p->discount != -1)
			{
				printf("%-10.2f", p->discount);
			}
			else
			{
				printf("%-10s", "无");
			}
		}
		else
		{
			printAligned("赠品", 10);
			printf("满%.2lf赠送", p->discount);
		}
		printf("%s\n", RESET);
		count++;
		p = p->next;
	}
	printSeparator("-", WHITE, getConsoleWidth());
	printf("%s共找到 %d 件商品%s", GREEN, count, RESET);
	if (currUser->memberLevel == 0)
	{
		printf("，其中 %d 件商品已售罄，%s请及时补货！%s", sold_out, RED, RESET);
	}
	printf("\n\n");
	return OK;
}

Status addProduct(ProductList& L)
{
	int max_id = 0, id, stock;
	double price, discount;
	char name[50], category1[15], category2[15];
	ProductList p = L->next;
	while (p)
	{
		if (p->id > max_id)
		{
			max_id = p->id;
		}
		p = p->next;
	}
	id = max_id + 1;	// 新商品 ID 从最大 ID + 1 开始
	while (1)
	{
		printf("商品名称: ");
		scanf(" %[^\n]", name);		// 读取包含空格的字符串
		if (strlen(name) == 0)		// 判断是否为空
		{
			printf("%s商品名称不能为空，请重新输入！%s\n", BOLD RED, RESET);
			continue;
		}
		else
		{
			break;
		}
	}
	while (1)
	{
		printf("商品一级分类（必填）: ");
		scanf("%s", category1);
		if (strlen(category1) == 0)
		{
			printf("%s商品一级分类不能为空，请重新输入！%s\n", BOLD RED, RESET);
			continue;
		}
		else
		{
			break;
		}
	}
	// 清除输入缓冲区
	getchar();
	printf("商品二级分类（若无则留空）: ");
	fgets(category2, sizeof(category2), stdin);
	category2[strcspn(category2, "\n")] = 0;	// 去除末尾换行符
	if (strlen(category2) == 0)
	{
		category2[0] = '\0';	// 如果没有输入二级分类，则将其设置为空字符串
	}
	while (1)
	{
		printf("商品数量: ");
		scanf("%d", &stock);
		if (stock <= 0)
		{
			printf("%s商品数量必须大于 0，请重新输入！%s\n", BOLD RED, RESET);
		}
		else
		{
			break;
		}
	}
	while (1)
	{
		printf("商品价格: ");
		scanf("%lf", &price);
		if (price <= 0)
		{
			printf("%s商品价格必须大于 0，请重新输入！%s\n", BOLD RED, RESET);
		}
		else
		{
			break;
		}
	}
	while (1)
	{
		printf("商品优惠价（若无优惠则输入 -1）: ");
		scanf("%lf", &discount);
		if (discount == -1 || discount > 0 && discount < price)
		{
			break;
		}
		else
		{
			printf("%s优惠价必须介于 0 和商品价格之间，请重新输入！%s\n", BOLD RED, RESET);
		}
	}
	// 添加商品到链表
	appendProductList(L, id, name, category1, category2, stock, price, discount);
	printf("%s商品 %s 添加成功！ID：%d%s\n\n", GREEN, name, id, RESET);
	return OK;
}

// 修改商品信息
Status updateProduct(ProductList& L)
{
	int id;
	printf("请输入要修改的商品 ID（输入 0 取消修改）:");
	scanf("%d", &id);
	if (id == 0)
	{
		return OK;
	}
	
	ProductList info;
	getProductInfo(L, id, info);
	if (info == NULL)
	{
		printf("%s商品 ID 不存在！%s\n", BOLD RED, RESET);
		return ERROR;
	}
	printf("选中的商品：%s%s%s\n\n", YELLOW, info->name, RESET);
	printf("选择要修改的信息：\n");
	printf("1. 商品名称\n");
	printf("2. 一级分类\n");
	printf("3. 二级分类\n");
	printf("4. 库存\n");
	printf("5. 价格\n");
	printf("6. 优惠价\n");
	printf("0. 取消修改\n");
	printf("%s请输入序号:%s ", YELLOW, RESET);
	int choice;
	scanf("%d", &choice);
	printf("\n");
	switch (choice)
	{
	case 1:
	{
		printf("当前商品名称: %s\n", info->name);
		printf("请输入新的商品名称: ");
		char name[50];
		scanf(" %[^\n]", name);
		strcpy(info->name, name);
		printf("%s商品名已修改为 %s%s\n\n", BOLD GREEN, info->name, RESET);
		break;
	}
	case 2:
	{
		printf("当前一级分类: %s\n", info->category[0]);
		printf("请输入新的一级分类: ");
		char category1[15];
		scanf("%s", category1);
		strcpy(info->category[0], category1);
		printf("%s一级分类已修改为 %s%s\n\n", BOLD GREEN, info->category[0], RESET);
		break;
	}
	case 3:
	{
		printf("当前二级分类: %s\n", strlen(info->category[1]) == 0 ? "无" : info->category[1]);
		printf("请输入新的二级分类（留空表示无）: ");
		char category2[15];
		fgets(category2, sizeof(category2), stdin);
		category2[strcspn(category2, "\n")] = 0; // 去除末尾换行符
		if (strlen(category2) == 0) {
			info->category[1][0] = '\0'; // 设置为空字符串
			printf("%s二级分类已清空%s\n\n", BOLD GREEN, RESET);
		}
		else {
			strcpy(info->category[1], category2);
			printf("%s二级分类已修改为 %s%s\n\n", BOLD GREEN, info->category[1], RESET);
		}
		break;
	}
	case 4:
	{
		printf("当前库存: %d\n", info->stock);
		printf("请输入新的库存: ");
		int stock;
		scanf("%d", &stock);
		info->stock = stock;
		printf("%s库存已修改为 %d%s\n\n", BOLD GREEN, info->stock, RESET);
		break;
	}
	case 5:
	{
		printf("当前价格: %.2f\n", info->price);
		printf("请输入新的价格: ");
		double price;
		scanf("%lf", &price);
		info->price = price;
		printf("%s价格已修改为 %.2f%s\n\n", BOLD GREEN, info->price, RESET);
		break;
	}
	case 6:
	{
		printf("当前优惠价: %.2f\n", info->discount);
		printf("请输入新的优惠价（输入 -1 表示无优惠）: ");
		double discount;
		scanf("%lf", &discount);
		if (discount == -1)
		{
			info->discount = -1;
			printf("%s优惠已清空%s\n\n", BOLD GREEN, RESET);
		}
		else
		{
			info->discount = discount;
			printf("%s优惠价已修改为 %.2f%s\n\n", BOLD GREEN, info->discount, RESET);
		}
		break;
	}
	case 0:
	{
		printf("\n");
		break;
	}
	default:
	{
		printf("%s输入的序号不存在！%s\n\n", BOLD RED, RESET);
		return ERROR;
	}
	}

	return OK;
}

// 删除商品
Status deleteProduct(ProductList& L)
{
	int id;
	printf("请输入要删除的商品 ID（输入 0 取消操作）:");
	scanf("%d", &id);
	if (id == 0)
	{
		return ERROR;
	}
	ProductList info;
	char name[50];
	if (getProductInfo(L, id, info) == ERROR)
	{
		printf("%s商品 ID 不存在！%s\n\n", BOLD RED, RESET);
		return ERROR;
	}
	strcpy(name, info->name);
	printf("选中的商品：%s%s%s\n", YELLOW, name, RESET);
	int choice;
	printf("确认删除输入 1，取消删除输入 0: ");
	scanf("%d", &choice);
	if (choice == 1)
	{
		deleteProductNode(L, id);
		printf("%s商品 %s 删除成功！%s\n\n", GREEN, name, RESET);
	}
	else
	{
		printf("%s取消删除商品%s\n\n", BOLD YELLOW, RESET);
	}
	return OK;
}

// 生成商品分类列表
Status generateCategories(ProductList L, CategoryList& categories, int& count)
{
	count = 0;
	ProductList p = L->next;

	while (p)
	{
		int found = 0;
		for (int i = 0; i < count; i++)
		{
			if(strcmp(categories[i].category1, p->category[0]) == 0 &&
				strcmp(categories[i].category2, p->category[1]) == 0)
			{
				found = 1;
				categories[i].productCount++;
				break;
			}
		}
		if (!found && strcmp(p->category[0], "赠品") != 0)
		{
			strcpy(categories[count].category1, p->category[0]);
			strcpy(categories[count].category2, p->category[1]);
			categories[count].productCount = 1;
			count++;
		}
		p = p->next;
	}
	return OK;
}

// 初始化购物车链表
Status initCartList(CartList& L)
{
	L = new CartNode;
	L->next = NULL;
	return OK;
}

// 向购物车链表的末尾添加一个节点
Status appendCartList(CartList L, int id, int quantity)
{
	CartList p, newNode;
	// 新建一个节点
	newNode = new CartNode;
	newNode->id = id;
	newNode->quantity = quantity;
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

// 查找购物车中是否存在指定商品，并返回该节点的指针
Status findItemInCart(CartList L, int id, CartList& result)
{
	CartList p = L->next;
	while (p)
	{
		if (p->id == id)
		{
			// 找到了匹配的商品
			result = p;
			return OK;
		}
		p = p->next;
	}
	// 没有找到匹配的商品
	result = NULL;
	return ERROR;
}

// 从购物车删除指定商品
Status deleteFromCart(CartList& L, int id)
{
	if (L->next == NULL)
	{
		return ERROR;
	}

	CartList p = L;
	CartList q = L->next;

	while (q)
	{
		if (q->id == id)
		{
			p->next = q->next;
			delete q;
			return OK;
		}
		p = q;
		q = q->next;
	}
	return ERROR;
}

// 初始化优惠链表
Status initDiscountList(DiscountList& L)
{
	L = new DiscountNode;
	L->next = NULL;
	return OK;
}

// 添加优惠信息
Status addDiscount(DiscountList& L, int id, char category1[15], char category2[15], double discountRate, double minAmount, time_t endDate, int minimumLevel)
{
	DiscountList p, newNode;
	// 新建一个节点
	newNode = new DiscountNode;
	newNode->id = id;
	strcpy(newNode->category[0], category1);
	strcpy(newNode->category[1], category2);
	newNode->discountRate = discountRate;
	newNode->minAmount = minAmount;
	newNode->endDate = endDate;
	newNode->minimumLevel = minimumLevel;
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

// 删除优惠信息
Status deleteDiscount(DiscountList& L, int id)
{
	DiscountList p = L->next;
	DiscountList prev = L;
	while (p)
	{
		if (p->id == id)
		{
			prev->next = p->next;
			delete p;
			return OK;
		}
		prev = p;
		p = p->next;
	}
	return ERROR; // 优惠 ID 不存在
}

// 删除过期的优惠信息
Status deleteExpiredDiscount(DiscountList& L)
{
	DiscountList p = L->next;
	DiscountList prev = L;
	while (p)
	{
		if (p->endDate < time(NULL))
		{
			prev->next = p->next;
			delete p;
			p = prev->next; // 更新 p
		}
		else
		{
			prev = p;
			p = p->next;
		}
	}
	saveDiscountToFile(DISCOUNT_FILE, L); // 保存删除后的优惠信息到文件
	return OK;
}

Status addDiscountUI(ProductList Product_L, DiscountList& L)
{
	CategoryList categories = new CategoryNode[MAX_CATEGORIES_LENTH];
	int count = 0;
	generateCategories(Product_L, categories, count);

	printAligned("编号", 5);
	printAligned("分类名", 30);
	printAligned("商品种数", 10);
	printf("\n");
	printSeparator("-", WHITE, getConsoleWidth());

	for (int i = 0; i < count; i++)
	{
		char category[31];
		if (categories[i].category2[0] != '\0')
		{
			strcpy(category, categories[i].category1);
			strcat(category, "->");
			strcat(category, categories[i].category2);
		}
		else
		{
			strcpy(category, categories[i].category1);
		}
		printf("%-5d", i + 1);
		printAligned(category, 30);
		printf("%-10d", categories[i].productCount);
		printf("\n");
	}

	int choice = -1;
	printf("请选择要添加优惠的分类编号（输入 0 取消操作）: ");
	scanf("%d", &choice);
	if (choice == 0)
	{
		return ERROR;
	}
	else if (choice < 1 || choice > count)
	{
		printf("%s输入的编号不存在，请重新输入！%s\n", BOLD RED, RESET);
		return ERROR;
	}

	char category[31];
	strcpy(category, categories[choice - 1].category1);
	if (categories[choice - 1].category2[0] != '\0')
	{
		strcat(category, "->");
		strcat(category, categories[choice - 1].category2);
	}
	printf("您选择的分类是：%s\n", category);

	double discountRate;
	printf("请输入折扣率（0-1）: ");
	scanf("%lf", &discountRate);
	if (discountRate <= 0 || discountRate > 1)
	{
		printf("%s折扣率必须在 0 到 1 之间！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return ERROR;
	}

	double minAmount;
	printf("请输入生效金额（大于 0）: ");
	scanf("%lf", &minAmount);
	if (minAmount <= 0)
	{
		printf("%s生效金额必须大于 0！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return ERROR;
	}

	time_t endDate;
	long long duration;
	char unit;
	printf("请输入优惠有效期（示例：7d 为 7 天，1w 为 1 周，1m 为 1 月）: ");
	scanf("%lld%c", &duration, &unit);
	if (duration <= 0)
	{
		printf("%s输入的时间格式不正确！%s\n", BOLD RED, RESET);
		return ERROR;
	}
	switch (unit)
	{
	case 'd':
		endDate = time(NULL) + duration * 24 * 60 * 60; // 天
		break;
	case 'w':
		endDate = time(NULL) + duration * 7 * 24 * 60 * 60; // 周
		break;
	case 'm':
		endDate = time(NULL) + duration * 30 * 24 * 60 * 60; // 月
		break;
	default:
		printf("%s输入的时间格式不正确！%s\n", BOLD RED, RESET);
		return ERROR;
	}

	int id = 0;
	DiscountList p = L->next;
	while (p)
	{
		if (p->id > id)
		{
			id = p->id;
		}
		p = p->next;
	}
	id++;		// 新优惠 ID 为最大 ID + 1

	int minimumLevel;
	printf("请输入享受折扣所需的最低会员等级（1~5）: ");
	scanf("%d", &minimumLevel);
	if (minimumLevel < 1 && minimumLevel > 5)
	{
		printf("%s会员等级必须在 1 到 5 之间！%s\n", BOLD RED, RESET);
		return ERROR;
	}
	
	// 添加优惠到链表
	addDiscount(L, id, categories[choice - 1].category1, categories[choice - 1].category2, discountRate, minAmount, endDate, minimumLevel);
	saveDiscountToFile(DISCOUNT_FILE, L);
	return OK;
}

// 从文件读取优惠信息并存储到 DiscountList 中
Status readDiscountFromFile(const char* filename, DiscountList& L)
{
	FILE* fp;
	if ((fp = fopen(filename, "r")) == NULL)
	{
		return ERROR;
	}

	int id;
	char category1[15], category2[15];
	double discountRate, minAmount;
	time_t endDate;
	int minimumLevel;

	// Update the format to include minimumLevel
	while (fscanf(fp, "%d %s %s %lf %lf %lld %d", &id, category1, category2, &discountRate, &minAmount, &endDate, &minimumLevel) != EOF)
	{
		addDiscount(L, id, category1, category2, discountRate, minAmount, endDate, minimumLevel);
	}

	fclose(fp);
	return OK;
}

Status saveDiscountToFile(const char* filename, DiscountList L)
{
	FILE* fp;
	if ((fp = fopen(filename, "w")) == NULL)
	{
		return ERROR;
	}

	DiscountList p = L->next;
	if (p == NULL)
	{
		return ERROR;
	}
	while (p)
	{
		fprintf(fp, "%d %s %s %.2f %.2f %lld %d\n", p->id, p->category[0], p->category[1], p->discountRate, p->minAmount, p->endDate, p->minimumLevel);
		p = p->next;
	}
	fclose(fp);
	return OK;
}

// 打印优惠列表
Status printDiscountList(DiscountList L)
{
	DiscountList p = L->next;
	if (p == NULL)
	{
		printf("优惠列表为空\n\n");
		return ERROR;
	}
	printAligned("ID", 4);
	printAligned("商品分类", 30);
	printAligned("折扣率", 10);
	printAligned("生效金额", 10);
	printAligned("截止日期", 22);
	printAligned("最低会员等级", 10);
	printf("\n");
	printSeparator("-", WHITE, getConsoleWidth());

	int count = 0;
	while (p)
	{
		// 处理分类
		char category[31];
		strcpy(category, p->category[0]);
		if (p->category[1][0] != '\0')
		{
			strcat(category, "->");
			strcat(category, p->category[1]);
		}
		// 处理时间格式
		struct tm* timeinfo = localtime(&p->endDate);
		char timeStr[30];
		strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", timeinfo);

		printf("%-4d", p->id);
		printAligned(category, 30);
		printf("%-10.2f", p->discountRate);
		printf("%-10.2f", p->minAmount);
		printf("%-22s",timeStr);
		printf("%-10d\n", p->minimumLevel);
		p = p->next;
		count++;
	}
	printSeparator("-", WHITE, getConsoleWidth());
	printf("共 %d 条优惠信息\n\n", count);
	return OK;
}

// 将销售记录保存到文件
Status salesLog(int pointsUsed, double totalPay, int pointsEarned)
{
	FILE* fp;
	
	// 以追加模式打开文件
	if ((fp = fopen(LOG_FILE, "a")) == NULL)
	{
		return ERROR;
	}

	time_t now = time(NULL);

	int itemCount = 0;
	CartList cart_p = Cart_L->next;
	while (cart_p)
	{
		itemCount++;
		cart_p = cart_p->next;
	}

	fprintf(fp, "%lld %s %d ", now, currUser->username, itemCount);

	cart_p = Cart_L->next;
	while (cart_p)
	{
		fprintf(fp, "%d %d ", cart_p->id, cart_p->quantity);
		cart_p = cart_p->next;
	}
	fprintf(fp, "%d %.2f %d\n", pointsUsed, totalPay, pointsEarned);

	fclose(fp);
	return OK;
}

// 判断时间戳是否为当日
bool isToday(time_t timestamp)
{
	// 获取当前时间
	time_t now = time(NULL);

	// 转换为本地时间结构
	struct tm* today = localtime(&now);
	struct tm* timeinfo = localtime(&timestamp);

	// 比较年、月、日是否相同
	return (today->tm_year == timeinfo->tm_year &&
		today->tm_mon == timeinfo->tm_mon &&
		today->tm_mday == timeinfo->tm_mday);
}


// 主菜单
SystemState mainMenu()
{
	int console_width = getConsoleWidth();
	int choice = -1;

	// 清屏  
	system("cls");

	// 打印标题
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
	case 9:
		return TEST;  // 仅供测试使用
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
				// 为用户创建购物车链表
				initCartList(Cart_L);
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
		printf("%s注册成功！正在跳转登录%s\n", BOLD GREEN, RESET);
		Sleep(1000);
		return LOGIN;
	}
	else
	{
		printf("%s两次输入的密码不一致，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return REGISTER;
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

// 显示用户菜单
SystemState userMenu()
{
	int choice = -1;

	// 显示欢迎信息
	system("cls");
	printHeader();
	printf("欢迎回来，%s！\n您的会员等级：%d，购物积分：%d\n", currUser->username, currUser->memberLevel, currUser->points);
	printf("%s注意事项：退出登录后购物车将被清空！%s\n", RED, RESET);
	printSeparator("-", WHITE, getConsoleWidth());

	// 显示菜单选项
	printf("请选择要进行的操作：\n");
	printf("1. 浏览商品\n");
	printf("2. 搜索商品\n");
	printf("3. 查看购物车\n");
	printf("4. 自助结账\n");
	printf("5. 优惠信息\n");
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
		return DISCOUNT_INFO;
		break;
	case 0:
		currUser = NULL;
		printf("%s已退出登录！正在返回主菜单%s\n", BOLD GREEN, RESET);
		Sleep(1000);
		return MAIN_MENU;
		break;
	default:
		printf("%s输入的序号不存在，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return USER_MENU;
		break;
	}
}

// 显示商品列表
SystemState viewProducts(ProductList& L)
{
	system("cls");
	printHeader();
	printProductList(L);
	while (1)
	{
		int id = -1, num = -1;
		printf("%s输入商品 ID 进行购买（输入 0 返回用户菜单）：%s", YELLOW, RESET);
		scanf("%d", &id);
		if (id == 0)
		{
			return USER_MENU;
		}
		else if (id < 0)
		{
			printf("%s输入的商品 ID 不合法，请重新输入！%s\n", BOLD RED, RESET);
			Sleep(1000);
			continue;
		}
		else if (!productIDExist(L, id))
		{
			printf("%s商品不存在，请重新输入！%s\n", BOLD RED, RESET);
			Sleep(1000);
		}
		else
		{
			ProductList p = NULL;
			getProductInfo(L, id, p);
			while (1)
			{
				printf("%s输入购买数量（输入 0 取消购买）：%s", YELLOW, RESET);
				scanf("%d", &num);
				if(num == 0)
				{
					break;
				}
				else if (num < 0)
				{
					printf("%s输入的数量不合法，请重新输入！%s\n", BOLD RED, RESET);
				}
				else if (p->stock == 0)
				{
					printf("%s商品已售罄！请重新输入！%s\n", BOLD RED, RESET);
				}
				else if (num > p->stock)
				{
					printf("%s库存不足！当前剩余 %d 件。请重新输入！%s\n", BOLD RED, p->stock, RESET);
				}
				else
				{
					// 处理购物车中的重复商品
					CartList existingItem = NULL;
					if (findItemInCart(Cart_L, id, existingItem) == OK)
					{
						// 购物车中已存在该商品，增加数量
						int totalQuantity = existingItem->quantity + num;
						// 再次检查总数量是否超过库存
						if (totalQuantity > p->stock)
						{
							printf("%s添加后总数量超过库存！当前剩余 %d 件，购物车已有 %d 件。请重新输入%s\n", BOLD RED, p->stock, existingItem->quantity, RESET);
							continue;
						}
						existingItem->quantity = totalQuantity;
						printf("%s已增加 %d 件 %s 到购物车，购物车中现在共有 %d 件%s\n\n", BOLD GREEN, num, p->name, totalQuantity, RESET);
					}
					else
					{
						// 购物车中不存在该商品，添加新条目
						appendCartList(Cart_L, id, num);
						printf("%s已添加 %d 件 %s 到购物车%s\n\n", BOLD GREEN, num, p->name, RESET);
					}
					break;
				}
				printf("\n");
			}
		}
	}
}

SystemState searchProductUI(ProductList& L)
{
	system("cls");
	printHeader();

	while (1)
	{
		char keyword[50] = { 0 };
		printf("输入商品名称或分类进行搜索（支持模糊搜索）\n%s请输入（输入 0 返回%s菜单）：%s", YELLOW,
			currUser->memberLevel == 0 ? "管理员" : "用户", RESET);
		scanf("%49s", keyword);

		if (strcmp(keyword, "0") == 0)
		{
			return USER_MENU;
		}

		int count = 0;
		searchProduct(L, keyword, count);
	}
}

// 显示购物车页面
SystemState shoppingCart(ProductList Product_L)
{
	while (1)
	{
		system("cls");
		printHeader();
		printCentered("购物车", WHITE, getConsoleWidth());
		printf("\n");

		CartList cart = Cart_L->next;
		if (cart == NULL)
		{
			printf("%s购物车空空如也！%s\n正在返回用户菜单", BOLD YELLOW, RESET);
			Sleep(1000);
			return USER_MENU;
		}

		// 显示表头
		printAligned("ID", 4);
		printAligned("商品名称", 50);
		printAligned("数量", 10);
		printAligned("单价", 10);
		printAligned("小计", 10);
		printf("\n");
		printSeparator("-", WHITE, getConsoleWidth());

		double total = 0.0;
		while (cart)
		{
			double price = 0.0;
			ProductList p = NULL;

			getProductInfo(Product_L, cart->id, p);
			price = p->discount = -1 ? p->price : p->discount;

			printf("%-4d", cart->id);
			printAligned(p->name, 50);
			printf("%-10d", cart->quantity);
			printf("%-10.2f", price);
			printf("%-10.2f\n", price * cart->quantity);

			total += price * cart->quantity;
			cart = cart->next;
		}
		printSeparator("-", WHITE, getConsoleWidth());
		printf("总计：%.2f 元\n%s价格仅供参考，请以结算页面实际显示为准！%s\n\n", total, BOLD YELLOW, RESET);

		// 显示操作菜单
		printf("请选择要进行的操作：\n");
		printf("1. 修改商品数量\n");
		printf("2. 删除商品\n");
		printf("3. 自助结账\n");
		printf("0. 返回用户菜单\n");
		printf("%s请输入操作选项：%s", YELLOW, RESET);

		int choice;
		scanf("%d", &choice);

		if (choice == 0)
		{
			return USER_MENU;
		}
		else if (choice == 1)
		{
			printf("%s请输入要修改的商品 ID：%s", YELLOW, RESET);
			int id;
			scanf("%d", &id);

			CartList item = NULL;
			if (findItemInCart(Cart_L, id, item) == OK)
			{
				ProductList p = NULL;
				getProductInfo(Product_L, id, p);
				printf("当前数量：%d\n", item->quantity);
				printf("%s请输入新的数量（输入 0 取消修改）：%s", YELLOW, RESET);
				int newQuantity;
				scanf("%d", &newQuantity);

				if (newQuantity > 0)
				{
					if (newQuantity > p->stock)
					{
						printf("%s数量超过库存！当前库存：%d%s\n", BOLD RED, p->stock, RESET);
						Sleep(1500);
					}
					else
					{
						item->quantity = newQuantity;
						printf("%s修改成功！%s\n", BOLD GREEN, RESET);
						Sleep(1000);
					}
				}
			}
			else
			{
				printf("%s购物车中不存在该商品！%s\n", BOLD RED, RESET);
				Sleep(1500);
			}
		}
		else if (choice == 2)
		{
			printf("请输入要删除的商品 ID：");
			int id;
			scanf("%d", &id);

			if (deleteFromCart(Cart_L, id) == OK)
			{
				printf("%s删除成功！%s\n", BOLD GREEN, RESET);
				Sleep(1000);
			}
			else
			{
				printf("%s购物车中不存在该商品！%s\n", BOLD RED, RESET);
				Sleep(1500);
			}
		}
		else if (choice == 3)
		{
			return CHECKOUT;
		}
		else
		{
			printf("%s无效的操作选项！%s\n", BOLD RED, RESET);
			Sleep(1500);
		}
	}
}

// 结账函数 - 自动选择最优折扣
SystemState checkout(ProductList Product_L, DiscountList Discount_L, UserList& User_L)
{
	time_t currTime = time(0);
	tm* ltm = localtime(&currTime);

	system("cls");
	printHeader();
	printCentered("结算页面", WHITE, getConsoleWidth());

	// 显示当前日期时间
	printf("当前系统时间：%04d-%02d-%02d %02d:%02d:%02d\n", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

	CartList cart = Cart_L->next;
	if (cart == NULL)
	{
		printf("%s购物车为空，无法结账！%s\n", BOLD RED, RESET);
		Sleep(1500);
		return USER_MENU;
	}

	// 显示购物清单
	printf("%s您的购物清单：%s\n", BOLD, RESET);
	printAligned("商品名称", 50);
	printAligned("数量", 10);
	printAligned("单价", 15);
	printAligned("原价小计", 15);
	printf("\n");
	printSeparator("-", WHITE, getConsoleWidth());

	// 计算购物车总价（包含商品自身优惠价）
	double cartTotal = 0.0;
	CartList tempCart = cart;
	while (tempCart)
	{
		ProductList p = NULL;
		getProductInfo(Product_L, tempCart->id, p);

		double price = (p->discount != -1) ? p->discount : p->price;
		double subtotal = price * tempCart->quantity;

		printf("%-50s", p->name);
		printf("%-10d", tempCart->quantity);
		printf("%-15.2f", price);
		printf("%-15.2f\n", subtotal);

		cartTotal += subtotal;
		tempCart = tempCart->next;
	}
	printSeparator("-", WHITE, getConsoleWidth());
	printf("商品原始总价：%.2f 元\n\n", cartTotal);

	// 检查可用折扣并自动计算最优折扣
	time_t now = time(NULL);
	DiscountList discount = Discount_L->next;
	int discountCount = 0;

	// 统计有效折扣数量
	while (discount)
	{
		if (now < discount->endDate && currUser->memberLevel >= discount->minimumLevel) // 只考虑未过期的折扣且用户等级满足要求的折扣
		{
			discountCount++;
		}
		discount = discount->next;
	}

	// 如果没有折扣，直接进行结算
	if (discountCount == 0)
	{
		printf("%s当前没有有效的折扣活动%s\n\n", YELLOW, RESET);
	}
	else
	{
		// 准备分析每个折扣
		DiscountList bestDiscount = NULL;
		double maxSavings = 0.0;
		double bestFinalTotal = cartTotal;
		double bestCategoryTotal = 0.0;

		discount = Discount_L->next;
		while (discount)
		{
			// 只考虑未过期折扣
			if (now < discount->endDate)
			{
				// 收集符合这个折扣分类的商品
				int maxItems = 100; // 假设最多100种商品
				DiscountItem* discountItems = new DiscountItem[maxItems];
				int itemCount = 0;
				double categoryTotal = 0.0; // 符合分类的商品总价（原价计算）

				cart = Cart_L->next;
				while (cart)
				{
					ProductList p = NULL;
					getProductInfo(Product_L, cart->id, p);

					// 检查商品分类是否匹配
					bool categoryMatch = false;
					if (strcmp(p->category[0], discount->category[0]) == 0)
					{
						if (discount->category[1][0] == '\0' ||
							strcmp(p->category[1], discount->category[1]) == 0)
						{
							categoryMatch = true;
						}
					}

					if (categoryMatch)
					{
						// 如果商品无优惠价，或者商品优惠比率大于折扣比率，则使用折扣
						double discountRatio = 1.0;
						if (p->discount != -1)
						{
							discountRatio = p->discount / p->price; // 商品自身优惠比率
						}

						if (p->discount == -1 || discountRatio > discount->discountRate)
						{
							// 符合条件，加入折扣商品列表
							discountItems[itemCount].productId = p->id;
							discountItems[itemCount].quantity = cart->quantity;
							discountItems[itemCount].originalPrice = p->price;
							strcpy(discountItems[itemCount].category[0], p->category[0]);
							strcpy(discountItems[itemCount].category[1], p->category[1]);

							categoryTotal += p->price * cart->quantity;
							itemCount++;
						}
					}

					cart = cart->next;
				}

				// 检查是否达到折扣的最低金额
				if (categoryTotal >= discount->minAmount)
				{
					// 计算折扣节省的金额
					double discountedTotal = categoryTotal * discount->discountRate;
					double savings = categoryTotal - discountedTotal;

					// 如果这个折扣比之前找到的更好，则更新
					if (savings > maxSavings)
					{
						maxSavings = savings;
						bestDiscount = discount;
						bestFinalTotal = cartTotal - savings;
						bestCategoryTotal = categoryTotal;
					}
				}

				delete[] discountItems;
			}

			discount = discount->next;
		}

		// 如果找到了可用的最优折扣
		if (bestDiscount != NULL)
		{
			// 显示折扣信息
			printf("%s系统已自动为您选择最优惠的折扣！%s\n\n", BOLD GREEN, RESET);

			// 显示折扣详情
			char category[31];
			strcpy(category, bestDiscount->category[0]);
			if (bestDiscount->category[1][0] != '\0')
			{
				strcat(category, "->");
				strcat(category, bestDiscount->category[1]);
			}

			printf("已选择折扣：分类 \"%s\" 折扣率 %.2f\n", category, bestDiscount->discountRate);
			printf("符合折扣条件的商品总价：%.2f 元\n", bestCategoryTotal);
			printf("折扣后价格：%.2f 元\n", bestCategoryTotal * bestDiscount->discountRate);
			printf("节省金额：%.2f 元\n", maxSavings);
			printf("%s最终应付金额：%.2f 元%s\n\n", BOLD YELLOW, bestFinalTotal, RESET);

			// 结算过程
			printf("是否确认结账？(输入 1 确认，0 取消): ");
			int confirm;
			scanf("%d", &confirm);

			if (confirm)
			{
				// 更新库存和积分
				cart = Cart_L->next;
				while (cart)
				{
					ProductList p = NULL;
					getProductInfo(Product_L, cart->id, p);
					p->stock -= cart->quantity; // 减少库存
					cart = cart->next;
				}

				// 更新用户积分（假设每消费1元增加1积分）
				int pointsEarned = (int)bestFinalTotal;
				currUser->points += pointsEarned;
				saveUserToFile(USER_FILE, User_L);
				saveProductToFile(PRODUCT_FILE, Product_L);

				salesLog(0, cartTotal, pointsEarned);

				// 更新会员等级
				if (currUser->memberLevel < 5 && pointsEarned >= currUser->memberLevel * 100)
				{
					currUser->memberLevel++;
				}

				// 清空购物车
				CartList curr = Cart_L->next;
				while (curr)
				{
					CartList temp = curr;
					curr = curr->next;
					delete temp;
				}
				Cart_L->next = NULL;

				printf("%s结账成功！您获得了 %d 积分，当前总积分：%d%s\n", BOLD GREEN, pointsEarned, currUser->points, RESET);

				ProductList gift = Product_L->next;
				int giftID[20];
				int giftCount = 0;
				int foundGift = 0;
				while (gift)
				{
					if (gift->price == -1 && gift->stock > 0 && bestFinalTotal >= gift->discount)
					{
						if (foundGift == 0)
						{
							printf("%s您可以选择以下赠品：%s\n", BOLD GREEN, RESET);
							foundGift = 1;
						}
						giftID[giftCount] = gift->id;
						printf("%d. %s\n", ++giftCount, gift->name);
					}
					gift = gift->next;
				}
				if (foundGift)
				{
					while (1)
					{
						printf("请选择：");
						int giftChoice = 0;
						scanf("%d", &giftChoice);
						if (giftChoice<1 || giftChoice>giftCount)
						{
							printf("赠品序号不存在！\n");
						}
						else
						{
							getProductInfo(Product_L, giftID[giftChoice - 1], gift);
							gift->stock--;
							printf("已获得赠品：%s\n", gift->name);
							break;
						}
					}
				}

				Sleep(2000);
				return USER_MENU;
			}
			else
			{
				printf("%s已取消结账%s\n", YELLOW, RESET);
				Sleep(1500);
				return USER_MENU;
			}
		}
		else
		{
			printf("%s没有找到适用的折扣，按原价结算%s\n\n", YELLOW, RESET);
		}
	}

	int choice = -1;
	int pointsToUse = 0;
	printf("是否使用积分抵扣？（输入 1 使用，0 不使用）: ");
	scanf("%d", &choice);
	if (choice == 1)
	{
		printf("100 积分可抵扣 1 元，您当前积分：%d，最多可使用 %d\n", currUser->points, (int)(cartTotal * 10));
		printf("请输入要使用的积分（输入 0 取消）: ");
		while (1)
		{
			scanf("%d", &pointsToUse);
			if (pointsToUse > currUser->points)
			{
				printf("%s积分不足，请重新输入！%s\n", BOLD RED, RESET);
			}
			else if (pointsToUse < 0)
			{
				printf("%s输入的积分不合法，请重新输入！%s\n", BOLD RED, RESET);
			}
			else if (pointsToUse > (int)(cartTotal * 10))
			{
				printf("%s超过可抵扣积分上限，请重新输入！%s\n", BOLD RED, RESET);
			}
			else
			{
				cartTotal -= pointsToUse / 100.0; // 抵扣金额
				break;
			}
		}
	}


	// 常规结账流程（不使用折扣或没有可用折扣）
	printf("%s应付金额：%.2f 元%s\n", BOLD YELLOW, cartTotal, RESET);
	printf("是否确认结账？(输入 1 确认，0 取消): ");
	int confirm;
	scanf("%d", &confirm);

	if (confirm)
	{
		// 更新库存和积分
		cart = Cart_L->next;
		while (cart)
		{
			ProductList p = NULL;
			getProductInfo(Product_L, cart->id, p);
			p->stock -= cart->quantity; // 减少库存
			cart = cart->next;
		}
		int pointsEarned = (int)cartTotal;
		currUser->points -= pointsToUse; // 扣除使用的积分
		currUser->points += pointsEarned;
		saveUserToFile(USER_FILE, User_L);
		saveProductToFile(PRODUCT_FILE, Product_L);

		salesLog(pointsToUse, cartTotal, pointsEarned);

		// 更新会员等级
		if (currUser->memberLevel < 5 && pointsEarned >= currUser->memberLevel * 100)
		{
			currUser->memberLevel++;
		}

		// 清空购物车
		CartList curr = Cart_L->next;
		while (curr)
		{
			CartList temp = curr;
			curr = curr->next;
			delete temp;
		}
		Cart_L->next = NULL;

		printf("%s结账成功！您获得了 %d 积分，当前总积分：%d%s\n", BOLD GREEN, pointsEarned, currUser->points, RESET);
		
		ProductList gift = Product_L->next;
		int giftID[20];
		int giftCount = 0;
		int foundGift = 0;
		while (gift)
		{
			if (gift->price == -1 && gift->stock > 0 && cartTotal >= gift->discount)
			{
				if (foundGift == 0)
				{
					printf("%s您可以选择以下赠品：%s\n", BOLD GREEN, RESET);
					foundGift = 1;
				}
				giftID[giftCount] = gift->id;
				printf("%d. %s\n", ++giftCount, gift->name);
			}
			gift = gift->next;
		}
		if (foundGift)
		{
			while (1)
			{
				printf("请选择：");
				int giftChoice = 0;
				scanf("%d", &giftChoice);
				if (giftChoice<1 || giftChoice>giftCount)
				{
					printf("赠品序号不存在！\n");
				}
				else
				{
					getProductInfo(Product_L, giftID[giftChoice - 1], gift);
					gift->stock--;
					printf("已获得赠品：%s\n", gift->name);
					break;
				}
			}
		}
		
		Sleep(2000);
	}
	else
	{
		printf("%s已取消结账%s\n", YELLOW, RESET);
		Sleep(1500);
	}

	return USER_MENU;
}

// 显示优惠信息
SystemState discountInfo(ProductList Product_L, DiscountList Discount_L, UserList& User_L)
{
	time_t now = time(0);
	tm* ltm = localtime(&now);

	system("cls");
	printHeader();
	printCentered("优惠信息", WHITE, getConsoleWidth());
	printf("当前系统时间：%04d-%02d-%02d %02d:%02d:%02d\n", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);

	printf("优惠规则：\n");
	printf("1. 部分商品自身存在优惠价\n");
	printf("2. 部分商品分类存在折扣，该分类下商品总价与会员等级达到要求后即可享受\n");
	printf("3. 优惠价和折扣不能叠加使用，即折扣使用商品的原价计算\n");
	printf("4. 单次购物最多只能使用一个折扣\n");
	printf("5. 结算时系统会自动选择最优惠的组合\n");
	printf("6. 结算时可使用积分抵扣，100 积分可抵扣 1 元，不得与折扣同时使用\n");
	printf("7. 结算时积分可抵扣金额不得超过购物车总价的 10%%\n");
	printf("8. 您当前会员等级为 %d", currUser->memberLevel);
	if (currUser->memberLevel == 5)
	{
		printf("，已是最高等级！\n");
	}
	else
	{
		printf("，再消费 %d 可升级\n", currUser->memberLevel * 100);
	}

	printSeparator("-", WHITE, getConsoleWidth());

	// 显示折扣列表
	printCentered("折扣列表", WHITE, getConsoleWidth());
	printDiscountList(Discount_L);
	// 按任意键返回用户菜单
	printf("%s按任意键返回用户菜单%s\n", YELLOW, RESET);
	_getch();
	return USER_MENU;
}

SystemState adminMenu()
{
	time_t now = time(0);
	tm* ltm = localtime(&now);

	system("cls");
	printHeader();
	printf("欢迎回来，管理员 %s！\n", currUser->username);
	printf("当前系统时间：%04d-%02d-%02d %02d:%02d:%02d\n", 1900 + ltm->tm_year, 1 + ltm->tm_mon, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
	printSeparator("-", WHITE, getConsoleWidth());

	int choice = -1;
	printf("请选择要进行的操作：\n");
	printf("1. 商品管理\n");
	printf("2. 用户列表\n");
	printf("3. 优惠管理\n");
	printf("4. 销售统计\n");
	printf("0. 退出登录\n");
	printf("%s输入序号进行对应操作：%s", YELLOW, RESET);
	scanf("%d", &choice);

	switch (choice)
	{
	case 1:
		return PRODUCT_MANAGEMENT;
		break;
	case 2:
		return USER_MANAGEMENT;
		break;
	case 3:
		return DISCOUNT_MANAGEMENT;
		break;
	case 4:
		return SALES_REPORT;
		break;
	case 0:
		currUser = NULL;
		printf("%s已退出登录！正在返回主菜单%s\n", BOLD GREEN, RESET);
		Sleep(1000);
		return MAIN_MENU;
		break;
	default:
		printf("%s输入的序号不存在，请重新输入！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return ADMIN_MENU;
		break;
	}
}

SystemState productManagement(ProductList& Product_L)
{
	system("cls");
	printHeader();
	printCentered("商品管理", WHITE, getConsoleWidth());
	printf("\n");
	printProductList(Product_L);
	while (1)
	{
		printf("请选择要进行的操作：\n");
		printf("1. 添加商品\n");
		printf("2. 修改商品\n");
		printf("3. 删除商品\n");
		printf("4. 搜索商品\n");
		printf("0. 返回管理员菜单\n");
		printf("%s输入序号进行对应操作：%s", YELLOW, RESET);
		int choice = -1;
		scanf("%d", &choice);
		printf("\n");
		switch (choice)
		{
		case 1:
			addProduct(Product_L);
			saveProductToFile(PRODUCT_FILE, Product_L);
			break;
		case 2:
			if(updateProduct(Product_L) == OK)
			{
				saveProductToFile(PRODUCT_FILE, Product_L);
			}
			break;
		case 3:
			if (deleteProduct(Product_L) == OK)
			{
				saveProductToFile(PRODUCT_FILE, Product_L);
			}
			break;
		case 4:
			return SEARCH_PRODUCTS;
		case 0:
			return ADMIN_MENU;
		default:
			printf("%s输入的序号不存在，请重新输入！%s\n\n", BOLD RED, RESET);
			Sleep(1000);
		}
	}
}

SystemState userManagement(UserList& User_L)
{
	system("cls");
	printHeader();
	printCentered("用户列表", WHITE, getConsoleWidth());
	printUserList(User_L);

	// 按任意键返回
	printf("%s按任意键返回管理员菜单%s\n", YELLOW, RESET);
	_getch();

	return ADMIN_MENU;
}

SystemState discountManagement(ProductList Product_L, DiscountList& Discount_L)
{
	system("cls");
	printHeader();
	printCentered("优惠管理", WHITE, getConsoleWidth());
	printDiscountList(Discount_L);

	int choice = -1;
	printf("请选择要进行的操作：\n");
	printf("1. 添加优惠\n");
	printf("0. 返回管理员菜单\n");
	printf("%s输入序号进行对应操作：%s", YELLOW, RESET);
	scanf("%d", &choice);
	switch (choice)
	{
	case 1:
		printf("\n");
		if (addDiscountUI(Product_L, Discount_L))
		{
			printf("%s优惠添加成功！%s\n", BOLD GREEN, RESET);
			Sleep(1000);
		}
		return DISCOUNT_MANAGEMENT;
	case 0:
		return ADMIN_MENU;
	default:
		printf("%s输入的序号不存在，请重新输入！%s\n\n", BOLD RED, RESET);
		Sleep(1000);
		return DISCOUNT_MANAGEMENT;
	}
}

SystemState salesReport(ProductList Product_L)
{
	system("cls");
	printHeader();
	printCentered("销售统计", WHITE, getConsoleWidth());

	FILE* fp;
	if ((fp = fopen(LOG_FILE, "r")) == NULL)
	{
		printf("%s打开日志文件失败！%s\n", BOLD RED, RESET);
		Sleep(1000);
		return ADMIN_MENU;
	}

	int todayOrders = 0;
	double todaySales = 0.0;

	time_t timestamp;
	char username[20];
	int itemCount, id, quantity, pointsUsed, pointsEarned;
	double actualPay;

	int itemSold[2][100] = { 0 }; // 商品 ID 和数量
	int count = 0;

	// 逐条读取日志
	while (fscanf(fp, "%lld %s %d", &timestamp, username, &itemCount) != EOF)
	{
		// 读取商品 ID 和数量
		for (int i = 0; i < itemCount; i++)
		{
			fscanf(fp, "%d %d", &id, &quantity);
			int j;
			for (j = 0; j < count; j++)
			{
				if (itemSold[0][j] == id)
				{
					itemSold[1][j] += quantity; // 累加数量
					break;
				}
			}
			if (j == count)
			{
				itemSold[0][count] = id; // 新商品 ID
				itemSold[1][count] = quantity; // 数量
				count++;
			}
		}

		// 读取积分和实际支付金额
		fscanf(fp, "%d %lf %d", &pointsUsed, &actualPay, &pointsEarned);

		// 计算今天的订单
		if (isToday(timestamp))
		{
			todayOrders++;
			todaySales += actualPay;
		}
	}
	fclose(fp);

	// 对 itemSold 数组进行排序
	for (int i = 0; i < count - 1; i++)
	{
		for (int j = 0; j < count - i - 1; j++)
		{
			if (itemSold[1][j] < itemSold[1][j + 1])
			{
				int tempId = itemSold[0][j];
				itemSold[0][j] = itemSold[0][j + 1];
				itemSold[0][j + 1] = tempId;
				int tempQuantity = itemSold[1][j];
				itemSold[1][j] = itemSold[1][j + 1];
				itemSold[1][j + 1] = tempQuantity;
			}
		}
	}

	// 打印统计结果
	time_t now = time(NULL);
	struct tm* timeinfo = localtime(&now);
	printf("当前日期：%04d-%02d-%02d\n", 1900 + timeinfo->tm_year, 1 + timeinfo->tm_mon, timeinfo->tm_mday);
	printf("今日订单数：%d\n", todayOrders);
	printf("今日销售额：%.2f 元\n", todaySales);
	printf("今日热销商品（按数量降序）：\n");
	printAligned("商品 ID", 10);
	printAligned("商品名称", 50);
	printAligned("数量", 10);
	printf("\n");
	printSeparator("-", WHITE, getConsoleWidth());
	for (int i = 0; i < count; i++)
	{
		ProductList p = NULL;
		getProductInfo(Product_L, itemSold[0][i], p);
		if (p != NULL)
		{
			printf("%-10d", itemSold[0][i]);
			printAligned(p->name, 50);
			printf("%-10d\n", itemSold[1][i]);
		}
	}
	printSeparator("-", WHITE, getConsoleWidth());
	printf("%s按任意键返回管理员菜单%s\n", YELLOW, RESET);
	_getch();
	return ADMIN_MENU;
}
