#include "Simdisk.h"

BootBlock bootblock;
SuperBlock superblock;
GroupDescription group_desciption;
BlockBitMap block_bitmap;
iNodeBitMap inode_bitmap;
iNode_Array inode_array;

Path path;
queue<Block> blockCache; //用于主机系统与模拟文件系统YDFS之间的数据拷贝                     



// 获取指定索引的 iNode
iNode* GetInode(unsigned int inode_index) {
	// 判断 iNode 索引是否有效
	if (inode_index < 32768) {  // 最大支持 32768 个 iNode
		// 返回对应索引的 iNode
		return &inode_array.inode[inode_index];
	}
	else {
		// 如果索引无效，返回空指针
		return nullptr;
	}
}

// 递归计算文件或目录的总大小（根据 iNode 索引）
unsigned int CalculateFileSize(unsigned int file_inode_index) {
	// 获取指定索引的 iNode
	iNode* fileInode = GetInode(file_inode_index);
	unsigned int totalSize = 0;

	// 如果是目录类型
	if (fileInode->isDirectoryType()) {
		// 计算目录文件本身的大小
		totalSize += fileInode->CalculateTempSize();

		// 遍历目录下的每个文件（子文件）
		for (unsigned int i = 0; i < fileInode->files_num; ++i) {
			// blockPointer 存储的是子文件的 iNode 编号
			unsigned int child_inode_index = fileInode->blockPointer[i];
			// 递归计算子文件的大小
			totalSize += CalculateFileSize(child_inode_index);
		}
	}
	else {
		// 普通文件，直接计算其大小
		totalSize += fileInode->CalculateTempSize();
		// 处理链接文件：如果该文件有下一个 iNode，则递归计算
		while (fileInode->nextInode_index != Nothing) {
			// 获取下一个 iNode
			fileInode = GetInode(fileInode->nextInode_index);
			// 递归计算下一个 iNode 的大小
			totalSize += fileInode->CalculateTempSize();
		}
	}
	// 返回计算的总大小
	return totalSize;
}

// 递归计算文件或目录的总大小（根据传入的 iNode 对象）
unsigned int CalculateFileSize(iNode* fileInode) {
	unsigned int totalSize = 0;

	// 如果是目录类型
	if (fileInode->isDirectoryType()) {
		// 计算目录文件本身的大小
		totalSize += fileInode->CalculateTempSize();
		// 获取目录的第一个数据块的位置
		unsigned int blockpostion = fileInode->blockPointer[0];
		blockpostion = GetDataBlockPosition(blockpostion);  // 获取数据块的实际位置
		Block dirBlock;
		// 加载目录块数据
		dirBlock.loadBlockData(blockpostion);

		// 遍历目录下的每个文件（子文件）
		for (unsigned int i = 0; i < fileInode->files_num; ++i) {
			// 假设目录块中存储的是子文件的 iNode 编号
			unsigned int child_inode_index = dirBlock.data[i];
			// 递归计算子文件的大小
			totalSize += CalculateFileSize(GetInode(child_inode_index));
		}
	}
	else {
		// 普通文件，直接计算其大小
		totalSize += fileInode->CalculateTempSize();
		// 处理链接文件：如果该文件有下一个 iNode，则递归计算
		while (fileInode->nextInode_index != Nothing) {
			// 获取下一个 iNode
			fileInode = GetInode(fileInode->nextInode_index);
			// 递归计算下一个 iNode 的大小
			totalSize += fileInode->CalculateTempSize();
		}
	}
	// 返回计算的总大小
	return totalSize;
}

// 用于获取文件或目录的详细信息，并格式化输出为字符串
string SubFileInformation(iNode* fileInode) {
	std::stringstream message;

	// 定义输出的列宽度
	int fileNameWidth = 36;      // 文件名宽度
	int typeWidth = 15;          // 类型宽度
	int permissionWidth = 20;    // 权限宽度
	int sizeWidth = 20;          // 大小宽度（包括单位）
	int parentDirWidth = 20;     // 父目录宽度

	// 文件名：左对齐并设置列宽
	message << std::left << std::setw(fileNameWidth) << fileInode->file_name;

	// 判断文件类型并输出相关信息
	if (fileInode->isDirectoryType()) {
		// 如果是目录类型
		// 获取父目录名称，若没有父目录则默认显示 "-"
		std::string parentDirName = "-";  // 默认值
		if (fileInode->parent_directory_index != Nothing) {  // 检查是否有父目录
			// 获取父目录的 iNode
			iNode* parentInode = GetInode(fileInode->parent_directory_index);
			if (parentInode) {
				// 获取父目录的文件名
				parentDirName = parentInode->file_name;
			}
		}

		// 格式化输出：目录的类型、权限、大小（单位为字节）和父目录名
		message << std::setw(typeWidth) << "[DIR]"
			<< std::setw(permissionWidth) << fileInode->getPermission()  // 文件权限
			<< std::setw(sizeWidth) << (std::to_string(CalculateFileSize(fileInode)) + " B")  // 文件大小（单位：字节）
			<< std::setw(parentDirWidth) << parentDirName << "\n";  // 父目录名称
	}
	else {
		// 如果是普通文件类型
		// 格式化输出：文件的类型、权限、大小（单位为字节）
		message << std::setw(typeWidth) << "[FILE]"
			<< std::setw(permissionWidth) << fileInode->getPermission()
			<< std::setw(sizeWidth) << (std::to_string(CalculateFileSize(fileInode)) + " B");

		// 获取父目录名，并输出
		iNode* parentInode = GetInode(fileInode->parent_directory_index);
		std::string parentDirName = parentInode ? parentInode->file_name : "-";  // 若没有父目录则显示 "-"
		message << std::setw(parentDirWidth) << parentDirName << "\n";
	}

	if (fileInode->file_name == "/") {
		return "";
	}
	// 返回格式化后的信息字符串
	return message.str();
}

