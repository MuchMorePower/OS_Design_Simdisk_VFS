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
// ������ַ�����
extern wchar_t InputFileName[50];        // ����
extern wchar_t OuputFileName[50];       // ����
extern wchar_t CommFlagsFileName[50];       // ����
extern const wchar_t* UserFileName;   // ����

// Command �������û����������ֽ⣬�����ж���������
class Command {
public:
	unsigned int cmd_count;  // �����еĵ�������
	string cmd[20];          // �洢�ֽ��������
	unsigned int type;       // �������ͣ��������ֲ�ͬ����

	// ���캯������ʼ������
	Command() {
		clear_command();  // ��ʼ��ʱ�������
	}

	// �����������
	void clear_command() {
		cmd_count = 0;  // �����������
		for (int i = 0; i < 20; i++)
			cmd[i] = "";  // ������������е�ÿ��Ԫ��
		type = Nothing;  // ������������Ϊδ����
	}

	// ����������ַ��������ж���������
	void Determine_Command_Type() {
		// �������������͵�ӳ���ϵ
		static std::unordered_map<std::string, int> command_map = {
			{"exit", 0}, {"EXIT", 0}, {"Exit", 0},  // �˳�����
			{"info", 1},                         // ��ʾ��Ϣ
			{"cd", 2},                           // �л�Ŀ¼
			{"dir", 3},                          // �г�Ŀ¼����
			{"md", 4},                           // ����Ŀ¼
			{"rd", 5},                           // ɾ��Ŀ¼
			{"newfile", 6},                      // �������ļ�
			{"cat", 7},                          // �鿴�ļ�����
			{"copy<host>", 8}, {"copy<ydfs>", 8}, // �ļ����ƣ����ػ�Զ�̣�
			{"del", 9},                          // ɾ���ļ�
			{"check", 10},                       // ����ļ�ϵͳ
			{"ls", 11},                          // ��ʾĿ¼���ݣ���棩
			{"help",12}                          // ��ʾ������Ϣ
		};

		// ��ӳ����в�����������
		auto it = command_map.find(this->cmd[0]);  // ��������ĵ�һ�����֣�cmd[0]��
		if (it != command_map.end()) {
			this->type = it->second;  // ����ҵ���ƥ������������������
		}
		else {
			this->type = Nothing;  // ���δ�ҵ�ƥ����������������Ϊ Nothing
		}
	}
};

// User �����ڴ洢�û������� shell �˽�������ʱʹ��
class User {
public:
	char user_name[50];  // �û�������󳤶�Ϊ 50

	// ���캯������ʼ���û���Ϊ��
	User() {
		for (int i = 0; i < sizeof(user_name); i++) {
			user_name[i] = '\0';  // ��ʼ���û���Ϊ���ַ�
		}
	}

	// �����û���
	void set_userName(char* name) {
		// ��������û����ַ������Ƶ� user_name ������
		for (int i = 0; i < 50; i++) {
			this->user_name[i] = name[i];
		}
	}
};

// Reader_And_Writer ������ʵ�ֶ�д�������е��ź�������
// ���ƶ��ߺ�д�ߵķ���Ȩ�ޣ���֤�����ͬ��
class Reader_And_Writer {
public:
	HANDLE hSemaphore;  // ���ƶ�д�����ź�������֤д��������
	HANDLE hReaderEvent;  // ���ƶ����¼�������Э��������ߵķ���
	int readerCount;      // ��¼��ǰ��Ծ���ߵ�����

	// ���캯������ʼ���ź������¼���׼��Э����д����
	Reader_And_Writer() {
		readerCount = 0;  // ��ʼ�����߼���Ϊ 0

		// ����һ���ź�������ʼֵΪ 1����ʾ����һ��д�߷�����Դ
		hSemaphore = CreateSemaphore(NULL, 1, 1, L"Global\\RW_Semaphore");
		if (hSemaphore == NULL) {
			std::cerr << "CreateSemaphore error: " << GetLastError() << std::endl;
		}

		// ����һ���ֶ������¼�����ʼ״̬Ϊ���źţ����ã�
		hReaderEvent = CreateEvent(NULL, TRUE, TRUE, L"Global\\ReaderEvent");
		if (hReaderEvent == NULL) {
			std::cerr << "CreateEvent error: " << GetLastError() << std::endl;
		}
	}

	// �����������ͷ��ź������¼���Դ
	~Reader_And_Writer() {
		CloseHandle(hSemaphore);
		CloseHandle(hReaderEvent);
	}

