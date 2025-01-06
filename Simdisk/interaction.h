#ifndef interaction
#define interaction

#include<iostream>
#include<string>
#include<unordered_map>
#include<Windows.h>
#include<tchar.h>

#include"constant.h"
using namespace std;

#define BUFFER_SIZE 8192
// 定义宽字符数组
extern wchar_t InputFileName[50];        // 声明
extern wchar_t OuputFileName[50];       // 声明
extern wchar_t CommFlagsFileName[50];       // 声明
extern const wchar_t* UserFileName;   // 声明

// Command 类用于用户输入的命令分解，便于判断命令类型
class Command {
public:
	unsigned int cmd_count;  // 命令中的单词数量
	string cmd[20];          // 存储分解后的命令部分
	unsigned int type;       // 命令类型，用于区分不同命令

	// 构造函数，初始化命令
	Command() {
		clear_command();  // 初始化时清空命令
	}

	// 清空命令数据
	void clear_command() {
		cmd_count = 0;  // 重置命令计数
		for (int i = 0; i < 20; i++)
			cmd[i] = "";  // 清空命令数组中的每个元素
		type = Nothing;  // 设置命令类型为未定义
	}

	// 根据命令的字符串内容判断命令类型
	void Determine_Command_Type() {
		// 定义命令与类型的映射关系
		static std::unordered_map<std::string, int> command_map = {
			{"exit", 0}, {"EXIT", 0}, {"Exit", 0},  // 退出命令
			{"info", 1},                         // 显示信息
			{"cd", 2},                           // 切换目录
			{"dir", 3},                          // 列出目录内容
			{"md", 4},                           // 创建目录
			{"rd", 5},                           // 删除目录
			{"newfile", 6},                      // 创建新文件
			{"cat", 7},                          // 查看文件内容
			{"copy<host>", 8}, {"copy<ydfs>", 8}, // 文件复制（本地或远程）
			{"del", 9},                          // 删除文件
			{"check", 10},                       // 检查文件系统
			{"ls", 11},                          // 显示目录内容（简版）
			{"help",12}                          // 显示帮助信息
		};

		// 在映射表中查找命令类型
		auto it = command_map.find(this->cmd[0]);  // 查找命令的第一个部分（cmd[0]）
		if (it != command_map.end()) {
			this->type = it->second;  // 如果找到了匹配的命令，设置命令类型
		}
		else {
			this->type = Nothing;  // 如果未找到匹配的命令，将类型设置为 Nothing
		}
	}
};

// User 类用于存储用户名，与 shell 端建立连接时使用
class User {
public:
	char user_name[50];  // 用户名，最大长度为 50

	// 构造函数，初始化用户名为空
	User() {
		for (int i = 0; i < sizeof(user_name); i++) {
			user_name[i] = '\0';  // 初始化用户名为空字符
		}
	}

	// 设置用户名
	void set_userName(char* name) {
		// 将传入的用户名字符串复制到 user_name 数组中
		for (int i = 0; i < 50; i++) {
			this->user_name[i] = name[i];
		}
	}
};

// Reader_And_Writer 类用于实现读写者问题中的信号量机制
// 控制读者和写者的访问权限，保证互斥和同步
class Reader_And_Writer {
public:
	HANDLE hSemaphore;  // 控制读写器的信号量，保证写操作互斥
	HANDLE hReaderEvent;  // 控制读者事件，用于协调多个读者的访问
	int readerCount;      // 记录当前活跃读者的数量

	// 构造函数：初始化信号量和事件，准备协调读写操作
	Reader_And_Writer() {
		readerCount = 0;  // 初始化读者计数为 0

		// 创建一个信号量，初始值为 1，表示允许一个写者访问资源
		hSemaphore = CreateSemaphore(NULL, 1, 1, L"Global\\RW_Semaphore");
		if (hSemaphore == NULL) {
			std::cerr << "CreateSemaphore error: " << GetLastError() << std::endl;
		}

		// 创建一个手动重置事件，初始状态为有信号（可用）
		hReaderEvent = CreateEvent(NULL, TRUE, TRUE, L"Global\\ReaderEvent");
		if (hReaderEvent == NULL) {
			std::cerr << "CreateEvent error: " << GetLastError() << std::endl;
		}
	}

