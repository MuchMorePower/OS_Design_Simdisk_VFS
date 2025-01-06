#include "interaction.h"


// 定义宽字符数组
wchar_t InputFileName[50] = L"YDAI_INPUT";
wchar_t OuputFileName[50] = L"YDAI_OUTPUT";
wchar_t CommFlagsFileName[50] = L"YDAI_COMMFLAGS";
const wchar_t* UserFileName = L"USER";

Message shell_to_simdisk;
Message simdisk_to_shell;

Command command;

ShareMemoryManager sharememory_manager;

HANDLE hMapFileUser;
User* pBufUser = NULL;

Reader_And_Writer rw_Controller;

HANDLE hMapFileCommFlags;
CommunicationFlags commFlags;
CommunicationFlags* pCommFlags = &commFlags;

HANDLE hMapFileInput;
Message* pInput_Message = NULL;

HANDLE hMapFileOutput;
Message Output_Message;
Message* pOutput_Message = &Output_Message;

Producer_And_Consumer* pc_Controller = new Producer_And_Consumer();

// 从共享内存中读取数据到 shell_to_simdisk 对象中
void Input_Data_From_ShareMemory() {
	// 通知 shell 端 simdisk 需要输入数据
	pCommFlags->Simdisk_Request_Shell_Input();

	// 等待共享内存满（数据可读），然后锁定访问
	pc_Controller->P(pc_Controller->getFullSem());
	pc_Controller->LockMutex();

	// 打开命名共享内存以读取输入数据
	bool flag = sharememory_manager.OpenShareMemory(hMapFileInput, pInput_Message, InputFileName);
	if (!flag) {  // 如果打开失败，打印错误信息
		cout << "无法打开共享内存" << endl;
	}

	// 将共享内存中的数据复制到 shell_to_simdisk 对象
	shell_to_simdisk.copy_Data(pInput_Message);

	// 解锁访问并释放空信号量，通知可以写入新的数据
	pc_Controller->UnlockMutex();
	pc_Controller->V(pc_Controller->getEmptySem());

	// 通知 shell 数据已读取完成，停止输入或输出
	pc_Controller->V(pc_Controller->getReadySem());
	pCommFlags->Pause_Shell();

	// 关闭共享内存
	sharememory_manager.CloseShareMemory(hMapFileInput, pInput_Message);
	return;
}

// 将 simdisk_to_shell 对象中的数据写入到共享内存中，传递给 shell
void Output_Data_Into_ShareMemory() {
	// 通知 shell 端 simdisk 需要输出数据
	pCommFlags->Simdisk_Request_Shell_Output();

	// 创建共享内存文件用于输出数据
	bool flag = sharememory_manager.CreateShareMemory(hMapFileOutput, pOutput_Message, OuputFileName);
	if (!flag) {  // 如果创建失败，打印错误信息
		cout << "无法打开共享内存" << endl;
		return;
	}

	// 等待共享内存为空（可写入），然后锁定访问
	pc_Controller->P(pc_Controller->getEmptySem());
	pc_Controller->LockMutex();

	// 清空共享内存的缓冲区
	pOutput_Message->clear_Message();
	// 将 simdisk_to_shell 对象的数据复制到共享内存
	pOutput_Message->copy_Data(&simdisk_to_shell);
	// 输出共享内存中的内容（调试用途）
	pOutput_Message->output_Data();

	// 解锁访问并释放满信号量，通知可以读取新的数据
	pc_Controller->UnlockMutex();
	pc_Controller->V(pc_Controller->getFullSem());

	// 等待 shell 端读取完成后，清空 simdisk_to_shell 数据
	pc_Controller->P(pc_Controller->getReadySem());
	simdisk_to_shell.clear_Message(); // 清空对象数据
	pCommFlags->Pause_Shell();        // 通知 shell 暂停输入和输出

	// 关闭共享内存
	sharememory_manager.CloseShareMemory(hMapFileOutput, pOutput_Message);

	return;
}

// 与 shell 端建立连接
void ConnectToShell() {
    bool flag = false;

    // 提示等待 shell 端创建共享文件并尝试连接
    cout << "等待 shell 端创建共享文件 , 尝试与 shell 端建立连接......" << endl;

    // 等待 shell 端运行，建立连接
    pc_Controller->P(pc_Controller->getConnectSem()); // 等待 shell 端信号量，表示 shell 启动
    pc_Controller->V(pc_Controller->getGetSem());    // 通知 shell simdisk 已准备好

    // 打开用户共享内存
    flag = sharememory_manager.OpenShareMemory(hMapFileUser, pBufUser, UserFileName);
    if (!flag) {
        cout << "无法打开用户共享内存" << endl;
        return;
    }

    // 等待 shell 端输入用户名
    pc_Controller->P(pc_Controller->getUserSem());

    // 创建宽字符数组，用于存储转换后的用户名
    wchar_t wNameIn[50] = L"YDAI_INPUT";
    wchar_t wNameOut[50] = L"YDAI_OUTPUT";
    wchar_t wNameIoo[50] = L"YDAI_COMMFLAGS";

    // 将用户名从 char* 转换为 wchar_t*
    MultiByteToWideChar(CP_ACP, 0, pBufUser->user_name, -1, wNameIn + wcslen(wNameIn), 50 - wcslen(wNameIn));
    MultiByteToWideChar(CP_ACP, 0, pBufUser->user_name, -1, wNameOut + wcslen(wNameOut), 50 - wcslen(wNameOut));
    MultiByteToWideChar(CP_ACP, 0, pBufUser->user_name, -1, wNameIoo + wcslen(wNameIoo), 50 - wcslen(wNameIoo));

    // 将转换后的宽字符数组赋值给共享内存文件名变量
    wcscpy_s(InputFileName, wNameIn);
    wcscpy_s(OuputFileName, wNameOut);
    wcscpy_s(CommFlagsFileName, wNameIoo);

    // 将用户名复制到当前用户变量
    strcpy_s(Current_User, sizeof(Current_User), pBufUser->user_name);

    // 等待 0.05 秒，避免过快关闭共享内存
    Sleep(50);

    // 通知 shell simdisk 准备完毕
    pc_Controller->V(pc_Controller->getReadySem());

    // 关闭用户共享内存
    sharememory_manager.CloseShareMemory(hMapFileUser, pBufUser);

    // 等待一小段时间以防止过快操作
    Sleep(10);

    return;
}

