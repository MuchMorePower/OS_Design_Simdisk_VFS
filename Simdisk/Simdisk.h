#ifndef Simdisk
#define Simdisk
#include<vector>
#include<iostream>
#include<fstream>
#include<iomanip> // 为 std::setw 提供支持
#include<sstream>
#include<queue>
#include"constant.h"

using namespace std;

class BootBlock { // 引导块，占文件系统1块，总大小为1024字节
public:
	unsigned int iNode_array_begin;           // iNode表的起始块号
	unsigned int iNode_array_end;             // iNode表的结束块号
	unsigned int dataBlock_begin;             // 数据块的起始块号
	unsigned int dataBlock_end;               // 数据块的结束块号

	unsigned int padding[252];//用于占据模拟磁盘空间

	BootBlock() {
		  // 初始化 space 的大小为 256
		for (size_t i = 0; i < 252; ++i) {
			padding[i] = 0; //unsigned int占4B，4 * 256 = 1024字节
		}
		// iNode表的起始和结束块号 (存储文件的元数据)
		iNode_array_begin = 23;      // iNode表从第23块开始
		iNode_array_end = 4118;      // iNode表占用了4096个块，直到第4118块

		// 数据块的起始和结束块号 (用于存储文件的实际数据)
		dataBlock_begin = 4119;      // 数据块从第4119块开始
		dataBlock_end = 102399;      // 数据块一直到组的结束块，即第102399块
	}
	unsigned int BlockNum() {
		return dataBlock_end - dataBlock_begin + 1;
	}
	unsigned int iNodeArrayNum() {
		return iNode_array_end - iNode_array_begin + 1;
	}
};

class SuperBlock { // 超级块，占文件系统1块，总大小为1024字节，8192比特
public:
	// 文件系统的基本信息
	unsigned int total_block;     // 总磁盘块数
	unsigned int total_datablock; // 总数据块数目
	unsigned int used_datablock;  // 已使用的数据块数目（初始时，根目录占用一个数据块）
	unsigned int total_inode;     // 总 inode 数目
	unsigned int used_inode;      // 已使用的 inode 数目（初始时，根目录占用一个 inode）
	unsigned int block_size;      // 磁盘块大小，单位字节（1024B，即1KB）
	unsigned int inode_size;      // 每个 inode 的大小，单位字节（128B）

	// 文件系统中关于文件的具体配置
	unsigned int file_total_num;  // 文件系统中所有文件的总数目
	unsigned int file_num_directory;  // 每个目录下最多可包含的文件数量
	unsigned int name_length;        // 文件名的最大长度（单位：字符）

	// 用于填充，确保超级块大小为1024字节
	unsigned int padding[246]; // 用于占据模拟磁盘空间，总共 246 个 unsigned int，占 1024 字节

	// 构造函数，初始化超级块的各项参数
	SuperBlock() {
		total_block = TOTAL_BLOCK_NUM; // 设置总块数
		total_datablock = DataBlockNum; // 设置数据块数
		used_datablock = 1; // 根目录占用一个数据块
		total_inode = TOTAL_INODE_NUM; // 设置总 inode 数
		used_inode = 1; // 根目录占用一个 inode
		block_size = BLOCK_SIZE; // 设置块大小
		inode_size = INODE_SIZE; // 设置 inode 大小
		file_total_num = TOTAL_INODE_NUM; // 文件总数等于 inode 总数
		file_num_directory = File_Num_Directory; // 每个目录下文件的上限
		name_length = NameLength; // 文件名长度上限

		// 填充 padding 数组，确保超级块总大小为1024字节
		for (size_t i = 0; i < 246; ++i) {
			padding[i] = 0; // 将 padding 数组的所有元素初始化为0
			// unsigned int 占用 4 字节，246 * 4 = 984 字节，剩余 40 字节将由其他字段占用
		}
	}
};