	// 析构函数：释放信号量和事件资源
	~Reader_And_Writer() {
		CloseHandle(hSemaphore);
		CloseHandle(hReaderEvent);
	}

	// Set_Writer_XLock：相当于 P 操作，试图获取写权限
	void Set_Writer_XLock() {
		WaitForSingleObject(hSemaphore, INFINITE);  // 获取写权限，阻塞直到可以写
	}

	// Release_Writer_XLock：相当于 V 操作，释放写权限
	void Release_Writer_XLock() {
		ReleaseSemaphore(hSemaphore, 1, NULL);  // 释放信号量，允许其他写操作
	}

	// Set_Reader_SLock：相当于 P 操作，控制读者的进入
	void Set_Reader_SLock() {
		WaitForSingleObject(hReaderEvent, INFINITE);  // 等待可读事件信号，控制并发访问

		if (readerCount == 0) {
			// 如果是第一个读者到达，则阻止写操作
			WaitForSingleObject(hSemaphore, INFINITE);  // 等待写者的访问权限
		}
		readerCount++;  // 读者计数器加 1
		SetEvent(hReaderEvent);  // 允许其他读者继续进入
	}

	// Release_Reader_SLock：相当于 V 操作，控制读者的离开
	void Release_Reader_SLock() {
		WaitForSingleObject(hReaderEvent, INFINITE);  // 等待可读事件信号

		readerCount--;  // 读者计数器减 1
		if (readerCount == 0) {
			// 如果是最后一个读者离开，允许写操作
			ReleaseSemaphore(hSemaphore, 1, NULL);
		}
		SetEvent(hReaderEvent);  // 重置事件信号，允许其他读者继续进入
	}
};

// Message 类用于与 shell 交换数据，存储和操作消息内容
class Message {
public:
	char message[MAX_MESSAGES][MAX_MESSAGE_LENGTH]; // 使用固定大小的字符数组存储消息
	int message_num;  // 当前消息的数量

	// 构造函数：初始化消息内容
	Message() {
		clear_Message();  // 清空消息
	}

	// 清空消息数据
	void clear_Message() {
		message_num = 0;  // 重置消息数量为 0
		memset(message, 0, sizeof(message));  // 清空消息内容（每个字符都设为 '\0'）
	}

	// 拷贝另一个 Message 对象的消息数据
	void copy_Data(Message* pObject) {
		this->message_num = pObject->message_num;  // 复制消息数量
		// 逐个字符复制消息内容
		for (int i = 0; i < MAX_MESSAGES; i++) {
			for (int j = 0; j < MAX_MESSAGE_LENGTH; j++) {
				this->message[i][j] = pObject->message[i][j];
			}
		}
	}

	// 输出消息数据到控制台
	void output_Data() {
		for (int i = 0; i < this->message_num; i++) {
			cout << this->message[i];  // 输出每条消息
		}
	}
};

// CommunicationFlags 类用于管理 simdisk 与 shell 之间的通信标志
class CommunicationFlags {
public:
	int request_to_shell; // 状态标识位，表示 simdisk 端告诉 shell 端当前的需求

	// 构造函数：初始化标志位为 0（默认不做任何操作）
	CommunicationFlags() {
		request_to_shell = 0;
	}

	// Simdisk 请求 shell 输入数据
	void Simdisk_Request_Shell_Input() {
		request_to_shell = 1;  // 设置标志位为 1，表示请求 shell 输入数据
	}

	// Simdisk 请求 shell 输出共享内存中的数据
	void Simdisk_Request_Shell_Output() {
		request_to_shell = 2;  // 设置标志位为 2，表示请求 shell 输出数据
	}

	// 停止 shell 端操作（即数据已读入完成，后续既不输入也不输出）
	void Pause_Shell() {
		request_to_shell = 0;  // 设置标志位为 0，表示暂停 shell 端的输入和输出
	}
};