	// Set_Writer_XLock���൱�� P ��������ͼ��ȡдȨ��
	void Set_Writer_XLock() {
		WaitForSingleObject(hSemaphore, INFINITE);  // ��ȡдȨ�ޣ�����ֱ������д
	}

	// Release_Writer_XLock���൱�� V �������ͷ�дȨ��
	void Release_Writer_XLock() {
		ReleaseSemaphore(hSemaphore, 1, NULL);  // �ͷ��ź�������������д����
	}

	// Set_Reader_SLock���൱�� P ���������ƶ��ߵĽ���
	void Set_Reader_SLock() {
		WaitForSingleObject(hReaderEvent, INFINITE);  // �ȴ��ɶ��¼��źţ����Ʋ�������

		if (readerCount == 0) {
			// ����ǵ�һ�����ߵ������ֹд����
			WaitForSingleObject(hSemaphore, INFINITE);  // �ȴ�д�ߵķ���Ȩ��
		}
		readerCount++;  // ���߼������� 1
		SetEvent(hReaderEvent);  // �����������߼�������
	}

	// Release_Reader_SLock���൱�� V ���������ƶ��ߵ��뿪
	void Release_Reader_SLock() {
		WaitForSingleObject(hReaderEvent, INFINITE);  // �ȴ��ɶ��¼��ź�

		readerCount--;  // ���߼������� 1
		if (readerCount == 0) {
			// ��������һ�������뿪������д����
			ReleaseSemaphore(hSemaphore, 1, NULL);
		}
		SetEvent(hReaderEvent);  // �����¼��źţ������������߼�������
	}
};

// Message �������� shell �������ݣ��洢�Ͳ�����Ϣ����
class Message {
public:
	char message[MAX_MESSAGES][MAX_MESSAGE_LENGTH]; // ʹ�ù̶���С���ַ�����洢��Ϣ
	int message_num;  // ��ǰ��Ϣ������

	// ���캯������ʼ����Ϣ����
	Message() {
		clear_Message();  // �����Ϣ
	}

	// �����Ϣ����
	void clear_Message() {
		message_num = 0;  // ������Ϣ����Ϊ 0
		memset(message, 0, sizeof(message));  // �����Ϣ���ݣ�ÿ���ַ�����Ϊ '\0'��
	}

	// ������һ�� Message �������Ϣ����
	void copy_Data(Message* pObject) {
		this->message_num = pObject->message_num;  // ������Ϣ����
		// ����ַ�������Ϣ����
		for (int i = 0; i < MAX_MESSAGES; i++) {
			for (int j = 0; j < MAX_MESSAGE_LENGTH; j++) {
				this->message[i][j] = pObject->message[i][j];
			}
		}
	}

	// �����Ϣ���ݵ�����̨
	void output_Data() {
		for (int i = 0; i < this->message_num; i++) {
			cout << this->message[i];  // ���ÿ����Ϣ
		}
	}
};

// CommunicationFlags �����ڹ��� simdisk �� shell ֮���ͨ�ű�־
class CommunicationFlags {
public:
	int request_to_shell; // ״̬��ʶλ����ʾ simdisk �˸��� shell �˵�ǰ������

	// ���캯������ʼ����־λΪ 0��Ĭ�ϲ����κβ�����
	CommunicationFlags() {
		request_to_shell = 0;
	}

	// Simdisk ���� shell ��������
	void Simdisk_Request_Shell_Input() {
		request_to_shell = 1;  // ���ñ�־λΪ 1����ʾ���� shell ��������
	}

	// Simdisk ���� shell ��������ڴ��е�����
	void Simdisk_Request_Shell_Output() {
		request_to_shell = 2;  // ���ñ�־λΪ 2����ʾ���� shell �������
	}

	// ֹͣ shell �˲������������Ѷ�����ɣ������Ȳ�����Ҳ�������
	void Pause_Shell() {
		request_to_shell = 0;  // ���ñ�־λΪ 0����ʾ��ͣ shell �˵���������
	}
};

// ShareMemoryManager �����ڹ������ڴ�Ĵ������򿪺͹رղ���
class ShareMemoryManager {
public:
	// ������������ӡ������Ϣ
	void HandleError(const char* msg, const wchar_t* name) {
		int error = GetLastError();  // ��ȡ�������
		_tprintf(TEXT("Error: %s for %s (Error Code: %d).\n"), msg, name, error);
	}