class GroupDescription {
public:
	// 文件系统中的不同区域的起始和结束块号
	unsigned int group_begin;                 // 组的起始块号
	unsigned int group_end;                   // 组的结束块号
	unsigned int superBlock_begin;            // 超级块的起始块号
	unsigned int superBlock_end;              // 超级块的结束块号
	unsigned int groupDescription_begin;      // 组描述符区域的起始块号
	unsigned int groupDescription_end;        // 组描述符区域的结束块号
	unsigned int block_bit_map_begin;         // 块位图的起始块号
	unsigned int block_bit_map_end;           // 块位图的结束块号
	unsigned int iNode_bit_map_begin;         // iNode位图的起始块号
	unsigned int iNode_bit_map_end;           // iNode位图的结束块号

	unsigned int padding[246];               // 模拟一个向量，表示组中的空间占用情况

	GroupDescription() {
		// 初始化各个区域的起始和结束块号
		group_begin = 1;             // 第1块是组的开始块
		group_end = 102399;          // 组的结束块是第102399块（假设整个系统有102400个块，第0块是引导块）

		// 超级块的起始和结束块号 (超级块通常存储文件系统的总体信息)
		superBlock_begin = 1;        // 超级块从第1块开始
		superBlock_end = 1;          // 超级块仅占1块，因此结束也是第1块

		// 组描述符区域的起始和结束块号 (记录文件系统中各个组的信息)
		groupDescription_begin = 2;  // 组描述符从第2块开始
		groupDescription_end = 2;    // 组描述符只占用第2块

		// 块位图的起始和结束块号 (用于记录哪些数据块已被使用)
		block_bit_map_begin = 3;     // 块位图从第3块开始
		block_bit_map_end = 18;      // 块位图占用了16个块，直到第18块

		// iNode位图的起始和结束块号 (记录哪些iNode已被使用)
		iNode_bit_map_begin = 19;    // iNode位图从第19块开始
		iNode_bit_map_end = 22;      // iNode位图占用了4个块，直到第22块

		// 初始化 `padding` 向量的大小为242，并将所有元素设置为0
		for (int i = 0; i < 246; i++) {
			padding[i] = 0;            // 将向量的每个元素都设置为0
		}
	}

	// 计算组描述符的大小
	unsigned int groupDescriptionSize() {
		return groupDescription_end - groupDescription_begin + 1;
	}

	// 计算块位图的大小
	unsigned int blockBitMapSize() {
		return block_bit_map_end - block_bit_map_begin + 1;
	}

	// 计算 iNode 位图的大小
	unsigned int iNodeBitMapSize() {
		return iNode_bit_map_end - iNode_bit_map_begin + 1;
	}
};

class BlockBitMap { // 块位图（Block Bitmap）占16个磁盘块, 总大小为16KB（16384B)，用来跟踪数据块的使用情况
public:
	unsigned int is_used[BLOCKBITMAP_SIZE];  // 用于存储块位图的数组，每个元素对应 32 个数据块的使用情况
	unsigned int padding[BLOCKBITMAP_PADDING]; // 用于填充，确保块位图的大小符合要求

	// 构造函数，初始化块位图
	BlockBitMap() {
		// 初始化所有块位图为 0，表示所有块都是空闲的
		for (int i = 0; i < BLOCKBITMAP_SIZE; i++) {
			is_used[i] = 0;  // 设置所有位为 0，表示所有数据块初始时为空闲状态
		}
		is_used[0] = 1;  // 第 0 块作为引导块（Boot Block），根目录占用该块，将其标记为已使用
		for (int i = 0; i < BLOCKBITMAP_PADDING; i++) {
			padding[i] = 0; // 填充位图中的空余部分
		}
	}