// ShareMemoryManager 类用于管理共享内存的创建、打开和关闭操作
class ShareMemoryManager {
public:
	// 错误处理函数，打印错误信息
	void HandleError(const char* msg, const wchar_t* name) {
		int error = GetLastError();  // 获取错误代码
		_tprintf(TEXT("Error: %s for %s (Error Code: %d).\n"), msg, name, error);
	}

	// 创建共享内存
	template <typename T>
	bool CreateShareMemory(HANDLE& hMapFile, T*& pMemory, const wchar_t* name) {
		// 创建文件映射对象（共享内存）
		hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUFFER_SIZE, name);
		if (hMapFile == NULL) {  // 如果创建失败，打印错误并返回 false
			HandleError("Failed to create file mapping object", name);
			return false;
		}

		// 映射文件的视图，获得指向共享内存的指针
		pMemory = (T*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
		if (pMemory == NULL) {  // 如果映射失败，打印错误并关闭文件映射对象
			HandleError("Failed to map view of file", name);
			CloseHandle(hMapFile);
			return false;
		}

		return true;  // 成功创建共享内存并映射
	}

	// 打开已存在的共享内存
	template <typename T>
	bool OpenShareMemory(HANDLE& hMapFile, T*& pMemory, const wchar_t* name) {
		// 打开已有的文件映射对象（共享内存）
		hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
		if (hMapFile == NULL) {  // 如果打开失败，打印错误并返回 false
			HandleError("Failed to open file mapping object", name);
			return false;
		}

		// 映射文件的视图，获得指向共享内存的指针
		pMemory = (T*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
		if (pMemory == NULL) {  // 如果映射失败，打印错误并关闭文件映射对象
			HandleError("Failed to map view of file", name);
			CloseHandle(hMapFile);
			return false;
		}

		return true;  // 成功打开并映射共享内存
	}

	// 关闭共享内存
	template <typename T>
	void CloseShareMemory(HANDLE& hMapFile, T*& pMemory) {
		// 如果共享内存指针不为空，解除映射
		if (pMemory) {
			UnmapViewOfFile(pMemory);
		}

		// 如果文件映射对象不为空，关闭句柄
		if (hMapFile) {
			CloseHandle(hMapFile);
		}

		Sleep(10);  // 为了防止频繁调用，延迟 10 毫秒
	}
};


// Producer_And_Consumer 类实现了生产者-消费者模型的同步控制，用于管理信号量和互斥锁
class Producer_And_Consumer {
private:
	HANDLE sem_empty;   // 信号量：表示共享内存为空，可以写入
	HANDLE sem_full;    // 信号量：表示共享内存已满，可以读取
	HANDLE sem_ready;   // 信号量：用于判断是否关闭共享内存
	HANDLE sem_connect; // 信号量：等待与 shell 端建立连接
	HANDLE sem_user;    // 信号量：等待 simdisk 端获取用户输入的用户名
	HANDLE sem_get;     // 信号量：等待 shell 端创建共享内存
	HANDLE hMutex;      // 互斥锁：确保对共享内存的互斥访问

public:
	// 构造函数：创建各类信号量和互斥锁
	Producer_And_Consumer() {
		// 使用命名信号量，多个进程可以通过名称访问同一个信号量
		sem_empty = CreateSemaphore(NULL, 1, 1, L"GlobalSemaphoreEmpty"); // 共享内存为空，初始化为 1
		sem_full = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreFull");  // 共享内存已满，初始化为 0
		sem_ready = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreReady"); // 用于关闭共享内存，初始化为 0
		sem_connect = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreConnect"); // 等待与 shell 建立连接，初始化为 0
		sem_user = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreUser"); // 等待 simdisk 获取用户名，初始化为 0
		sem_get = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreGet"); // 等待 shell 创建共享内存，初始化为 0
		hMutex = CreateMutex(NULL, FALSE, L"GlobalShareMemoryMutex"); // 创建互斥锁，确保对共享内存的互斥访问

		// 检查信号量和互斥锁是否创建成功
		if (!sem_empty || !sem_full || !hMutex || !sem_connect || !sem_ready) {
			std::cerr << "无法创建信号量或互斥锁" << std::endl; // 如果创建失败，打印错误并退出程序
			exit(1);
		}
	}

