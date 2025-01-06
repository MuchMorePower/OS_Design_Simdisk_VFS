#include "interaction.h"


// ������ַ�����
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

// �ӹ����ڴ��ж�ȡ���ݵ� shell_to_simdisk ������
void Input_Data_From_ShareMemory() {
	// ֪ͨ shell �� simdisk ��Ҫ��������
	pCommFlags->Simdisk_Request_Shell_Input();

	// �ȴ������ڴ��������ݿɶ�����Ȼ����������
	pc_Controller->P(pc_Controller->getFullSem());
	pc_Controller->LockMutex();

	// �����������ڴ��Զ�ȡ��������
	bool flag = sharememory_manager.OpenShareMemory(hMapFileInput, pInput_Message, InputFileName);
	if (!flag) {  // �����ʧ�ܣ���ӡ������Ϣ
		cout << "�޷��򿪹����ڴ�" << endl;
	}

	// �������ڴ��е����ݸ��Ƶ� shell_to_simdisk ����
	shell_to_simdisk.copy_Data(pInput_Message);

	// �������ʲ��ͷſ��ź�����֪ͨ����д���µ�����
	pc_Controller->UnlockMutex();
	pc_Controller->V(pc_Controller->getEmptySem());

	// ֪ͨ shell �����Ѷ�ȡ��ɣ�ֹͣ��������
	pc_Controller->V(pc_Controller->getReadySem());
	pCommFlags->Pause_Shell();

	// �رչ����ڴ�
	sharememory_manager.CloseShareMemory(hMapFileInput, pInput_Message);
	return;
}

// �� simdisk_to_shell �����е�����д�뵽�����ڴ��У����ݸ� shell
void Output_Data_Into_ShareMemory() {
	// ֪ͨ shell �� simdisk ��Ҫ�������
	pCommFlags->Simdisk_Request_Shell_Output();

	// ���������ڴ��ļ������������
	bool flag = sharememory_manager.CreateShareMemory(hMapFileOutput, pOutput_Message, OuputFileName);
	if (!flag) {  // �������ʧ�ܣ���ӡ������Ϣ
		cout << "�޷��򿪹����ڴ�" << endl;
		return;
	}

	// �ȴ������ڴ�Ϊ�գ���д�룩��Ȼ����������
	pc_Controller->P(pc_Controller->getEmptySem());
	pc_Controller->LockMutex();

	// ��չ����ڴ�Ļ�����
	pOutput_Message->clear_Message();
	// �� simdisk_to_shell ��������ݸ��Ƶ������ڴ�
	pOutput_Message->copy_Data(&simdisk_to_shell);
	// ��������ڴ��е����ݣ�������;��
	pOutput_Message->output_Data();

	// �������ʲ��ͷ����ź�����֪ͨ���Զ�ȡ�µ�����
	pc_Controller->UnlockMutex();
	pc_Controller->V(pc_Controller->getFullSem());

	// �ȴ� shell �˶�ȡ��ɺ���� simdisk_to_shell ����
	pc_Controller->P(pc_Controller->getReadySem());
	simdisk_to_shell.clear_Message(); // ��ն�������
	pCommFlags->Pause_Shell();        // ֪ͨ shell ��ͣ��������

	// �رչ����ڴ�
	sharememory_manager.CloseShareMemory(hMapFileOutput, pOutput_Message);

	return;
}

// �� shell �˽�������
void ConnectToShell() {
    bool flag = false;

    // ��ʾ�ȴ� shell �˴��������ļ�����������
    cout << "�ȴ� shell �˴��������ļ� , ������ shell �˽�������......" << endl;

    // �ȴ� shell �����У���������
    pc_Controller->P(pc_Controller->getConnectSem()); // �ȴ� shell ���ź�������ʾ shell ����
    pc_Controller->V(pc_Controller->getGetSem());    // ֪ͨ shell simdisk ��׼����

    // ���û������ڴ�
    flag = sharememory_manager.OpenShareMemory(hMapFileUser, pBufUser, UserFileName);
    if (!flag) {
        cout << "�޷����û������ڴ�" << endl;
        return;
    }

    // �ȴ� shell �������û���
    pc_Controller->P(pc_Controller->getUserSem());

    // �������ַ����飬���ڴ洢ת������û���
    wchar_t wNameIn[50] = L"YDAI_INPUT";
    wchar_t wNameOut[50] = L"YDAI_OUTPUT";
    wchar_t wNameIoo[50] = L"YDAI_COMMFLAGS";

    // ���û����� char* ת��Ϊ wchar_t*
    MultiByteToWideChar(CP_ACP, 0, pBufUser->user_name, -1, wNameIn + wcslen(wNameIn), 50 - wcslen(wNameIn));
    MultiByteToWideChar(CP_ACP, 0, pBufUser->user_name, -1, wNameOut + wcslen(wNameOut), 50 - wcslen(wNameOut));
    MultiByteToWideChar(CP_ACP, 0, pBufUser->user_name, -1, wNameIoo + wcslen(wNameIoo), 50 - wcslen(wNameIoo));

    // ��ת����Ŀ��ַ����鸳ֵ�������ڴ��ļ�������
    wcscpy_s(InputFileName, wNameIn);
    wcscpy_s(OuputFileName, wNameOut);
    wcscpy_s(CommFlagsFileName, wNameIoo);

    // ���û������Ƶ���ǰ�û�����
    strcpy_s(Current_User, sizeof(Current_User), pBufUser->user_name);

    // �ȴ� 0.05 �룬�������رչ����ڴ�
    Sleep(50);

    // ֪ͨ shell simdisk ׼�����
    pc_Controller->V(pc_Controller->getReadySem());

    // �ر��û������ڴ�
    sharememory_manager.CloseShareMemory(hMapFileUser, pBufUser);

    // �ȴ�һС��ʱ���Է�ֹ�������
    Sleep(10);

    return;
}