// 用于设置表头信息并格式化输出为字符串，通常用于显示文件系统内容的头部
void SetMessageHeader(std::string& message_to_shell) {
	std::stringstream header;

	// 定义表头的列宽度
	int fileNameWidth = 36;      // 文件名宽度
	int typeWidth = 15;          // 类型宽度
	int permissionWidth = 20;    // 权限宽度
	int sizeWidth = 15;          // 大小宽度
	int parentDirWidth = 20;     // 父目录宽度

	// 使用相同的列宽和对齐方式设置表头
	header << std::left << std::setw(fileNameWidth) << "子目录/子文件名"
		<< std::setw(typeWidth) << "文件类型"
		<< std::setw(permissionWidth) << "文件权限"
		<< std::setw(sizeWidth) << "文件大小"
		<< std::setw(parentDirWidth) << "父目录" << "\n";

	// 将格式化后的表头信息存入 message_to_shell
	message_to_shell = header.str();
} 

// 获取当前目录的绝对路径
string Get_DirectoryAbsolutePath() {
	string absolutePath = "";  // 初始化为空字符串，用于存储绝对路径
	unsigned int current_directory_inode_index = Current_Directory_iNode;  // 当前目录的 inode 索引
	iNode* dirInode = GetInode(current_directory_inode_index); // 获取当前目录的 iNode

	// 从当前目录向上遍历，逐层构建绝对路径
	for (;; current_directory_inode_index = dirInode->parent_directory_index) { // 无限循环，直到到达根目录
		dirInode = GetInode(current_directory_inode_index); // 获取当前目录的 iNode
		string dirName = dirInode->file_name;  // 获取当前目录的名称

		// 如果到达根目录（即文件名为 "/"），退出循环
		if (dirName == "/") {
			break;  // 退出循环
		}

		// 将当前目录名添加到路径的前面
		absolutePath = "/" + dirName + absolutePath;  // 拼接目录名到路径
	}

	return absolutePath;  // 返回最终的绝对路径
}

// 尝试打开模拟磁盘文件，检查文件是否可访问
bool Open_Simdisk() {
	// Step 1: 尝试以只读二进制模式打开文件系统的二进制文件
	file_IO.open(My_File_System_Name, ios::in | ios::binary);

	// Step 2: 检查文件是否成功打开
	if (!file_IO) {
		// 如果文件打开失败，打印错误信息并返回 false
		cerr << "错误: 打开文件系统 '" << My_File_System_Name << "' 失败" << endl;
		return false;  // 文件打开失败
	}

	// Step 3: 文件成功打开后，关闭文件流并返回 true
	file_IO.close();  // 关闭文件流
	return true;  // 返回 true，表示文件系统成功打开
}

// 判断是否满足分配条件（即是否可以继续分配新的 inode 或数据块）
bool is_Allocation_Available(unsigned int father_directory_inode_index) {
	// 获取父目录的 iNode
	iNode* parentDir = GetInode(father_directory_inode_index);

	// 检查以下分配条件是否满足：
	// 1. 如果文件系统已用 inode 数量达到最大 inode 数量
	// 2. 如果文件系统已用数据块数量达到最大数据块数量
	// 3. 如果父目录的文件数量已达到最大文件数量
	if (superblock.used_inode == superblock.total_inode ||   // inode 使用完毕
		superblock.used_datablock == superblock.total_datablock || // 数据块使用完毕
		parentDir->files_num == superblock.file_num_directory) {  // 父目录文件数量已满
		return false;  // 不满足条件，不能进行分配
	}

	return true;  // 满足条件，可以进行分配
}

