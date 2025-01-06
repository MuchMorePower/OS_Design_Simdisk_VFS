
#include"interaction.h"
using namespace std;

/**
 * 与 Simdisk 端建立连接，获取用户名并设置共享内存。
 */
void ConnectToSimdisk() {
    // 创建共享内存用于保存用户名
    bool flag = sharememory_manager.CreateShareMemory(hMapFileUser, pBufUser, UserFileName);
    if (!flag) {
        std::cerr << "无法创建共享内存，连接失败！" << std::endl;
        return;
    }

    // 通知 Simdisk，Shell 端已启动并建立连接
    pc_Controller->V(pc_Controller->getConnectSem());

    // 提示用户输入用户名并保存到共享内存
    cout << "请输入用户名: ";
    cin >> pBufUser->user_name;

    // 通知用户正在与 Simdisk 建立连接
    cout << "尝试与 Simdisk 端建立连接..." << endl;

    // 等待 Simdisk 端启动并准备好接收连接
    pc_Controller->P(pc_Controller->getGetSem());

    // 通知 Simdisk 端用户名已输入
    pc_Controller->V(pc_Controller->getUserSem());

    // 根据用户名生成共享内存文件名
    strcat_s(InputFileName, pBufUser->user_name);
    strcat_s(OuputFileName, pBufUser->user_name);
    strcat_s(CommFlagsFileName, pBufUser->user_name);

    // 等待 Simdisk 同步完成，确保共享内存安全关闭
    pc_Controller->P(pc_Controller->getReadySem());

    // 关闭用户信息共享内存
    sharememory_manager.CloseShareMemory(hMapFileUser, pBufUser);
}

/**
 * 启动 Shell，轮询与 Simdisk 的交互状态，发送或接收数据。
 */
void Run_Shell() {
    // 打开通信标志共享内存，用于 Shell 和 Simdisk 之间的状态交互
    bool flag = sharememory_manager.OpenShareMemory(hMapFileCommFlags, pCommFlags, CommFlagsFileName);
    if (!flag) {
        std::cerr << "无法打开通信标志共享内存，Shell 启动失败！" << std::endl;
        return;
    }

    // 持续轮询 Simdisk 的状态，检查是否需要发送或接收消息
    while (true) {
        if (pCommFlags->Shell_Send_Message()) {
            Send_Messages_To_ShareMemory(); // 发送消息到 Simdisk
        }
        else if (pCommFlags->Shell_Get_Message()) {
            Receive_Messages_From_ShareMemory(); // 接收 Simdisk 的消息
        }
        Sleep(10); // 稍作休眠以降低 CPU 占用
    }

    // 关闭通信标志共享内存
    sharememory_manager.CloseShareMemory(hMapFileCommFlags, pCommFlags);
}


int main() {
	//与simdisk建立连接
	ConnectToSimdisk();
	//等待Simdisk建立CommunicationFlags共享内存区
	pc_Controller->P(pc_Controller->getConnectSem());
	//运行
	Run_Shell();

	return 0;
}