// ����һ����Ϣ�� shell ��
void Deliver_Message_To_Shell(string message_to_shell) {
	// ������Ϣ����Ϊ 1
	simdisk_to_shell.message_num = 1;

	// ����Ϣ���ݸ��Ƶ� simdisk_to_shell ����
	strcpy_s(simdisk_to_shell.message[0], MAX_MESSAGE_LENGTH, message_to_shell.c_str());

	// �������������������д�빲���ڴ沢���͵� shell
	Output_Data_Into_ShareMemory();
}

// ���Ͷ�����Ϣ�� shell ��
void send_Messages(string* mes, int num) {
	// ������Ϣ����
	simdisk_to_shell.message_num = num;

	// ��ÿ����Ϣ���ݸ��Ƶ� simdisk_to_shell ����
	for (int i = 0; i < num; i++) {
		strcpy_s(simdisk_to_shell.message[i], MAX_MESSAGE_LENGTH, mes[i].c_str());
	}

	// �������������������д�빲���ڴ沢���͵� shell
	Output_Data_Into_ShareMemory();
}

// �����û����������
void Parse_Command(Command& input_command) {
    // Step 1: ����������ȷ��û�������ľ�����
    input_command.clear_command();

    // Step 2: �ӹ����ڴ��ж�ȡ���ݲ��ֽ������ַ���
    Input_Data_From_ShareMemory(); // �ӹ����ڴ��л�ȡ shell ����������

    // ����ȡ�������������� Command ����
    input_command.cmd_count = shell_to_simdisk.message_num; // ��¼��������
    for (int i = 0; i < input_command.cmd_count; i++) {
        input_command.cmd[i] = shell_to_simdisk.message[i]; // �����洢����
    }

    // Step 3: ���������ַ���������������
    input_command.Determine_Command_Type();
}

// չʾ�����˵����� Shell �˷���������ʾ��Ϣ
void Display_HelpMenu() {
    // ����������ʾ��Ϣ
    string message_to_shell[20];
    message_to_shell[0] = "--------------------------------------------------------------------------------\n";
    message_to_shell[1] = "YDFS                                                                   Help Menu\n";
    send_Messages(message_to_shell, 2);
    message_to_shell[0] = "--------------------------------------------------------------------------------\n\n";
    message_to_shell[1] = "    info                                             չʾ�ļ�ϵͳ�Ļ�����Ϣ\n";
    message_to_shell[2] = "    cd {path}                                        �ı乤��Ŀ¼\n";
    message_to_shell[3] = "    dir [path] [s]                                   չʾĿ¼\n";
    message_to_shell[4] = "    md {dirname} [path] {0/1/2}                      �½�Ŀ¼\n";
    message_to_shell[5] = "    rd {path}                                        ɾ��Ŀ¼\n";
    message_to_shell[6] = "    newfile {filename} [path] {0/1/2}                �½��ļ�\n";
    message_to_shell[7] = "    cat {path}                                       ���ļ�\n";
    message_to_shell[8] = "    copy<host> {HostPath} {VFSPath} {0/1} {0/1/2}    ���������ļ�\n";
    message_to_shell[9] = "    copy<ydfs> {SrcPath} {DestPath} {0/1/2}          �����ļ�ϵͳ�ļ�\n";
    message_to_shell[10] = "    del {path}                                       ɾ���ļ�\n";
    message_to_shell[11] = "    check                                            ��鲢�޸��ļ�һ����\n";
    message_to_shell[12] = "    help                                             չʾ����Ŀ¼\n";
    message_to_shell[13] = "    exit                                             �˳��ļ�ϵͳ\n";
    message_to_shell[14] = "    ls                                               չʾ�ļ�ϵͳ�µ�Ŀ¼���ļ���Ϣ\n";
    message_to_shell[15] = "\n";
    message_to_shell[16] = "    []�ڲ�Ϊ��ѡ����,�������ʾʹ�����·��\n";
    message_to_shell[17] = "    {}�ڲ�Ϊ��������\n";
    message_to_shell[18] = "    {0/1/2}��ʾȨ�޿���, 0���������û��޶�дȨ��, 1���������û��ж�Ȩ�ޣ���дȨ��, 2���������û��ж�дȨ��\n";
    message_to_shell[19] = "    copy<host>������, {0/1}��, 0��ʾ������������VFS��, 1��ʾ��VFS������������\n";

    // ���Ͱ�����Ϣ�� Shell
    send_Messages(message_to_shell, 20);

    // ������ʾ·��֧��
    message_to_shell[0] = "    YDFS֧�־���·�������·��\n";
    message_to_shell[1] = "    ����·�� : /aaa/bbb/ccc\n";
    message_to_shell[2] = "    ���·�� : ./bbb/ccc �� ccc\n\n";
    send_Messages(message_to_shell, 3);

    return;
}

// ���·���Ƿ����
bool isPath_Exist(unsigned int path_index) {
    // ���·�������ڣ��� Shell �˷��ʹ�����Ϣ
    if (path_index == Nothing) {
        Deliver_Message_To_Shell("��·��������!\n");
        return false; // ����·�������ڵĽ��
    }
    return true; // ����·�����ڵĽ��
}