	// 析构函数：关闭信号量和互斥锁
	~Producer_And_Consumer() {
		CloseHandle(sem_empty);   // 关闭信号量 sem_empty
		CloseHandle(sem_full);    // 关闭信号量 sem_full
		CloseHandle(sem_ready);   // 关闭信号量 sem_ready
		CloseHandle(sem_connect); // 关闭信号量 sem_connect
		CloseHandle(sem_user);    // 关闭信号量 sem_user
		CloseHandle(sem_get);     // 关闭信号量 sem_get
		CloseHandle(hMutex);      // 关闭互斥锁
	}

	// P 操作（Wait）：请求信号量，阻塞当前线程直到信号量可用
	void P(HANDLE sem) {
		WaitForSingleObject(sem, INFINITE); // 等待信号量，若不可用则阻塞
	}

	// V 操作（Signal）：释放信号量，唤醒等待的线程
	void V(HANDLE sem) {
		ReleaseSemaphore(sem, 1, NULL); // 释放信号量，唤醒一个等待线程
	}

	// LockMutex：请求互斥锁，确保对共享内存的独占访问
	void LockMutex() {
		WaitForSingleObject(hMutex, INFINITE); // 等待互斥锁，若不可用则阻塞
	}

	// UnlockMutex：释放互斥锁，允许其他线程访问共享内存
	void UnlockMutex() {
		ReleaseMutex(hMutex); // 释放互斥锁
	}

	// 获取各类信号量
	HANDLE getEmptySem() { return sem_empty; }
	HANDLE getFullSem() { return sem_full; }
	HANDLE getReadySem() { return sem_ready; }
	HANDLE getConnectSem() { return sem_connect; }
	HANDLE getUserSem() { return sem_user; }
	HANDLE getGetSem() { return sem_get; }
};

// 以下是共享内存及相关数据的声明

// 用于 simdisk 与 shell 之间的数据交换
extern Message shell_to_simdisk;    // shell 发送到 simdisk 的消息
extern Message simdisk_to_shell;    // simdisk 发送到 shell 的消息

// 当前的命令输入
extern Command command;             // 用于存储 shell 端输入的命令

// 用户输入的共享内存
extern HANDLE hMapFileUser;         // 用户共享内存文件句柄
extern User* pBufUser;              // 指向用户共享内存的指针

// 控制读写器同步
extern Reader_And_Writer rw_Controller; // 读写器控制器

// 通信标志
extern HANDLE hMapFileCommFlags;     // 通信标志共享内存文件句柄
extern CommunicationFlags commFlags; // 通信标志实例
extern CommunicationFlags* pCommFlags; // 通信标志指针

// 输入消息共享内存
extern HANDLE hMapFileInput;          // 输入消息共享内存文件句柄
extern Message* pInput_Message;       // 指向输入消息共享内存的指针

// 输出消息共享内存
extern HANDLE hMapFileOutput;         // 输出消息共享内存文件句柄
extern Message Output_Message;        // 输出消息
extern Message* pOutput_Message;      // 指向输出消息共享内存的指针

// 共享内存管理实例
extern ShareMemoryManager sharememory_manager; // 共享内存管理实例

// 生产者消费者模型控制器
extern Producer_And_Consumer* pc_Controller; // 生产者消费者模型控制器实例


// 从共享内存中读取数据到 shell_to_simdisk 对象中
void Input_Data_From_ShareMemory();

// 将 simdisk_to_shell 对象中的数据写入到共享内存中，传递给 shell
void Output_Data_Into_ShareMemory();

// 与 shell 端建立连接
void ConnectToShell();

// 发送一条消息到 shell 端
void Deliver_Message_To_Shell(string message_to_shell);

// 发送多条消息到 shell 端
void send_Messages(string* mes, int num);

// 解析用户输入的命令
void Parse_Command(Command& input_command);

// 展示帮助菜单，向 Shell 端发送命令提示信息
void Display_HelpMenu();

// 检查路径是否存在
bool isPath_Exist(unsigned int path_index);
#endif