// 获取一个空闲的数据块索引
unsigned int Get_FreeBlockIndex() {
	// 尝试分配一个空闲的数据块
	unsigned int bit_position = block_bitmap.Allocate_FreeBlock(); // 调用位图分配函数，查找一个空闲块
	if (bit_position != Nothing) {
		// 如果成功分配，更新超级块中的已使用数据块数
		superblock.used_datablock++; // 更新超级块的数据块已用数
		return bit_position;  // 返回分配到的空闲数据块的索引
	}
	else {
		// 如果分配失败，返回 Nothing
		return Nothing;  // 无空闲数据块，返回 Nothing
	}
}

// 获取一个空闲的 iNode 索引
unsigned int Get_FreeINodeIndex() {
	// 尝试分配一个空闲的 iNode
	unsigned int bit_position = inode_bitmap.Allocate_FreeINode(); // 调用位图分配函数，查找一个空闲 iNode
	if (bit_position != Nothing) {
		// 如果成功分配，更新超级块中的已使用 iNode 数
		superblock.used_inode++; // 更新超级块的 iNode 已用数
		return bit_position;  // 返回分配到的空闲 iNode 的索引
	}
	else {
		// 如果分配失败，返回 Nothing
		return Nothing;  // 无空闲 iNode，返回 Nothing
	}
}

// 写入/读取 文件系统数据
void YDFS_IO(std::fstream& file, bool isWrite) {
	if (isWrite) {
		// 写入操作
		file.write((char*)&bootblock, sizeof(BootBlock));       // 写入引导块
		file.write((char*)&superblock, sizeof(SuperBlock));     // 写入超级块
		file.write((char*)&group_desciption, sizeof(GroupDescription)); // 写入组描述符
		file.write((char*)&block_bitmap, sizeof(BlockBitMap));       // 写入块位图
		file.write((char*)&inode_bitmap, sizeof(iNodeBitMap));       // 写入iNode位图
		file.write((char*)&inode_array, sizeof(iNode_Array));        // 写入iNode表
	}
	else {
		// 读取操作
		file.read((char*)&bootblock, sizeof(BootBlock));       // 提取引导块
		file.read((char*)&superblock, sizeof(SuperBlock));     // 提取超级块
		file.read((char*)&group_desciption, sizeof(GroupDescription)); // 提取组描述符
		file.read((char*)&block_bitmap, sizeof(BlockBitMap));       // 提取块位图
		file.read((char*)&inode_bitmap, sizeof(iNodeBitMap));       // 提取iNode位图
		file.read((char*)&inode_array, sizeof(iNode_Array));        // 提取iNode表
	}
}

// 创建文件系统并写入
void Create_Simdisk() {
	file_IO.open(My_File_System_Name, ios::out | ios::binary); // 打开文件用于写入

	YDFS_IO(file_IO, true);  // 写入数据

	// 写入数据块
	Block block;
	for (int i = 0; i < DataBlockNum; i++) {
		file_IO.write((char*)&block, sizeof(Block));  // 写入数据块
	}

	file_IO.close();
}

// 加载文件系统数据
void Load_Simdisk() {
	file_IO.open(My_File_System_Name, ios::in | ios::binary);  // 打开文件用于读取

	YDFS_IO(file_IO, false);  // 读取数据

	file_IO.close();
}

// 保存文件系统数据
void Save_Simdisk() {
	file_IO.open(My_File_System_Name, ios::in | ios::out | ios::binary); // 打开文件用于读取和写入
	file_IO.seekp(0, ios::beg); // 将写指针移动到文件开始

	YDFS_IO(file_IO, true);  // 写入数据

	file_IO.close();
}

