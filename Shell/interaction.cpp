#include "interaction.h"


// 文件名定义：用于标识共享内存的名称
char UserFileName[] = "USER";                  // 用户信息共享内存名称
char InputFileName[50] = "YDAI_INPUT";         // 输入共享内存名称
char OuputFileName[50] = "YDAI_OUTPUT";        // 输出共享内存名称
char CommFlagsFileName[50] = "YDAI_COMMFLAGS"; // 通信标志共享内存名称

// 全局变量定义

// 消息相关
Message shell_to_simdisk; // 存储从 Shell 端向 Simdisk 端发送的消息

// 用户信息相关
HANDLE hMapFileUser;       // 用户信息共享内存的句柄
User user;                 // 当前用户对象
User* pBufUser = &user;    // 指向用户对象的指针，用于操作共享内存中的用户信息

// 通信标志相关
HANDLE hMapFileCommFlags;          // 通信标志共享内存的句柄
CommunicationFlags* pCommFlags = nullptr; // 指向通信标志共享内存的指针

// 输入共享内存相关
HANDLE hMapFileInput;             // 输入共享内存的句柄
Message Input_Message;            // Shell 端存储输入消息的对象
Message* pInput_Message = &Input_Message; // 指向输入消息对象的指针，用于操作共享内存

// 输出共享内存相关
HANDLE hMapFileOutput;             // 输出共享内存的句柄
Message* pOutput_Message = nullptr; // 指向输出消息共享内存的指针

// 共享内存管理器
ShareMemoryManager sharememory_manager; // 用于管理共享内存的创建、映射和关闭

// 生产者与消费者信号量控制器
Producer_And_Consumer* pc_Controller = new Producer_And_Consumer(); // 用于控制共享内存的生产者-消费者模型


// 将 Shell 进程中的数据发送到 Simdisk 进程
void Send_Messages_To_ShareMemory() {
	// 创建共享内存，用于存储输入数据
	bool flag = sharememory_manager.CreateShareMemory(hMapFileInput, pInput_Message, InputFileName);
	if (!flag) {
		cout << "无法打开共享内存" << endl;
		return; // 如果创建失败，退出函数
	}

	// 等待共享内存为空，确保可以写入数据
	pc_Controller->P(pc_Controller->getEmptySem());
	pc_Controller->LockMutex(); // 加锁以确保写入操作的原子性

	// 清空共享内存中的消息缓冲区
	pInput_Message->clear_Message();

	// 从标准输入获取数据，将其存入共享内存
	while (cin >> pInput_Message->message[pInput_Message->message_num]) {
		pInput_Message->message_num++; // 增加消息数量计数
		if (cin.get() == '\n') break; // 检测到换行符结束输入
	}

	pc_Controller->UnlockMutex(); // 解锁
	pc_Controller->V(pc_Controller->getFullSem()); // 通知共享内存已满，可以读取

	// 等待 Simdisk 进程处理完成
	pc_Controller->P(pc_Controller->getReadySem());

	// 关闭共享内存，释放资源
	sharememory_manager.CloseShareMemory(hMapFileInput, pInput_Message);
	return;
}

// 从 Simdisk 进程接收数据并在 Shell 中输出
void Receive_Messages_From_ShareMemory() {
	// 等待共享内存有数据可读
	pc_Controller->P(pc_Controller->getFullSem());
	pc_Controller->LockMutex(); // 加锁以确保读取操作的原子性

	// 打开共享内存，用于读取输出数据
	bool flag = sharememory_manager.OpenShareMemory(hMapFileOutput, pOutput_Message, OuputFileName);
	if (!flag) {
		cout << "无法打开共享内存" << endl;
		pc_Controller->UnlockMutex(); // 解锁
		return; // 如果打开失败，退出函数
	}

	// 清空消息对象，以确保数据不会混乱
	shell_to_simdisk.clear_Message();

	// 从共享内存中读取数据到消息对象
	shell_to_simdisk.copy_Data(pOutput_Message);

	// 将数据输出到标准输出
	shell_to_simdisk.output_Data();

	pc_Controller->UnlockMutex(); // 解锁
	pc_Controller->V(pc_Controller->getEmptySem()); // 通知共享内存已空，可以写入

	// 通知 Simdisk 进程，处理完成
	pc_Controller->V(pc_Controller->getReadySem());

	// 关闭共享内存，释放资源
	sharememory_manager.CloseShareMemory(hMapFileOutput, pOutput_Message);
	return;
}