	// ���������ڴ�
	template <typename T>
	bool CreateShareMemory(HANDLE& hMapFile, T*& pMemory, const wchar_t* name) {
		// �����ļ�ӳ����󣨹����ڴ棩
		hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, BUFFER_SIZE, name);
		if (hMapFile == NULL) {  // �������ʧ�ܣ���ӡ���󲢷��� false
			HandleError("Failed to create file mapping object", name);
			return false;
		}

		// ӳ���ļ�����ͼ�����ָ�����ڴ��ָ��
		pMemory = (T*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
		if (pMemory == NULL) {  // ���ӳ��ʧ�ܣ���ӡ���󲢹ر��ļ�ӳ�����
			HandleError("Failed to map view of file", name);
			CloseHandle(hMapFile);
			return false;
		}

		return true;  // �ɹ����������ڴ沢ӳ��
	}

	// ���Ѵ��ڵĹ����ڴ�
	template <typename T>
	bool OpenShareMemory(HANDLE& hMapFile, T*& pMemory, const wchar_t* name) {
		// �����е��ļ�ӳ����󣨹����ڴ棩
		hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, name);
		if (hMapFile == NULL) {  // �����ʧ�ܣ���ӡ���󲢷��� false
			HandleError("Failed to open file mapping object", name);
			return false;
		}

		// ӳ���ļ�����ͼ�����ָ�����ڴ��ָ��
		pMemory = (T*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUFFER_SIZE);
		if (pMemory == NULL) {  // ���ӳ��ʧ�ܣ���ӡ���󲢹ر��ļ�ӳ�����
			HandleError("Failed to map view of file", name);
			CloseHandle(hMapFile);
			return false;
		}

		return true;  // �ɹ��򿪲�ӳ�乲���ڴ�
	}

	// �رչ����ڴ�
	template <typename T>
	void CloseShareMemory(HANDLE& hMapFile, T*& pMemory) {
		// ��������ڴ�ָ�벻Ϊ�գ����ӳ��
		if (pMemory) {
			UnmapViewOfFile(pMemory);
		}

		// ����ļ�ӳ�����Ϊ�գ��رվ��
		if (hMapFile) {
			CloseHandle(hMapFile);
		}

		Sleep(10);  // Ϊ�˷�ֹƵ�����ã��ӳ� 10 ����
	}
};


// Producer_And_Consumer ��ʵ����������-������ģ�͵�ͬ�����ƣ����ڹ����ź����ͻ�����
class Producer_And_Consumer {
private:
	HANDLE sem_empty;   // �ź�������ʾ�����ڴ�Ϊ�գ�����д��
	HANDLE sem_full;    // �ź�������ʾ�����ڴ����������Զ�ȡ
	HANDLE sem_ready;   // �ź����������ж��Ƿ�رչ����ڴ�
	HANDLE sem_connect; // �ź������ȴ��� shell �˽�������
	HANDLE sem_user;    // �ź������ȴ� simdisk �˻�ȡ�û�������û���
	HANDLE sem_get;     // �ź������ȴ� shell �˴��������ڴ�
	HANDLE hMutex;      // ��������ȷ���Թ����ڴ�Ļ������

public:
	// ���캯�������������ź����ͻ�����
	Producer_And_Consumer() {
		// ʹ�������ź�����������̿���ͨ�����Ʒ���ͬһ���ź���
		sem_empty = CreateSemaphore(NULL, 1, 1, L"GlobalSemaphoreEmpty"); // �����ڴ�Ϊ�գ���ʼ��Ϊ 1
		sem_full = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreFull");  // �����ڴ���������ʼ��Ϊ 0
		sem_ready = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreReady"); // ���ڹرչ����ڴ棬��ʼ��Ϊ 0
		sem_connect = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreConnect"); // �ȴ��� shell �������ӣ���ʼ��Ϊ 0
		sem_user = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreUser"); // �ȴ� simdisk ��ȡ�û�������ʼ��Ϊ 0
		sem_get = CreateSemaphore(NULL, 0, 1, L"GlobalSemaphoreGet"); // �ȴ� shell ���������ڴ棬��ʼ��Ϊ 0
		hMutex = CreateMutex(NULL, FALSE, L"GlobalShareMemoryMutex"); // ������������ȷ���Թ����ڴ�Ļ������

		// ����ź����ͻ������Ƿ񴴽��ɹ�
		if (!sem_empty || !sem_full || !hMutex || !sem_connect || !sem_ready) {
			std::cerr << "�޷������ź����򻥳���" << std::endl; // �������ʧ�ܣ���ӡ�����˳�����
			exit(1);
		}
	}