	// 分配一个空闲数据块
	unsigned int Allocate_FreeBlock() {
		// Step 1: 遍历块位图找到第一个空闲块
		for (int row_index = 0; row_index < BLOCKBITMAP_SIZE; row_index++) {
			unsigned int& bitmap = this->is_used[row_index]; // 获取当前块的位图，使用引用减少内存开销
			// 遍历当前块的32个位
			for (int column_index = 0; column_index < 32; column_index++) {
				if ((bitmap & (1 << column_index)) == 0) { // 检查当前位是否为空闲（0表示空闲）
					bitmap |= (1 << column_index); // 将当前位设置为已占用（1表示已使用）
					return row_index * 32 + column_index; // 返回空闲块的下标
				}
			}
		}
		return Nothing; // 如果没有找到空闲块，返回“无”表示失败（理论上不应该到达这里）
	}

	// 移除某个已占用块的位图标记
	void Remove_Block_BitMap(unsigned int block_position) {
		unsigned int row_index = 0, column_index = 0; // 块索引和位索引
		row_index = block_position / 32; // 计算块索引
		column_index = block_position % 32;   // 计算位索引
		unsigned int mask = ~(1 << (31 - column_index)); // 创建掩码，清除对应位置的 1（表示空闲）
		this->is_used[row_index] &= mask; // 清除相应位置的位，标记为未使用（0表示空闲）
	}
};

class iNodeBitMap { // 用于表示文件系统的 iNode 位图（iNode Bitmap），占 4 个磁盘块，大小为 4KB
public:
	unsigned int is_used[INODEBITMAP_SIZE];  // iNode 位图数组，每个元素表示 32 个 iNode 的使用情况

	// 构造函数，初始化 iNode 位图
	iNodeBitMap() {
		// 初始化所有 iNode 位图为 0，表示所有 iNode 初始时为空闲
		for (int i = 0; i < INODEBITMAP_SIZE; i++) {
			is_used[i] = 0;  // 将所有位设置为 0，表示所有 iNode 初始时为空闲状态
		}
		is_used[0] = 1;  // 第 0 个 iNode 已被占用（根目录占用），将其设置为已使用
	}

	// 分配一个空闲的 iNode
	unsigned int Allocate_FreeINode() {
		// Step 1: 遍历 iNode 位图，找到第一个空闲的 iNode
		for (int row_index = 0; row_index < INODEBITMAP_SIZE; row_index++) {
			unsigned int& bitmap = this->is_used[row_index];  // 获取当前 iNode 位图块，引用减少内存开销
			// 遍历当前块的 32 个 bit 位
			for (int column_index = 0; column_index < 32; column_index++) {
				// 检查当前 bit 位是否为空闲（0表示空闲）
				if ((bitmap & (1 << column_index)) == 0) {
					// 将当前 bit 位设置为已占用（1表示已使用）
					bitmap |= (1 << column_index);
					return row_index * 32 + column_index;  // 返回分配的 iNode 索引
				}
			}
		}
		return Nothing;  // 如果没有空闲的 iNode，返回 Nothing（理论上不应达到此处）
	}

	// 释放已占用的 iNode 位图空间
	void Remove_Inode_Bitmap(unsigned int current_file_inode_index) {
		// Step 1: 计算当前 iNode 所在的块索引和位索引
		unsigned int row_index = current_file_inode_index / 32;  // 计算所在磁盘块的索引
		unsigned int column_index = current_file_inode_index % 32;  // 计算位索引（32 个 bit 位）

		// Step 2: 清除该 bit 位，表示该 iNode 被释放
		this->is_used[row_index] &= ~(1 << (31 - column_index));  // 清除对应的位（将 1 变为 0，标记为空闲）
	}
};

class iNode { // iNode 类，用于表示文件的元数据，每个 iNode 占 128B
public:
	char file_name[NameLength];            // 文件名，最长为 NameLength
	unsigned int parent_directory_index;   // 父目录的 iNode 编号，用于标识文件所在的目录
	unsigned int nextInode_index;          // 下一个 iNode 编号（链表结构中指向下一个 iNode 的位置）
	bool isDirectory;                      // 文件类型标识：true 表示目录，false 表示普通文件
	int files_num;                         // 如果是目录文件，记录该目录下文件数量
	int owner_id;                          // 文件所有者的用户 ID
	bool ReadPermission;                   // 读取权限：true 表示可读，false 表示不可读
	bool WritePermission;                  // 写入权限：true 表示可写，false 表示不可写
	int block_num;                         // 文件占用的块数；目录文件最多占一个块，普通文件可占多个块
	unsigned int blockPointer[MAX_BLOCK_POINTER]; // 存储数据块的指针，目录文件存放子文件的 iNode，普通文件存放数据块号
	unsigned int filesize;                 // 文件大小