// 核对文件系统数据一致性
bool Compare_With_File() {
	// 在堆上分配临时对象用于存储从文件读取的数据
	auto tempBootBlock = new BootBlock;           // 分配 BootBlock 临时对象
	auto tempSuperBlock = new SuperBlock;         // 分配 SuperBlock 临时对象
	auto tempGroupDescription = new GroupDescription; // 分配 GroupDescription 临时对象
	auto tempBlockBitmap = new BlockBitMap;       // 分配 BlockBitMap 临时对象
	auto tempInodeBitmap = new iNodeBitMap;       // 分配 iNodeBitMap 临时对象
	auto tempInodeArray = new iNode_Array;        // 分配 iNode_Array 临时对象

	// 打开文件，准备读取文件系统的二进制数据
	file_IO.open(My_File_System_Name, ios::in | ios::binary); // 以二进制读模式打开文件

	// Lambda 函数：读取数据到指定对象
	auto readData = [&](auto* obj, size_t size) {
		file_IO.read(reinterpret_cast<char*>(obj), size); // 从文件中读取指定大小的数据到 obj 指向的对象
	};

	// 使用 lambda 逐一读取各个结构体的数据
	readData(tempBootBlock, sizeof(BootBlock));           // 读取 BootBlock 数据
	readData(tempSuperBlock, sizeof(SuperBlock));         // 读取 SuperBlock 数据
	readData(tempGroupDescription, sizeof(GroupDescription)); // 读取 GroupDescription 数据
	readData(tempBlockBitmap, sizeof(BlockBitMap));       // 读取 BlockBitMap 数据
	readData(tempInodeBitmap, sizeof(iNodeBitMap));       // 读取 iNodeBitMap 数据
	readData(tempInodeArray, sizeof(iNode_Array));        // 读取 iNode_Array 数据

	// 关闭文件流
	file_IO.close();  // 完成读取后关闭文件

	// Lambda 函数：比较两个对象的内存内容是否相等
	auto isEqual = [](const auto* obj1, const auto* obj2, size_t size) {
		return std::memcmp(obj1, obj2, size) == 0; // 使用 memcmp 比较 obj1 和 obj2 指向的内存数据是否相同
	};

	// 使用 lambda 逐个比较内存数据
	bool result = isEqual(&bootblock, tempBootBlock, sizeof(BootBlock)) && // 比较 BootBlock
		isEqual(&superblock, tempSuperBlock, sizeof(SuperBlock)) &&       // 比较 SuperBlock
		isEqual(&group_desciption, tempGroupDescription, sizeof(GroupDescription)) && // 比较 GroupDescription
		isEqual(&block_bitmap, tempBlockBitmap, sizeof(BlockBitMap)) &&   // 比较 BlockBitMap
		isEqual(&inode_bitmap, tempInodeBitmap, sizeof(iNodeBitMap)) &&   // 比较 iNodeBitMap
		isEqual(&inode_array, tempInodeArray, sizeof(iNode_Array));       // 比较 iNode_Array

	// 释放堆上分配的临时内存
	delete tempBootBlock;         // 释放 BootBlock 对象
	delete tempSuperBlock;        // 释放 SuperBlock 对象
	delete tempGroupDescription;  // 释放 GroupDescription 对象
	delete tempBlockBitmap;       // 释放 BlockBitMap 对象
	delete tempInodeBitmap;       // 释放 iNodeBitMap 对象
	delete tempInodeArray;        // 释放 iNode_Array 对象

	return result; // 返回比较结果，若所有数据一致返回 true，否则返回 false
}

// 将相对路径转换为绝对路径
void Change_To_AbsolutePath(string& relativePath) {
	// 如果是绝对路径，直接返回，不做任何处理
	if (relativePath[0] == '/') return;

	// 如果路径没有以 '.' 开头，意味着它是一个普通的路径，添加 './' 作为相对路径的前缀
	if (relativePath[0] != '.') {
		relativePath = "./" + relativePath; // 将路径加上相对路径的标识符
	}

	// 创建一个空字符串 temp，用来存储去掉前缀的路径部分
	string temp = "";
	int length = relativePath.length();

	// 从第二个字符开始遍历路径字符串（跳过 './'）
	for (int i = 1; i < length; i++) {
		temp += relativePath[i]; // 将路径中的字符添加到 temp 中
	}

	// 更新 relativePath 为去掉 './' 前缀后的路径
	relativePath = temp;

	// 获取当前工作目录的绝对路径，并将其与相对路径连接成完整的绝对路径
	relativePath = Get_DirectoryAbsolutePath() + relativePath;

	// 输出转换后的绝对路径（用于调试）
	cout << relativePath << endl;
}

// 在当前目录下查找文件名为 filename 的文件，返回对应的 iNode 索引
unsigned int Search_File_INode_Under_Directory(unsigned int current_directory_inode_index, string filename) {
	// 根据当前目录的 iNode 索引获取该目录的 iNode 信息
	iNode* dirInode = GetInode(current_directory_inode_index);

	// 获取该目录所在的第一个数据块的磁盘块位置
	unsigned int directory_block_position = dirInode->blockPointer[0];

	// 获取数据块的位置并读取数据块
	directory_block_position = GetDataBlockPosition(directory_block_position);
	Block data_block;
	data_block.loadBlockData(directory_block_position);

	// 在该目录的数据块中，遍历目录下所有的文件，查找目标文件名
	for (int i = 0; i < dirInode->files_num; i++) {
		// 通过当前文件的 iNode 索引查找文件对应的 iNode 信息
		unsigned int current_file_inode_index = data_block.data[i];
		iNode* tempInode = GetInode(current_file_inode_index);

		// 如果找到匹配的文件名，返回该文件的 iNode 索引
		if (tempInode->file_name == filename) {
			return current_file_inode_index;
		}
	}

	// 如果遍历完所有文件都没有找到目标文件，返回 Nothing（文件不存在）
	return Nothing;
}