// 发送一条消息到 shell 端
void Deliver_Message_To_Shell(string message_to_shell) {
	// 设置消息数量为 1
	simdisk_to_shell.message_num = 1;

	// 将消息内容复制到 simdisk_to_shell 对象
	strcpy_s(simdisk_to_shell.message[0], MAX_MESSAGE_LENGTH, message_to_shell.c_str());

	// 调用输出函数，将数据写入共享内存并发送到 shell
	Output_Data_Into_ShareMemory();
}

// 发送多条消息到 shell 端
void send_Messages(string* mes, int num) {
	// 设置消息数量
	simdisk_to_shell.message_num = num;

	// 将每条消息内容复制到 simdisk_to_shell 对象
	for (int i = 0; i < num; i++) {
		strcpy_s(simdisk_to_shell.message[i], MAX_MESSAGE_LENGTH, mes[i].c_str());
	}

	// 调用输出函数，将数据写入共享内存并发送到 shell
	Output_Data_Into_ShareMemory();
}

// 解析用户输入的命令
void Parse_Command(Command& input_command) {
    // Step 1: 清空命令对象，确保没有遗留的旧数据
    input_command.clear_command();

    // Step 2: 从共享内存中读取数据并分解命令字符串
    Input_Data_From_ShareMemory(); // 从共享内存中获取 shell 的输入数据

    // 将读取的命令逐条存入 Command 对象
    input_command.cmd_count = shell_to_simdisk.message_num; // 记录命令条数
    for (int i = 0; i < input_command.cmd_count; i++) {
        input_command.cmd[i] = shell_to_simdisk.message[i]; // 逐条存储命令
    }

    // Step 3: 根据命令字符串解析命令类型
    input_command.Determine_Command_Type();
}

// 展示帮助菜单，向 Shell 端发送命令提示信息
void Display_HelpMenu() {
    // 定义命令提示信息
    string message_to_shell[20];
    message_to_shell[0] = "--------------------------------------------------------------------------------\n";
    message_to_shell[1] = "YDFS                                                                   Help Menu\n";
    send_Messages(message_to_shell, 2);
    message_to_shell[0] = "--------------------------------------------------------------------------------\n\n";
    message_to_shell[1] = "    info                                             展示文件系统的基本信息\n";
    message_to_shell[2] = "    cd {path}                                        改变工作目录\n";
    message_to_shell[3] = "    dir [path] [s]                                   展示目录\n";
    message_to_shell[4] = "    md {dirname} [path] {0/1/2}                      新建目录\n";
    message_to_shell[5] = "    rd {path}                                        删除目录\n";
    message_to_shell[6] = "    newfile {filename} [path] {0/1/2}                新建文件\n";
    message_to_shell[7] = "    cat {path}                                       打开文件\n";
    message_to_shell[8] = "    copy<host> {HostPath} {VFSPath} {0/1} {0/1/2}    拷贝主机文件\n";
    message_to_shell[9] = "    copy<ydfs> {SrcPath} {DestPath} {0/1/2}          拷贝文件系统文件\n";
    message_to_shell[10] = "    del {path}                                       删除文件\n";
    message_to_shell[11] = "    check                                            检查并修复文件一致性\n";
    message_to_shell[12] = "    help                                             展示帮助目录\n";
    message_to_shell[13] = "    exit                                             退出文件系统\n";
    message_to_shell[14] = "    ls                                               展示文件系统下的目录与文件信息\n";
    message_to_shell[15] = "\n";
    message_to_shell[16] = "    []内部为可选填入,不填入表示使用相对路径\n";
    message_to_shell[17] = "    {}内部为必须填入\n";
    message_to_shell[18] = "    {0/1/2}表示权限控制, 0代表其他用户无读写权限, 1代表其他用户有读权限，无写权限, 2代表其他用户有读写权限\n";
    message_to_shell[19] = "    copy<host>命令中, {0/1}中, 0表示从主机拷贝到VFS中, 1表示从VFS拷贝到主机中\n";

    // 发送帮助信息到 Shell
    send_Messages(message_to_shell, 20);

    // 额外提示路径支持
    message_to_shell[0] = "    YDFS支持绝对路径与相对路径\n";
    message_to_shell[1] = "    绝对路径 : /aaa/bbb/ccc\n";
    message_to_shell[2] = "    相对路径 : ./bbb/ccc 或 ccc\n\n";
    send_Messages(message_to_shell, 3);

    return;
}

// 检查路径是否存在
bool isPath_Exist(unsigned int path_index) {
    // 如果路径不存在，向 Shell 端发送错误消息
    if (path_index == Nothing) {
        Deliver_Message_To_Shell("该路径不存在!\n");
        return false; // 返回路径不存在的结果
    }
    return true; // 返回路径存在的结果
}