	// 构造函数：初始化为根目录
	iNode() {
		for (int i = 0; i < sizeof(file_name); ++i) file_name[i] = '\0'; // 文件名初始化为空字符串
		file_name[0] = '/';                    // 根目录文件名设为 '/'
		parent_directory_index = Nothing;      // 根目录没有父目录，设置为空
		nextInode_index = Nothing;             // 初始化为无后继 iNode
		isDirectory = true;                    // 设为目录
		files_num = 0;                         // 根目录初始文件数量为 0
		owner_id = AdminID;                    // 所有者设为根用户
		ReadPermission = true;                 // 默认允许读取权限
		WritePermission = true;                // 默认允许写入权限
		block_num = 1;                         // 根目录占用 1 个数据块
		blockPointer[0] = 0;                   // 根目录的第一个块号为 0
		for (int i = 1; i < MAX_BLOCK_POINTER; ++i) blockPointer[i] = Nothing; // 初始化剩余块指针为空
	}

	// 重置 iNode 信息，将其重新设为根目录的初始状态
	void reset_info() {
		for (int i = 0; i < sizeof(file_name); ++i) file_name[i] = '\0';  // 清空文件名
		file_name[0] = '/';                    // 文件名设置为 '/'
		parent_directory_index = Nothing;      // 根目录没有父目录
		nextInode_index = Nothing;             // 没有下一个 iNode
		isDirectory = true;                    // 设置为目录
		files_num = 0;                         // 设置文件数量为 0
		owner_id = AdminID;                    // 所有者为根用户
		ReadPermission = true;                 // 读取权限设置为可读
		WritePermission = true;                // 写入权限设置为可写
		block_num = 1;                         // 目录文件占 1 个数据块
		blockPointer[0] = 0;                   // 目录的第一个块号为 0
		for (int i = 1; i < MAX_BLOCK_POINTER; ++i) blockPointer[i] = Nothing; // 清空剩余块指针
	}

	// 清空所有 iNode 信息，表示不再占用任何块
	void clear_info() {
		for (int i = 0; i < sizeof(file_name); ++i) file_name[i] = '\0'; // 清空文件名
		parent_directory_index = Nothing;      // 清空父目录索引
		nextInode_index = Nothing;             // 清空后继 iNode 索引
		isDirectory = true;                    // 设为目录
		files_num = 0;                         // 设置文件数量为 0
		owner_id = AdminID;                    // 设置为根用户
		ReadPermission = true;                 // 读取权限设置为可读
		WritePermission = true;                // 写入权限设置为可写
		block_num = 0;                         // 设置占用块数为 0
		for (int i = 0; i < MAX_BLOCK_POINTER; ++i) blockPointer[i] = Nothing; // 清空块指针
	}

	// 设置新文件的 iNode 信息
	void setup_new_file(const string& filename, unsigned int father_inode) {
		strncpy_s(file_name, filename.c_str(), NameLength - 1);  // 设置文件名
		file_name[NameLength - 1] = '\0';                          // 确保文件名以 '\0' 结尾
		parent_directory_index = father_inode; // 设置父目录的 iNode 编号
		nextInode_index = Nothing;             // 新文件无后继 iNode
		isDirectory = false;                   // 标记为普通文件
		files_num = 0;                         // 普通文件不含子文件
		owner_id = Current_User_Id;            // 设置文件创建者的用户 ID
		ReadPermission = Current_Permission > 0 ? true : false;  // 根据当前权限设置读取权限
		WritePermission = Current_Permission > 1 ? true : false; // 根据当前权限设置写入权限
		block_num = 0;                         // 初始为空文件，不占用数据块
	}

