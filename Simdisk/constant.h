#ifndef CONSTANT_H
#define CONSTANT_H

#include<string>
#include<fstream>
#include<ostream>
#include<functional>

using namespace std;

// 块和inode相关常量
#define BLOCKBITMAP_SIZE 3200                // 块位图的大小
#define BLOCKBITMAP_PADDING 896              // 块位图的填充（未使用的空间）
#define INODEBITMAP_SIZE 1024               // inode位图的大小
#define BLOCK_SIZE 1024                     // 每个数据块的大小
#define INODE_SIZE 128                      // 每个inode的大小
#define MAX_BLOCK_POINTER 16                // 每个inode的最大块指针数量

// 文件系统相关常量
#define MaxUserId = 100000;                 // 最大用户数量
#define BOOTBLOCK_NUM 1                    // 启动块的数量
#define SUPERBLOCK_NUM 1                   // 超级块的数量
#define TOTAL_BLOCK_NUM 102400             // 文件系统的总块数
#define TOTAL_INODE_NUM 32768              // 文件系统的总inode数

// 特殊常量值
const unsigned int Nothing = 200000;     // 用于表示“无”的常量值
const unsigned int AdminID = 0;          // 管理员用户ID

// 文件系统相关常量
const string My_File_System_Name = "YDAI_FileSystem"; // 文件系统名称
const unsigned int Root_Directory = 0;  // 根目录的inode ID
const unsigned int DataBlockNum = 98281; // 数据块的总数量
const unsigned int NameLength = 32;      // 文件或目录名称的最大长度
const unsigned int File_Num_Directory = 256; // 每个目录最多包含的文件数量
const int MAX_MESSAGES = 20;             // 系统能存储的最大消息数量
const int MAX_MESSAGE_LENGTH = 350;      // 单个消息的最大长度

const unsigned int dataBlock_begin = 4119; // 数据块的起始索引

// 当前系统状态（全局变量声明）
extern unsigned int Current_Directory_iNode;  // 当前目录的inode ID
extern char Current_User[50];       // 当前用户的用户名
extern unsigned int Current_User_Id; // 当前用户的ID
extern unsigned int Current_Permission;    // 当前用户的权限

// 文件输入输出相关的流（声明）
extern fstream file_IO;  // 文件输入输出流
extern ofstream fout;    // 输出流

// 函数原型
unsigned int Parse_Permission(string s); // 解析权限字符串
void Calculate_UserId();                 // 计算用户ID
bool ExtractBytes(unsigned int data, std::string& bytes); // 提取字节数据
unsigned int Read_Data_To_Block();       // 将数据读入块
unsigned int GetDataBlockPosition(unsigned int block_index); // 获取数据块的位置

#endif // CONSTANT_H
