#ifndef CONSTANT_H
#define CONSTANT_H

#include<string>
#include<fstream>
#include<ostream>
#include<functional>

using namespace std;

// ���inode��س���
#define BLOCKBITMAP_SIZE 3200                // ��λͼ�Ĵ�С
#define BLOCKBITMAP_PADDING 896              // ��λͼ����䣨δʹ�õĿռ䣩
#define INODEBITMAP_SIZE 1024               // inodeλͼ�Ĵ�С
#define BLOCK_SIZE 1024                     // ÿ�����ݿ�Ĵ�С
#define INODE_SIZE 128                      // ÿ��inode�Ĵ�С
#define MAX_BLOCK_POINTER 16                // ÿ��inode������ָ������

// �ļ�ϵͳ��س���
#define MaxUserId = 100000;                 // ����û�����
#define BOOTBLOCK_NUM 1                    // �����������
#define SUPERBLOCK_NUM 1                   // �����������
#define TOTAL_BLOCK_NUM 102400             // �ļ�ϵͳ���ܿ���
#define TOTAL_INODE_NUM 32768              // �ļ�ϵͳ����inode��

// ���ⳣ��ֵ
const unsigned int Nothing = 200000;     // ���ڱ�ʾ���ޡ��ĳ���ֵ
const unsigned int AdminID = 0;          // ����Ա�û�ID

// �ļ�ϵͳ��س���
const string My_File_System_Name = "YDAI_FileSystem"; // �ļ�ϵͳ����
const unsigned int Root_Directory = 0;  // ��Ŀ¼��inode ID
const unsigned int DataBlockNum = 98281; // ���ݿ��������
const unsigned int NameLength = 32;      // �ļ���Ŀ¼���Ƶ���󳤶�
const unsigned int File_Num_Directory = 256; // ÿ��Ŀ¼���������ļ�����
const int MAX_MESSAGES = 20;             // ϵͳ�ܴ洢�������Ϣ����
const int MAX_MESSAGE_LENGTH = 350;      // ������Ϣ����󳤶�

const unsigned int dataBlock_begin = 4119; // ���ݿ����ʼ����

// ��ǰϵͳ״̬��ȫ�ֱ���������
extern unsigned int Current_Directory_iNode;  // ��ǰĿ¼��inode ID
extern char Current_User[50];       // ��ǰ�û����û���
extern unsigned int Current_User_Id; // ��ǰ�û���ID
extern unsigned int Current_Permission;    // ��ǰ�û���Ȩ��

// �ļ����������ص�����������
extern fstream file_IO;  // �ļ����������
extern ofstream fout;    // �����

// ����ԭ��
unsigned int Parse_Permission(string s); // ����Ȩ���ַ���
void Calculate_UserId();                 // �����û�ID
bool ExtractBytes(unsigned int data, std::string& bytes); // ��ȡ�ֽ�����
unsigned int Read_Data_To_Block();       // �����ݶ����
unsigned int GetDataBlockPosition(unsigned int block_index); // ��ȡ���ݿ��λ��

#endif // CONSTANT_H