// 根据绝对路径查找文件的 iNode 索引
unsigned int Search_File_INode(string absolute_path) {
	// 特判根目录路径，直接返回根目录的 iNode 索引 0
	if (absolute_path == "/") return 0; // 根目录直接返回 iNode 号 0

	// 分解绝对路径，得到各个路径段
	path.Divide_Path(absolute_path);

	// 从根目录开始逐层查找
	unsigned temp_inode = Root_Directory; // 起始点是根目录
	for (int i = 0; i < path.path_count; i++) {
		// 逐层向下查找子目录的 iNode，或者最终查找目标文件的 iNode
		temp_inode = Search_File_INode_Under_Directory(temp_inode, path.path_segment[i]);

		// 如果某一层的子目录或文件未找到，路径无效，返回 Nothing
		if (temp_inode == Nothing) {
			return Nothing; // 路径有误，返回 Nothing
		}
	}

	// 返回最终找到的 iNode 索引
	return temp_inode;
}

// 判断在父目录下是否存在名为 sonFile_name 的文件
bool is_File_Exist(unsigned int fatherDirectory_inode, string SubFile_name) {
	// 调用 Search_File_INode_Under_Directory 函数查找目标文件的 iNode
	unsigned int find_inode = Search_File_INode_Under_Directory(fatherDirectory_inode, SubFile_name);

	// 如果未找到目标文件，返回 false，表示文件不存在
	if (find_inode == Nothing) {
		return false;
	}

	// 如果找到了目标文件，返回 true，表示文件存在
	return true;
}

// 更新父目录信息，新增一个文件的 iNode，并在父目录的数据块中更新文件列表
void Update_info_Create(unsigned int father_directory_inode, unsigned int file_inode) {
	// 根据父目录的 iNode 索引获取父目录的 iNode 信息
	iNode* parentInode = GetInode(father_directory_inode);

	// 增加父目录中的文件数量
	parentInode->files_num++;

	// 获取父目录的第一个数据块位置
	unsigned int block_position = parentInode->blockPointer[0];
	block_position = GetDataBlockPosition(block_position); // 获取数据块的实际位置

	// 创建一个新的数据块对象并加载数据块内容
	Block new_datablock;
	new_datablock.loadBlockData(block_position);

	// 将新创建的文件 iNode 编号存储到父目录的数据块中
	int index = parentInode->files_num - 1; // 新文件应该存储在文件数目最后的位置
	new_datablock.data[index] = file_inode;  // 将文件的 iNode 编号添加到数据块

	// 将更新后的数据块保存回磁盘
	new_datablock.saveBlockData(block_position);
}

// 更新父目录信息，删除指定文件的 iNode，并在父目录的数据块中更新文件列表
void Update_info_Remove(unsigned int current_file_inode_index) {
	// 根据文件的 iNode 索引获取该文件的 iNode 信息
	iNode* fileInode = GetInode(current_file_inode_index);

	// 获取文件的父目录 iNode
	unsigned int father_directory_inode = fileInode->parent_directory_index;
	iNode* parentDir = GetInode(father_directory_inode);

	// 获取父目录的第一个数据块位置
	unsigned int block_position = parentDir->blockPointer[0];
	block_position = GetDataBlockPosition(block_position); // 获取数据块的实际位置

	// 创建一个新的数据块对象并加载数据块内容
	Block new_datablock;
	new_datablock.loadBlockData(block_position);

	// 标记是否已找到要删除的文件
	bool isFileDeleted = false;

	// 遍历父目录的数据块，找到要删除的文件并将后续文件向前移动，填补删除文件的空位
	for (int i = 0; i < parentDir->files_num; i++) {
		if (isFileDeleted) {
			// 如果已找到删除的文件，后续的文件向前移动
			new_datablock.data[i] = new_datablock.data[i + 1];
		}
		else if (new_datablock.data[i] == current_file_inode_index) {
			// 找到要删除的文件，标记并将后续文件移动到当前文件的位置
			isFileDeleted = true;
			new_datablock.data[i] = new_datablock.data[i + 1];
		}
	}

	// 保存更新后的数据块
	new_datablock.saveBlockData(block_position);

	// 减少父目录中的文件数量
	parentDir->files_num--;
}

// 创建一个新文件并将其添加到指定的父目录
unsigned int Create_New_File(unsigned int father_directory_inode, string filename) {
	// 检查文件系统是否有足够的资源（iNode 和数据块）来创建新文件
	if (!is_Allocation_Available(father_directory_inode)) {
		return Nothing;  // 如果资源不足，返回 Nothing 表示创建失败
	}

	// 在 iNode 位图中找到一个空闲的 iNode，并分配给新文件
	unsigned int file_inode_index = Get_FreeINodeIndex();

	// 更新父目录的信息，将新文件的 iNode 添加到父目录的数据块中
	Update_info_Create(father_directory_inode, file_inode_index);

	// 获取新创建的文件的 iNode 指针
	iNode* fileInode = GetInode(file_inode_index);

	// 设置新文件的属性，包括文件名和父目录的 iNode 索引
	fileInode->setup_new_file(filename, father_directory_inode);

	// 保存文件系统的元数据，确保数据一致性
	Save_Simdisk();

	// 更新当前工作目录为父目录
	Current_Directory_iNode = father_directory_inode;

	// 返回新文件的 iNode 索引
	return file_inode_index;
}

