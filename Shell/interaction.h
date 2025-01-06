#ifndef interaction
#define interaction

#include<iostream>
#include<cstdio>
#include<cstdlib>
#include<cmath>
#include<algorithm>
#include<vector>
#include<queue>
#include<cstring>
#include<string>
#include<string.h>
#include<fstream>
#include<istream>
#include<ostream>
#include<iomanip>
#include<Windows.h>
#include<conio.h>
#include<tchar.h>
#include<atlconv.h> 

using namespace std;
#define BUFFER_SIZE 8192

extern char UserFileName[];
extern char InputFileName[50];        // 声明
extern char OuputFileName[50];       // 声明
extern char CommFlagsFileName[50];       // 声明

const int MAX_MESSAGES = 20; // 可以存储的最大消息数
const int MAX_MESSAGE_LENGTH = 350; // 每条消息的最大长度

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

	bool Shell_Send_Message() {
		return request_to_shell == 1;
	}

	bool Shell_Get_Message() {
		return request_to_shell == 2;
	}
};

// ShareMemoryManager 类用于管理共享内存的创建、打开和关闭操作
class ShareMemoryManager {
public:
	// 错误处理函数，打印错误信息
	void HandleError(const char* msg, char* name) {
		int error = GetLastError();  // 获取错误代码
		_tprintf(TEXT("Error: %s for %s (Error Code: %d).\n"), msg, name, error);
	}

	// 创建共享内存
	template <typename T>
	bool CreateShareMemory(HANDLE& hMapFile, T*& pMemory, char* name) {
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
	bool OpenShareMemory(HANDLE& hMapFile, T*& pMemory, char* name) {
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
		sem_empty = CreateSemaphore(NULL, 1, 1, "GlobalSemaphoreEmpty"); // 共享内存为空，初始化为 1
		sem_full = CreateSemaphore(NULL, 0, 1, "GlobalSemaphoreFull");  // 共享内存已满，初始化为 0
		sem_ready = CreateSemaphore(NULL, 0, 1, "GlobalSemaphoreReady"); // 用于关闭共享内存，初始化为 0
		sem_connect = CreateSemaphore(NULL, 0, 1, "GlobalSemaphoreConnect"); // 等待与 shell 建立连接，初始化为 0
		sem_user = CreateSemaphore(NULL, 0, 1, "GlobalSemaphoreUser"); // 等待 simdisk 获取用户名，初始化为 0
		sem_get = CreateSemaphore(NULL, 0, 1, "GlobalSemaphoreGet"); // 等待 shell 创建共享内存，初始化为 0
		hMutex = CreateMutex(NULL, FALSE, "GlobalShareMemoryMutex"); // 创建互斥锁，确保对共享内存的互斥访问

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

// 全局变量的声明

// 消息相关
extern Message shell_to_simdisk;
// 用于存储从 Shell 端发送到 Simdisk 端的消息。

// 用户信息相关
extern HANDLE hMapFileUser;
// 用户信息共享内存的句柄，用于与共享内存交互。
extern User user;
// 当前用户信息对象，存储用户名等信息。
extern User* pBufUser;
// 指向用户信息的指针，用于操作共享内存中的用户信息。

// 通信标志相关
extern HANDLE hMapFileCommFlags;
// 通信标志共享内存的句柄，用于管理 Shell 和 Simdisk 的通信状态。
extern CommunicationFlags* pCommFlags;
// 指向通信标志共享内存的指针，用于读取和修改通信状态。

// 输入共享内存相关
extern HANDLE hMapFileInput;
// 输入共享内存的句柄，用于管理从 Shell 到 Simdisk 的数据流。
extern Message Input_Message;
// 用于存储用户输入的消息对象。
extern Message* pInput_Message;
// 指向输入消息对象的指针，方便操作共享内存。

// 输出共享内存相关
extern HANDLE hMapFileOutput;
// 输出共享内存的句柄，用于管理从 Simdisk 到 Shell 的数据流。
extern Message* pOutput_Message;
// 指向输出消息共享内存的指针，用于读取 Simdisk 端的输出数据。

// 共享内存管理器
extern ShareMemoryManager sharememory_manager;
// 管理共享内存的创建、映射、关闭等操作的管理器对象。

// 生产者与消费者信号量控制器
extern Producer_And_Consumer* pc_Controller;
// 使用生产者-消费者模型控制共享内存的数据流和同步。


// 将 Shell 进程中的数据发送到 Simdisk 进程
void Send_Messages_To_ShareMemory();

// 从 Simdisk 进程接收数据并在 Shell 中输出
void Receive_Messages_From_ShareMemory();

#endif // !interaction


