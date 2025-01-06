#include "constant.h"

// 当前状态的初始化
unsigned int Current_Directory_iNode = Root_Directory;  // 当前目录的 inode ID，默认为根目录
char Current_User[50];  // 当前用户名
unsigned int Current_User_Id; // 当前用户的ID
unsigned int Current_Permission; // 当前用户的权限模式

fstream file_IO;    // 文件输入输出流
ofstream fout;      // 文件输出流

// 解析权限字符串，将字符串转换为整数
unsigned int Parse_Permission(string s) {
    return std::stoi(s); // 将字符串转换为整数，通常用于解析文件或用户的权限
}

// 计算当前用户的ID
void Calculate_UserId() {
    Current_User_Id = 0;  // 默认用户ID为0（管理员）

    // 判断是否为管理员用户
    if (std::strcmp(Current_User, "admin") != 0) {
        // 将当前用户名转为 std::string 类型
        std::string user_str(Current_User);

        // 使用标准库的 hash 函数计算用户名的哈希值作为用户ID
        std::hash<std::string> hash_fn;
        Current_User_Id = hash_fn(user_str);
    }

    // 如果是管理员用户，Current_User_Id 会保持为 0
    return;
}

// 从一个 unsigned int 类型的数据提取字节并存储到字符串中
bool ExtractBytes(unsigned int data, std::string& bytes) {
    // 将 32 位整数分解为 4 个字节，并存入 bytes 字符串中
    bytes[0] = static_cast<char>((data >> 24) & 0xFF);  // 提取高位字节
    bytes[1] = static_cast<char>((data >> 16) & 0xFF);  // 提取次高位字节
    bytes[2] = static_cast<char>((data >> 8) & 0xFF);   // 提取次低位字节
    bytes[3] = static_cast<char>(data & 0xFF);          // 提取低位字节

    // 如果所有字节都为 '\0'，说明提取失败，返回 false
    return !(bytes[0] == '\0' && bytes[1] == '\0' && bytes[2] == '\0' && bytes[3] == '\0');
}

// 从文件读取 4 个字节，并将其转换为 unsigned int 类型的数据
unsigned int Read_Data_To_Block() {
    unsigned int data_to_copy = 0;  // 用于存储读取的数据
    char bytes[4];  // 存储读取的 4 个字节

    // 从文件流中读取 4 个字节到 bytes 数组中
    file_IO.read(bytes, sizeof(bytes));

    // 将字节数组转换为 unsigned int 类型的数据
    memcpy(&data_to_copy, bytes, sizeof(data_to_copy));

    // 返回读取的数据
    return data_to_copy;
}

// 获取数据块的位置，通过块索引计算位置
unsigned int GetDataBlockPosition(unsigned int block_index) {
    return dataBlock_begin + block_index;  // 计算并返回数据块的位置
}