// 创建一个新目录并将其添加到指定的父目录
unsigned int Create_New_Directory(unsigned int father_directory_inode_index, string directory_name) {
	// 检查文件系统是否有足够的资源（iNode 和数据块）来创建新目录
	if (!is_Allocation_Available(father_directory_inode_index)) {
		return Nothing;  // 如果资源不足，返回 Nothing 表示创建失败
	}

	// 在 iNode 位图中找到一个空闲的 iNode，并分配给新目录
	unsigned int directory_inode_index = Get_FreeINodeIndex();

	// 在数据块位图中找到一个空闲的数据块，用于存储新目录的内容
	unsigned int datablock_index = Get_FreeBlockIndex();

	// 更新父目录的信息，将新目录的 iNode 添加到父目录的数据块中
	Update_info_Create(father_directory_inode_index, directory_inode_index);

	// 获取新创建的目录的 iNode 指针
	iNode* dirInode = GetInode(directory_inode_index);

	// 设置新目录的属性，包括目录名、父目录的 iNode 索引和数据块索引
	dirInode->setup_new_directory(directory_name, father_directory_inode_index, datablock_index);

	// 保存文件系统的元数据，确保数据一致性
	Save_Simdisk();

	// 更新当前工作目录为新创建的目录
	Current_Directory_iNode = directory_inode_index;

	// 返回新目录的 iNode 索引
	return directory_inode_index;
}

// 清理指定文件的 iNode 位图标记，释放该文件占用的 iNode
void Clear_File_INode_Bitmap(unsigned int current_file_inode_index) {

	// 从 iNode 位图中移除当前文件的 iNode 标记，表示该 iNode 已被释放
	inode_bitmap.Remove_Inode_Bitmap(current_file_inode_index);
	superblock.used_inode--; // 减少已使用的 iNode 计数

	// 获取当前文件的 iNode 信息
	iNode* fileInode = GetInode(current_file_inode_index);

	// 检查该文件是否占用了多个 iNode（即是否有后续的 iNode）
	if (fileInode->nextInode_index != Nothing) {
		// 如果存在下一个 iNode，递归清理下一个 iNode 的位图标记
		Clear_File_INode_Bitmap(fileInode->nextInode_index);
	}
}

// 清理指定文件的数据块位图标记，释放该文件占用的数据块
void Clear_File_Block_Bitmap(unsigned int current_file_inode_index) {

	// 获取当前文件的 iNode 信息
	iNode* fileInode = GetInode(current_file_inode_index);

	// 遍历当前文件所占用的所有数据块，释放对应的块位图标记
	for (int i = 0; i < fileInode->block_num; i++) {
		unsigned int dataBlock_Position = fileInode->blockPointer[i]; // 获取数据块位置
		block_bitmap.Remove_Block_BitMap(dataBlock_Position); // 从块位图中移除数据块标记
		superblock.used_datablock--; // 减少已使用的数据块计数
	}

	// 检查该文件是否占用了多个 iNode（即是否有后续的 iNode）
	if (fileInode->nextInode_index != Nothing) {
		// 如果存在下一个 iNode，递归清理下一个 iNode 关联的数据块位图
		Clear_File_Block_Bitmap(fileInode->nextInode_index);
	}

	return;
}

// 清空指定文件所占用的数据块
void Clear_File_DataBlock(unsigned int current_file_inode_index) {

	// 获取指定文件的 iNode 信息
	iNode* fileInode = GetInode(current_file_inode_index);

	Block new_datablock;
	// 遍历文件所占用的所有数据块，将其清空
	for (int i = 0; i < fileInode->block_num; i++) {
		unsigned int block_position = fileInode->blockPointer[i];
		// 计算数据块在磁盘中的实际位置
		block_position = GetDataBlockPosition(block_position);
		// 保存空的数据块到磁盘，覆盖原有的数据
		new_datablock.saveBlockData(block_position);
	}

	// 如果文件占用多个 iNode，递归清空后续 iNode 关联的数据块
	if (fileInode->nextInode_index != Nothing) {
		Clear_File_DataBlock(fileInode->nextInode_index);
	}

	return;
}

// 删除指定的普通文件
void Delete_File(unsigned int current_file_inode_index) {
	// 清空文件所占用的数据块
	Clear_File_DataBlock(current_file_inode_index);

	// 更新父目录的信息，移除该文件的 iNode 引用
	Update_info_Remove(current_file_inode_index);

	// 清理文件所占用的块位图，释放数据块
	Clear_File_Block_Bitmap(current_file_inode_index);

	// 清理文件的 iNode 位图，释放 iNode
	Clear_File_INode_Bitmap(current_file_inode_index);

	// 重置文件的 iNode 信息，清除文件元数据
	iNode* fileInode = GetInode(current_file_inode_index);
	fileInode->reset_info();

	// 保存文件系统的元数据到磁盘，确保数据一致性
	Save_Simdisk();

	return;
}

