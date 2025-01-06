#ifndef vfs
#define vfs


#include"constant.h"
#include"Simdisk.h"
#include"interaction.h"


// ����Ƿ���Խ��п��������������Դ���㣬�� shell �˷��ʹ�����Ϣ
bool send_Copy_Error_Information(unsigned int need_block, unsigned int need_inode);

// ���ͺ�ִ���û����������
bool Command_Interpreter();

// ��ʼ���ļ�ϵͳ
bool Initial_FileSystem();

// �ı䵱ǰ����Ŀ¼������Ϊ����·��
void Change_Directory(string absolute_path);

// ���ָ���ļ���������������
void Output_File_Data(unsigned int currentInode);

// �������ļ�ϵͳ�е��ļ����Ƶ�����·��
void Copy_File_From_VFS_To_Host(unsigned int file_inode, string path);

// �������ļ����Ƶ������ļ�ϵͳ��
void copy_File_From_Host_To_Simdisk(string filename, string hostpath, unsigned int directory_inode);

// �����ļ������������Դ
bool Calculate_Required_Resources(unsigned int file_inode_index, unsigned int& need_block, unsigned int& need_inode);

// �������ļ�ϵͳ (VFS) �ڲ������ļ�����
void Copy_within_VFS(const string& filename, unsigned int file_inode_index, unsigned int directory_inode_index);

// ��ʾ�ļ�ϵͳ�Ļ�����Ϣ����������ʹ������ͽṹ��Ϣ
void display_Simdisk_Information();

// ��ʾָ��Ŀ¼���������ļ�����Ŀ¼����Ϣ
void DirectoryInformation(iNode* dirInode);



// ��ʾָ��Ŀ¼�����νṹ����������
// ��Ŀ¼����ʾ�㼶��ϵ
void DisplayDirectoryTree(iNode* dirInode, string s);

// ��ʾ��ǰĿ¼�ľ���·��
void Display_CurrentDirectoryPath();

// ��ʾ�ļ�ϵͳ�Ļ�����Ϣ
void Info();

// �ı䵱ǰ����Ŀ¼ (cd ����)
void Cd();

// ��ʾĿ¼���ݣ�֧�ֵ�ǰĿ¼��ָ��Ŀ¼����ѡ��ʾ��Ŀ¼��
void Dir();

// ������Ŀ¼��֧�ֵ�ǰĿ¼��ָ��·���´�����
void Md();

// ɾ��Ŀ¼ (rd path)
void Rd();

// �������ļ� (newfile filename [path] <0/1/2>)
void Newfile();

// ��ʾ�ļ����� (cat path)
void Cat();

// ʵ���ļ��������� (֧��������ģ���ļ�ϵͳ֮��Ŀ������Լ��ļ�ϵͳ�ڲ�����)
void Copy();

// ɾ��ָ��·�����ļ� (֧����ͨ�ļ�ɾ����Ŀ¼��ʹ�� rd ����)
void Del();

// ����ļ�ϵͳ��һ���Բ��޸�Ǳ�ڵ�����
void Check();

// �г��ļ�ϵͳ�е�����Ŀ¼���ļ���Ϣ
void Ls();

// �����к�����ģ���ļ�ϵͳ�ĺ��Ŀ����߼�
void RunSimdisk();
#endif // !vfs