	// 设置新目录的 iNode 信息
	void setup_new_directory(const string& directory_name, unsigned int father_inode, unsigned int datablock) {
		strncpy_s(file_name, directory_name.c_str(), NameLength - 1);  // 设置目录名
		file_name[NameLength - 1] = '\0';                               // 确保目录名以 '\0' 结尾
		parent_directory_index = father_inode; // 设置父目录的 iNode 编号
		nextInode_index = Nothing;             // 新目录无后继 iNode
		isDirectory = true;                    // 标记为目录文件
		files_num = 0;                         // 初始目录下无文件
		owner_id = Current_User_Id;            // 设置创建者的用户 ID
		ReadPermission = Current_Permission > 0 ? true : false;  // 根据当前权限设置读取权限
		WritePermission = Current_Permission > 1 ? true : false; // 根据当前权限设置写入权限
		block_num = 1;                         // 目录文件占用 1 个数据块
		blockPointer[0] = datablock;           // 记录目录的数据块编号
	}

	// 判断该 iNode 是否为目录类型
	bool isDirectoryType() const {
		return this->isDirectory;
	}

	// 判断该 iNode 是否为空目录
	bool isDirectoryEmpty() {
		return (this->isDirectory == true && this->files_num == 0);
	}

	// 检查当前用户是否有读取权限
	bool isRead_Available() {
		// 非管理员且文件所有者与当前用户不匹配，且没有读取权限时返回 false
		if (Current_User_Id != AdminID && this->owner_id != Current_User_Id && ReadPermission == false) {
			return false;
		}
		return true;
	}

	// 检查当前用户是否有写入权限
	bool isWrite_Available() {
		// 非管理员且文件所有者与当前用户不匹配，且没有写入权限时返回 false
		if (Current_User_Id != AdminID && this->owner_id != Current_User_Id && WritePermission == false) {
			return false;
		}
		return true;
	}

	// 获取文件的权限表示字符串（如 "rwx"）
	string getPermission() {
		string permission = "";
		if (this->isRead_Available()) {
			permission += "r";  // 允许读取
		}
		else {
			permission += "-";  // 不允许读取
		}
		if (this->isWrite_Available()) {
			permission += "w";  // 允许写入
		}
		else {
			permission += "-";  // 不允许写入
		}
		// 只有文件所有者才拥有执行权限（此处默认执行权限与拥有者权限挂钩）
		if (this->owner_id == Current_User_Id || Current_User_Id == AdminID) {
			permission += "x";  // 允许执行
		}
		else {
			permission += "-";  // 不允许执行
		}
		return permission;
	}

	// 计算该文件 iNode 所占的总空间大小
	unsigned int CalculateTempSize() {
		return INODE_SIZE + BLOCK_SIZE * this->block_num;  // iNode 大小 + 数据块大小 * 块数
	}
};

class iNode_Array { // 用于表示一个包含 4096 个模拟磁盘块的数组，每个块包含多个 iNode（32768 个）
public:
	iNode inode[32768]; // iNode 数组，每个 iNode 占 128 字节，总共有 32768 个 iNode（即 4096 个磁盘块）
};

class Block { // 一个磁盘块的大小为1024B（1KB）
public:
	unsigned int data[256];  // 数据数组，每个元素占用 4 字节（unsigned int），共 256 个元素，占用 1024 字节（即 1KB）

	Block() {
		this->clear_BlockData();  // 构造函数，初始化时清空块中的数据
	}

	// 清空磁盘块数据，将 data 数组中的每个元素设为 0
	void clear_BlockData() {
		for (auto& d : data) {
			d = 0;  // 清除数据，将每个元素设为 0
		}
	}

