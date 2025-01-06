#ifndef vfs
#define vfs


#include"constant.h"
#include"Simdisk.h"
#include"interaction.h"


// 检查是否可以进行拷贝操作，如果资源不足，向 shell 端发送错误信息
bool send_Copy_Error_Information(unsigned int need_block, unsigned int need_inode);

// 解释和执行用户输入的命令
bool Command_Interpreter();

// 初始化文件系统
bool Initial_FileSystem();

// 改变当前工作目录，输入为绝对路径
void Change_Directory(string absolute_path);

// 输出指定文件的所有数据内容
void Output_File_Data(unsigned int currentInode);

// 将虚拟文件系统中的文件复制到主机路径
void Copy_File_From_VFS_To_Host(unsigned int file_inode, string path);

// 将主机文件复制到虚拟文件系统中
void copy_File_From_Host_To_Simdisk(string filename, string hostpath, unsigned int directory_inode);

// 计算文件拷贝所需的资源
bool Calculate_Required_Resources(unsigned int file_inode_index, unsigned int& need_block, unsigned int& need_inode);

// 在虚拟文件系统 (VFS) 内部进行文件拷贝
void Copy_within_VFS(const string& filename, unsigned int file_inode_index, unsigned int directory_inode_index);

// 显示文件系统的基本信息，包括磁盘使用情况和结构信息
void display_Simdisk_Information();

// 显示指定目录下所有子文件和子目录的信息
void DirectoryInformation(iNode* dirInode);



// 显示指定目录的树形结构，不带缩进
// 子目录不显示层级关系
void DisplayDirectoryTree(iNode* dirInode, string s);

// 显示当前目录的绝对路径
void Display_CurrentDirectoryPath();

// 显示文件系统的基本信息
void Info();

// 改变当前工作目录 (cd 命令)
void Cd();

// 显示目录内容（支持当前目录和指定目录，可选显示子目录）
void Dir();

// 创建新目录（支持当前目录或指定路径下创建）
void Md();

// 删除目录 (rd path)
void Rd();

// 创建新文件 (newfile filename [path] <0/1/2>)
void Newfile();

// 显示文件内容 (cat path)
void Cat();

// 实现文件拷贝功能 (支持主机与模拟文件系统之间的拷贝，以及文件系统内部拷贝)
void Copy();

// 删除指定路径的文件 (支持普通文件删除，目录需使用 rd 命令)
void Del();

// 检查文件系统的一致性并修复潜在的问题
void Check();

// 列出文件系统中的所有目录和文件信息
void Ls();

// 主运行函数：模拟文件系统的核心控制逻辑
void RunSimdisk();
#endif // !vfs