	// �����������ر��ź����ͻ�����
	~Producer_And_Consumer() {
		CloseHandle(sem_empty);   // �ر��ź��� sem_empty
		CloseHandle(sem_full);    // �ر��ź��� sem_full
		CloseHandle(sem_ready);   // �ر��ź��� sem_ready
		CloseHandle(sem_connect); // �ر��ź��� sem_connect
		CloseHandle(sem_user);    // �ر��ź��� sem_user
		CloseHandle(sem_get);     // �ر��ź��� sem_get
		CloseHandle(hMutex);      // �رջ�����
	}

	// P ������Wait���������ź�����������ǰ�߳�ֱ���ź�������
	void P(HANDLE sem) {
		WaitForSingleObject(sem, INFINITE); // �ȴ��ź�������������������
	}

	// V ������Signal�����ͷ��ź��������ѵȴ����߳�
	void V(HANDLE sem) {
		ReleaseSemaphore(sem, 1, NULL); // �ͷ��ź���������һ���ȴ��߳�
	}

	// LockMutex�����󻥳�����ȷ���Թ����ڴ�Ķ�ռ����
	void LockMutex() {
		WaitForSingleObject(hMutex, INFINITE); // �ȴ�����������������������
	}

	// UnlockMutex���ͷŻ����������������̷߳��ʹ����ڴ�
	void UnlockMutex() {
		ReleaseMutex(hMutex); // �ͷŻ�����
	}

	// ��ȡ�����ź���
	HANDLE getEmptySem() { return sem_empty; }
	HANDLE getFullSem() { return sem_full; }
	HANDLE getReadySem() { return sem_ready; }
	HANDLE getConnectSem() { return sem_connect; }
	HANDLE getUserSem() { return sem_user; }
	HANDLE getGetSem() { return sem_get; }
};

// �����ǹ����ڴ漰������ݵ�����

// ���� simdisk �� shell ֮������ݽ���
extern Message shell_to_simdisk;    // shell ���͵� simdisk ����Ϣ
extern Message simdisk_to_shell;    // simdisk ���͵� shell ����Ϣ

// ��ǰ����������
extern Command command;             // ���ڴ洢 shell �����������

// �û�����Ĺ����ڴ�
extern HANDLE hMapFileUser;         // �û������ڴ��ļ����
extern User* pBufUser;              // ָ���û������ڴ��ָ��

// ���ƶ�д��ͬ��
extern Reader_And_Writer rw_Controller; // ��д��������

// ͨ�ű�־
extern HANDLE hMapFileCommFlags;     // ͨ�ű�־�����ڴ��ļ����
extern CommunicationFlags commFlags; // ͨ�ű�־ʵ��
extern CommunicationFlags* pCommFlags; // ͨ�ű�־ָ��

// ������Ϣ�����ڴ�
extern HANDLE hMapFileInput;          // ������Ϣ�����ڴ��ļ����
extern Message* pInput_Message;       // ָ��������Ϣ�����ڴ��ָ��

// �����Ϣ�����ڴ�
extern HANDLE hMapFileOutput;         // �����Ϣ�����ڴ��ļ����
extern Message Output_Message;        // �����Ϣ
extern Message* pOutput_Message;      // ָ�������Ϣ�����ڴ��ָ��

// �����ڴ����ʵ��
extern ShareMemoryManager sharememory_manager; // �����ڴ����ʵ��

// ������������ģ�Ϳ�����
extern Producer_And_Consumer* pc_Controller; // ������������ģ�Ϳ�����ʵ��


// �ӹ����ڴ��ж�ȡ���ݵ� shell_to_simdisk ������
void Input_Data_From_ShareMemory();

// �� simdisk_to_shell �����е�����д�뵽�����ڴ��У����ݸ� shell
void Output_Data_Into_ShareMemory();

// �� shell �˽�������
void ConnectToShell();

// ����һ����Ϣ�� shell ��
void Deliver_Message_To_Shell(string message_to_shell);

// ���Ͷ�����Ϣ�� shell ��
void send_Messages(string* mes, int num);

// �����û����������
void Parse_Command(Command& input_command);

// չʾ�����˵����� Shell �˷���������ʾ��Ϣ
void Display_HelpMenu();

// ���·���Ƿ����
bool isPath_Exist(unsigned int path_index);
#endif