	// 从文件中加载指定位置的数据块
	void loadBlockData(unsigned int position) {
		std::fstream file_IO(My_File_System_Name, std::ios::in | std::ios::binary); // 打开文件以读取数据
		unsigned int offset = position * sizeof(Block); // 计算数据块在文件中的偏移量
		file_IO.seekg(offset, std::ios::beg); // 定位到文件的正确位置
		file_IO.read(reinterpret_cast<char*>(this), sizeof(Block)); // 读取数据到当前 Block 对象
		file_IO.close();  // 关闭文件
	}

	// 将当前数据块保存到文件中指定位置
	void saveBlockData(unsigned int position) {
		std::fstream file_IO(My_File_System_Name, std::ios::in | std::ios::out | std::ios::binary); // 打开文件以读写数据
		unsigned int offset = position * sizeof(Block); // 计算数据块在文件中的偏移量
		file_IO.seekp(offset, std::ios::beg); // 定位到文件的正确位置
		file_IO.write(reinterpret_cast<char*>(this), sizeof(Block)); // 将当前 Block 对象的数据写入文件
		file_IO.close(); // 关闭文件
	}

	// 将从磁盘读取的多个数据块复制到当前块的数据数组中
	void copyBlockData(int times) {
		for (int i = 0; i < times; i++) {
			this->data[i] = Read_Data_To_Block(); // 从磁盘读取数据到 data 数组
		}
	}
};

class Path { // 用于路径分解，方便查找文件系统中的文件路径
public:
	unsigned int path_count; // 路径中分解出来的部分的计数器
	string path_segment[3000]; // 用于存储路径中的每一部分，最多可以存储 3000 个路径段

	Path() {
		clear();  // 构造函数，初始化路径分解器
	}

	// 清空路径数据
	void clear() {
		path_count = 0; // 路径部分计数器归零
		for (int i = 0; i < 3000; i++) {
			path_segment[i] = ""; // 将每个路径段初始化为空字符串
		}
	}

	// 手动遍历字符串拆解路径，将路径分解成各个部分
	void Divide_Path(string absolute_path) {
		this->clear(); // 清空之前的路径分解结果
		unsigned int length = absolute_path.length(); // 获取路径字符串的长度
		string segment = ""; // 临时字符串，用于拼接路径段

		for (unsigned int i = 1; i < length; i++) { // 从 1 开始跳过根目录的 '/'
			if (absolute_path[i] == '/') { // 路径中的分隔符
				if (!segment.empty()) { // 如果当前路径段不为空
					this->path_segment[this->path_count] = segment; // 将当前路径段保存到 path_segment 数组
					this->path_count++; // 增加路径部分计数器
					segment.clear(); // 清空临时路径段变量，为下一个路径段做准备
				}
			}
			else { // 如果不是 '/'，则继续拼接当前的路径段
				segment += absolute_path[i];
			}
		}

		// 最后一个路径段添加到 path 中
		if (!segment.empty()) { // 如果最后一个路径段非空
			this->path_segment[this->path_count] = segment; // 保存最后的路径段
			this->path_count++; // 路径部分计数器加 1
		}
	}
};

extern BootBlock bootblock;  // 引导块（Boot Block），文件系统的启动信息，通常存储文件系统的元数据，如磁盘的基本信息
extern SuperBlock superblock;  // 超级块（Super Block），存储文件系统的总体信息，包括总块数、已使用块数、总 iNode 数等
extern GroupDescription group_desciption;  // 分组描述符（Group Descriptor），用于描述磁盘上各个分区的信息，通常用于支持分区的文件系统
extern BlockBitMap block_bitmap;  // 块位图（Block Bitmap），用于跟踪磁盘上每个数据块是否被占用，用 0 表示未使用，1 表示已使用
extern iNodeBitMap inode_bitmap;  // iNode 位图（iNode Bitmap），用于跟踪文件系统中的每个 iNode 是否被占用，1 表示已使用，0 表示未使用
extern iNode_Array inode_array;  // iNode 数组（iNode Array），存储所有的 iNode，每个 iNode 描述了一个文件或目录的元数据（如文件名、权限等）