// 删除指定的空目录
void Remove_Empty_Directory(unsigned int current_directory_Inode_index) {
	// 获取当前目录的 iNode 信息
	iNode* dirInode = GetInode(current_directory_Inode_index);

	// 清空目录的数据块内容
	unsigned int dataBlock_Position = dirInode->blockPointer[0]; // 获取目录的第一个数据块索引
	dataBlock_Position = GetDataBlockPosition(dataBlock_Position); // 计算数据块在磁盘中的实际位置
	Block emptyBlock; // 创建一个空的块，用于覆盖原有的数据

	emptyBlock.saveBlockData(dataBlock_Position); // 将空块写入磁盘，清空数据块内容

	// 更新父目录的信息，移除该目录的 iNode 引用
	Update_info_Remove(current_directory_Inode_index);

	// 更新块位图，释放该目录占用的数据块
	block_bitmap.Remove_Block_BitMap(dataBlock_Position);
	superblock.used_datablock--; // 减少已使用的数据块计数

	// 更新 iNode 位图，释放该目录的 iNode
	inode_bitmap.Remove_Inode_Bitmap(current_directory_Inode_index);
	superblock.used_inode--; // 减少已使用的 iNode 计数

	// 重置该目录的 iNode 信息，清除元数据
	dirInode->reset_info();

	// 保存文件系统的元数据到磁盘，确保数据一致性
	Save_Simdisk();
}

// 递归删除指定的目录及其子目录和文件
void Remove_Directory(unsigned int dirInode_index) {
	// 获取指定目录的 iNode 信息
	iNode* dirInode = GetInode(dirInode_index);

	// 判断文件类型，如果是普通文件，直接删除（递归的出口）
	if (!dirInode->isDirectoryType()) {
		Delete_File(dirInode_index); // 调用删除文件的函数
		return;
	}
	else {
		// 如果是目录类型
		if (dirInode->isDirectoryEmpty()) {
			// 如果目录为空，直接删除该空目录
			Remove_Empty_Directory(dirInode_index);
			return;
		}
		else {
			// 如果目录不为空，需要递归删除子目录和文件
			unsigned int dataBlock_Position = dirInode->blockPointer[0]; // 获取目录的数据块索引
			dataBlock_Position = GetDataBlockPosition(dataBlock_Position); // 计算数据块在磁盘中的实际位置

			Block Directory_Block;
			Directory_Block.loadBlockData(dataBlock_Position); // 加载目录的数据块

			// 递归删除子文件和子目录
			while (!dirInode->isDirectoryEmpty()) {
				Directory_Block.loadBlockData(dataBlock_Position); // 重新加载数据块，因为数据可能已更新
				unsigned int child_File_Inode_index = Directory_Block.data[0]; // 获取第一个子文件的 iNode 索引
				Remove_Directory(child_File_Inode_index); // 递归删除子文件或子目录
			}

			// 当目录已清空，删除该空目录
			Remove_Empty_Directory(dirInode_index);
		}
	}
}

// 在文件系统内部将一个文件复制到另一个位置
void Transfer_File_Within_Simdisk(unsigned int src_inode_index, unsigned int dest_inode_index) {

	// 文件系统内部的二进制文件拷贝，将 src_inode 复制到 dest_inode 上
	// 获取源文件和目标文件的 iNode 指针
	iNode* srcInode = GetInode(src_inode_index);
	iNode* destInode = GetInode(dest_inode_index);

	// 使目标文件的占用数据块数量与源文件一致
	destInode->block_num = srcInode->block_num;

	// 复制文件名到目标文件的 iNode
	strcpy_s(destInode->file_name, NameLength, srcInode->file_name);

	// 复制具体的数据块内容
	Block data_block;
	for (int i = 0; i < srcInode->block_num; i++) {
		// 读取源文件的数据块
		unsigned int block_position = srcInode->blockPointer[i];
		block_position = dataBlock_begin + block_position; // 获取数据块在磁盘中的实际位置

		data_block.loadBlockData(block_position); // 加载源数据块内容

		// 分配新的数据块给目标文件
		unsigned int new_block_index = Get_FreeBlockIndex();  // 获取一个空闲的数据块索引
		destInode->blockPointer[i] = new_block_index;         // 更新目标文件的 iNode 表

		// 将数据块内容写入新的数据块位置
		data_block.saveBlockData(dataBlock_begin + new_block_index);
	}

	// 如果源文件占用了多个 iNode（链式 iNode），递归复制
	if (srcInode->nextInode_index != Nothing) {
		// 分配新的 iNode 给目标文件的下一个部分
		unsigned int next_inode_index = Get_FreeINodeIndex();  // 获取一个空闲的 iNode 索引
		iNode* nextInode = GetInode(next_inode_index);
		nextInode->clear_info();  // 清空新分配的 iNode 信息

		destInode->nextInode_index = next_inode_index; // 更新目标文件的 nextInode_index

		// 递归复制下一个 iNode
		Transfer_File_Within_Simdisk(srcInode->nextInode_index, destInode->nextInode_index);
	}

	return;
}

