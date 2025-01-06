#include "interaction.h"


// �ļ������壺���ڱ�ʶ�����ڴ������
char UserFileName[] = "USER";                  // �û���Ϣ�����ڴ�����
char InputFileName[50] = "YDAI_INPUT";         // ���빲���ڴ�����
char OuputFileName[50] = "YDAI_OUTPUT";        // ��������ڴ�����
char CommFlagsFileName[50] = "YDAI_COMMFLAGS"; // ͨ�ű�־�����ڴ�����

// ȫ�ֱ�������

// ��Ϣ���
Message shell_to_simdisk; // �洢�� Shell ���� Simdisk �˷��͵���Ϣ

// �û���Ϣ���
HANDLE hMapFileUser;       // �û���Ϣ�����ڴ�ľ��
User user;                 // ��ǰ�û�����
User* pBufUser = &user;    // ָ���û������ָ�룬���ڲ��������ڴ��е��û���Ϣ

// ͨ�ű�־���
HANDLE hMapFileCommFlags;          // ͨ�ű�־�����ڴ�ľ��
CommunicationFlags* pCommFlags = nullptr; // ָ��ͨ�ű�־�����ڴ��ָ��

// ���빲���ڴ����
HANDLE hMapFileInput;             // ���빲���ڴ�ľ��
Message Input_Message;            // Shell �˴洢������Ϣ�Ķ���
Message* pInput_Message = &Input_Message; // ָ��������Ϣ�����ָ�룬���ڲ��������ڴ�

// ��������ڴ����
HANDLE hMapFileOutput;             // ��������ڴ�ľ��
Message* pOutput_Message = nullptr; // ָ�������Ϣ�����ڴ��ָ��

// �����ڴ������
ShareMemoryManager sharememory_manager; // ���ڹ������ڴ�Ĵ�����ӳ��͹ر�

// ���������������ź���������
Producer_And_Consumer* pc_Controller = new Producer_And_Consumer(); // ���ڿ��ƹ����ڴ��������-������ģ��


// �� Shell �����е����ݷ��͵� Simdisk ����
void Send_Messages_To_ShareMemory() {
	// ���������ڴ棬���ڴ洢��������
	bool flag = sharememory_manager.CreateShareMemory(hMapFileInput, pInput_Message, InputFileName);
	if (!flag) {
		cout << "�޷��򿪹����ڴ�" << endl;
		return; // �������ʧ�ܣ��˳�����
	}

	// �ȴ������ڴ�Ϊ�գ�ȷ������д������
	pc_Controller->P(pc_Controller->getEmptySem());
	pc_Controller->LockMutex(); // ������ȷ��д�������ԭ����

	// ��չ����ڴ��е���Ϣ������
	pInput_Message->clear_Message();

	// �ӱ�׼�����ȡ���ݣ�������빲���ڴ�
	while (cin >> pInput_Message->message[pInput_Message->message_num]) {
		pInput_Message->message_num++; // ������Ϣ��������
		if (cin.get() == '\n') break; // ��⵽���з���������
	}

	pc_Controller->UnlockMutex(); // ����
	pc_Controller->V(pc_Controller->getFullSem()); // ֪ͨ�����ڴ����������Զ�ȡ

	// �ȴ� Simdisk ���̴������
	pc_Controller->P(pc_Controller->getReadySem());

	// �رչ����ڴ棬�ͷ���Դ
	sharememory_manager.CloseShareMemory(hMapFileInput, pInput_Message);
	return;
}

// �� Simdisk ���̽������ݲ��� Shell �����
void Receive_Messages_From_ShareMemory() {
	// �ȴ������ڴ������ݿɶ�
	pc_Controller->P(pc_Controller->getFullSem());
	pc_Controller->LockMutex(); // ������ȷ����ȡ������ԭ����

	// �򿪹����ڴ棬���ڶ�ȡ�������
	bool flag = sharememory_manager.OpenShareMemory(hMapFileOutput, pOutput_Message, OuputFileName);
	if (!flag) {
		cout << "�޷��򿪹����ڴ�" << endl;
		pc_Controller->UnlockMutex(); // ����
		return; // �����ʧ�ܣ��˳�����
	}

	// �����Ϣ������ȷ�����ݲ������
	shell_to_simdisk.clear_Message();

	// �ӹ����ڴ��ж�ȡ���ݵ���Ϣ����
	shell_to_simdisk.copy_Data(pOutput_Message);

	// �������������׼���
	shell_to_simdisk.output_Data();

	pc_Controller->UnlockMutex(); // ����
	pc_Controller->V(pc_Controller->getEmptySem()); // ֪ͨ�����ڴ��ѿգ�����д��

	// ֪ͨ Simdisk ���̣��������
	pc_Controller->V(pc_Controller->getReadySem());

	// �رչ����ڴ棬�ͷ���Դ
	sharememory_manager.CloseShareMemory(hMapFileOutput, pOutput_Message);
	return;
}