extern Path path;  // 路径解析器（Path），用于分解和存储路径中的每一部分，便于文件系统操作中的路径查找

extern queue<Block> blockCache;  // 块缓存（Block Cache），用于缓存从主机系统拷贝到模拟文件系统的数据


// 获取指定索引的 iNode
iNode* GetInode(unsigned int inode_index);

// 递归计算文件或目录的总大小（根据 iNode 索引）
unsigned int CalculateFileSize(unsigned int file_inode_index);

// 递归计算文件或目录的总大小（根据传入的 iNode 对象）
unsigned int CalculateFileSize(iNode* fileInode);

// 用于获取文件或目录的详细信息，并格式化输出为字符串
string SubFileInformation(iNode* fileInode);

// 用于设置表头信息并格式化输出为字符串，通常用于显示文件系统内容的头部
void SetMessageHeader(std::string& message_to_shell);

// 获取当前目录的绝对路径
string Get_DirectoryAbsolutePath();

// 尝试打开模拟磁盘文件，检查文件是否可访问
bool Open_Simdisk();

// 判断是否满足分配条件（即是否可以继续分配新的 inode 或数据块）
bool is_Allocation_Available(unsigned int father_directory_inode);

// 获取一个空闲的数据块索引
unsigned int Get_FreeBlockIndex();

// 获取一个空闲的 iNode 索引
unsigned int Get_FreeINodeIndex();

// 写入/读取 文件系统数据
void YDFS_IO(std::fstream& file, bool isWrite);

// 创建文件系统并写入
void Create_Simdisk();

// 加载文件系统数据
void Load_Simdisk();

// 保存文件系统数据
void Save_Simdisk();

// 核对文件系统数据一致性
bool Compare_With_File();

// 将相对路径转换为绝对路径
void Change_To_AbsolutePath(string& relative_path);

// 在当前目录下查找文件名为 filename 的文件，返回对应的 iNode 索引
unsigned int Search_File_INode_Under_Directory(unsigned int now_directory, string filename);

// 根据绝对路径查找文件的 iNode 索引
unsigned int Search_File_INode(string absolute_path);

// 判断在父目录下是否存在名为 sonFile_name 的文件
bool is_File_Exist(unsigned int fatherDirectory_inode, string sonFile_name);

// 更新父目录信息，新增一个文件的 iNode，并在父目录的数据块中更新文件列表
void Update_info_Create(unsigned int father_directory_inode, unsigned int file_inode);

// 更新父目录信息，删除指定文件的 iNode，并在父目录的数据块中更新文件列表
void Update_info_Remove(unsigned int current_file_inode);

// 创建一个新文件并将其添加到指定的父目录
unsigned int Create_New_File(unsigned int father_directory_inode, string filename);

// 创建一个新目录并将其添加到指定的父目录
unsigned int Create_New_Directory(unsigned int father_directory_inode, string directory_name);

// 清理指定文件的 iNode 位图标记，释放该文件占用的 iNode
void Clear_File_INode_Bitmap(unsigned int current_file_inode);

// 清理指定文件的数据块位图标记，释放该文件占用的数据块
void Clear_File_Block_Bitmap(unsigned int current_file_inode);

// 清空指定文件所占用的数据块
void Clear_File_DataBlock(unsigned int current_file_inode);

// 删除指定的普通文件
void Delete_File(unsigned int current_file_inode);

// 删除指定的空目录
void Remove_Empty_Directory(unsigned int current_directory_Inode);

// 递归删除指定的目录及其子目录和文件
void Remove_Directory(unsigned int dirInode);

// 在文件系统内部将一个文件复制到另一个位置
void Transfer_File_Within_Simdisk(unsigned int src_inode, unsigned int dest_inode);

// 将主机上的文件加载到缓存中，以便在文件系统中使用
void load_HostFile_To_Cache(string hostpath);

// 将缓冲区的数据追加写入文件系统
void Write_Cache_To_Simdisk(unsigned int file_inode);

#endif