// 将主机上的文件加载到缓存中，以便在文件系统中使用
void load_HostFile_To_Cache(string hostpath) {
	// 将要拷贝的文件放到缓冲区中

	// 清空缓冲区 blockCache，确保缓存中没有残留的数据
	while (!blockCache.empty()) {
		blockCache.pop(); // 移除队列中的所有块
	}

	// 获取文件大小，计算需要的完整磁盘块数和总块数
	unsigned int filesize = 0, full_need_block = 0, total_need_block = 0;
	file_IO.open(hostpath, ios::in | ios::binary); // 以二进制读模式打开主机文件
	file_IO.seekg(0, ios::end);      // 将文件指针移动到文件末尾
	filesize = file_IO.tellg();      // 获取文件大小（字节数）

	full_need_block = filesize / sizeof(Block); // 计算完整的块数
	if (filesize % sizeof(Block) == 0) {
		total_need_block = full_need_block;
	}
	else {
		total_need_block = full_need_block + 1; // 如果有剩余部分，块数加 1
	}
	file_IO.seekg(0, ios::beg); // 重置文件指针到文件开始

	// 读取完整的磁盘块并存入缓存
	Block full_block;
	for (unsigned int i = 0; i < full_need_block; i++) {
		full_block.copyBlockData(256); // 读取完整的块数据（256 个 unsigned int）
		blockCache.push(full_block);   // 将块加入缓存队列
	}

	// 读取剩余的数据（不足一个块）并存入缓存
	Block remain_block;
	unsigned int remain_size = filesize % sizeof(Block); // 剩余的数据大小（字节数）
	unsigned int need_int = remain_size / sizeof(unsigned int); // 需要读取的 unsigned int 个数
	unsigned int need_char = remain_size % sizeof(unsigned int); // 剩余的字节数

	remain_block.copyBlockData(need_int); // 读取剩余的 unsigned int 数据

	if (need_char != 0) {
		char chars[4] = { 0 }; // 创建一个字符数组，大小为 4 字节，初始化为 0
		file_IO.read(chars, need_char); // 读取剩余的字符数据

		// 将字符组合成一个 unsigned int
		unsigned int value = 0;
		for (unsigned int i = 0; i < need_char; i++) {
			value |= (static_cast<unsigned int>(static_cast<unsigned char>(chars[i])) << ((3 - i) * 8));
		}

		remain_block.data[need_int] = value; // 将组合后的 unsigned int 存入 remain_block
	}

	blockCache.push(remain_block); // 将剩余的数据块加入缓存

	// 关闭文件
	file_IO.close();
}

// 将缓冲区的数据追加写入文件系统
void Write_Cache_To_Simdisk(unsigned int file_inode_index) {

	// 原文件可能占用了多个 iNode，需要找到最后一个 iNode，在其后续添加数据
	iNode* fileInode = GetInode(file_inode_index);
	while (fileInode->nextInode_index != Nothing) {
		// 移动到下一个 iNode
		file_inode_index = fileInode->nextInode_index;
		fileInode = GetInode(file_inode_index);
	}

	// 开始将缓冲区的数据写入文件系统
	Block dataBlock;
	while (!blockCache.empty()) { // 当缓冲区不为空时
		// 判断当前 iNode 的数据块指针是否已满，如果已满，需要申请新的 iNode 并链接
		if (fileInode->block_num == MAX_BLOCK_POINTER) { // 当前 iNode 已经满了
			unsigned int next_inode_index = Get_FreeINodeIndex(); // 获取新的空闲 iNode 索引
			iNode* nextInode = GetInode(next_inode_index);

			nextInode->clear_info(); // 清空新 iNode 的信息
			fileInode->nextInode_index = next_inode_index; // 将新 iNode 链接到当前文件的 iNode 链表中

			// 递归调用，将剩余的数据写入新的 iNode
			Write_Cache_To_Simdisk(next_inode_index);
			return; // 返回，结束当前递归
		}

		// 从缓冲区中获取数据块
		dataBlock = blockCache.front();  // 取出队首的数据块
		blockCache.pop();                // 弹出队首的数据块

		// 分配新的数据块索引
		unsigned int free_block_index = Get_FreeBlockIndex(); // 获取一个空闲的数据块索引
		// 将数据块写入磁盘中的数据块区域
		dataBlock.saveBlockData(free_block_index + dataBlock_begin);

		// 更新当前 iNode 的信息
		unsigned int index = fileInode->block_num; // 获取当前 iNode 已使用的数据块数量
		fileInode->block_num++;                    // 增加数据块数量
		fileInode->blockPointer[index] = free_block_index; // 记录新分配的数据块索引
	}

	return;
}
