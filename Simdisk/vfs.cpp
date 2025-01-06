#include "vfs.h"

// 检查是否可以进行拷贝操作，如果资源不足，向 shell 端发送错误信息
bool send_Copy_Error_Information(unsigned int need_block, unsigned int need_inode) {
	bool flag = true;  // 标记是否满足拷贝条件

	// 判断是否有足够的数据块
	if (superblock.used_datablock + need_block > superblock.total_datablock) {
		// 数据块数量不足，拷贝失败
		string message_to_shell = "  数据块已经用尽，拷贝失败!\n";
		Deliver_Message_To_Shell(message_to_shell);  // 向 shell 端发送错误信息
		flag = false;
	}

	// 判断是否有足够的 iNode
	if (superblock.used_inode + need_inode > superblock.total_inode) {
		// iNode 数量不足，拷贝失败
		string message_to_shell = "  iNode已经用尽，拷贝失败!\n";
		Deliver_Message_To_Shell(message_to_shell);  // 向 shell 端发送错误信息
		flag = false;
	}

	return flag;  // 返回是否可以拷贝
}

// 解释和执行用户输入的命令
bool Command_Interpreter() {
	// 解析用户输入的命令
	Parse_Command(command);

	// 根据命令类型执行相应操作
	if (command.type == 0) {  // Exit 命令
		Deliver_Message_To_Shell("  正在退出文件系统...\n");
		return false;  // 返回 false，结束文件系统运行
	}
	else if (command.type == 1) {  // info 命令
		Info();  // 显示文件系统的基本信息
	}
	else if (command.type == 2) {  // cd 命令
		Cd();  // 改变工作目录
	}
	else if (command.type == 3) {  // dir 命令
		Dir();  // 展示目录内容
	}
	else if (command.type == 4) {  // md 命令
		Md();  // 创建目录
	}
	else if (command.type == 5) {  // rd 命令
		Rd();  // 删除目录
	}
	else if (command.type == 6) {  // newfile 命令
		Newfile();  // 创建新文件
	}
	else if (command.type == 7) {  // cat 命令
		Cat();  // 查看文件内容
	}
	else if (command.type == 8) {  // copy 命令
		Copy();  // 拷贝文件或目录
	}
	else if (command.type == 9) {  // del 命令
		Del();  // 删除文件
	}
	else if (command.type == 10) {  // check 命令
		Check();  // 检查并修复文件一致性
	}
	else if (command.type == 11) {  // ls 命令
		Ls();  // 列出当前目录内容
	}
	else if (command.type == 12) {  // help 命令
		Display_HelpMenu();  // 显示帮助菜单
	}
	else {
		// 如果命令无法识别，向 shell 端发送错误信息
		Deliver_Message_To_Shell("  命令错误, 请重新输入命令!...\n");
	}

	return true;  // 返回 true，继续文件系统运行
}

// 改变当前工作目录，输入为绝对路径
void Change_Directory(string absolute_path) {
	// 查找绝对路径对应的目录 iNode 编号
	unsigned int directory_inode_index = Search_File_INode(absolute_path);
	iNode* dirInode = GetInode(directory_inode_index); // 获取目录的 iNode 指针
	string message_to_shell;

	// 检查目录是否存在
	if (isPath_Exist(directory_inode_index)) {
		// 检查是否有读取权限
		if (!dirInode->isRead_Available()) {
			message_to_shell = "  该目录不可读!\n"; // 如果无权限，提示不可读
		}
		else {
			Current_Directory_iNode = directory_inode_index; // 更新当前工作目录的 iNode
			message_to_shell = "  cd命令执行成功!\n"; // 提示切换成功
		}
	}

	else {
		message_to_shell = "  未找到该目录!\n"; // 如果目录不存在，提示未找到
	}
	
	// 将提示信息发送到 Shell
	Deliver_Message_To_Shell(message_to_shell);
	return;
}

// 输出指定文件的所有数据内容
void Output_File_Data(unsigned int currentFileInode_index) {
	Block dataBlock;

	// 使用 for 循环读取文件的所有数据块
	for (; currentFileInode_index != Nothing; ) {
		// 获取当前文件的 iNode
		iNode* fileInode = GetInode(currentFileInode_index);
		if (!fileInode) {
			Deliver_Message_To_Shell("错误：无效的 iNode 索引。\n");
			return;
		}

		// 遍历当前 iNode 下的所有数据块
		for (int blockIndex = 0; blockIndex < fileInode->block_num; blockIndex++) {
			unsigned int blockPosition = fileInode->blockPointer[blockIndex]; // 获取数据块位置
			blockPosition = GetDataBlockPosition(blockPosition);

			dataBlock.loadBlockData(blockPosition); // 加载数据块内容

			// 遍历数据块中的每个数据单元
			for (int byteIndex = 0; byteIndex < 256; byteIndex++) {
				std::string bytes(4, '\0'); // 初始化一个长度为 4 的字符串，填充为 '\0'

				// 提取 4 个字节并检查是否全部为空字符
				bool valid = ExtractBytes(dataBlock.data[byteIndex], bytes);
				if (valid) {
					// 如果字节有效，将其写入共享内存并发送到 Shell
					Deliver_Message_To_Shell(bytes);
				}
				else {
					// 如果无效（表示文件结束），写入换行符并退出
					Deliver_Message_To_Shell("\n");
					break;
				}
			}
		}

		// 更新为下一个 iNode
		currentFileInode_index = fileInode->nextInode_index;
	}
}

// 将虚拟文件系统中的文件复制到主机路径
void Copy_File_From_VFS_To_Host(unsigned int file_inode_index, string path) {
	Block dataBlock;  // 创建一个 Block 实例用于操作数据块
	iNode* fileInode = GetInode(file_inode_index);  // 根据 iNode 编号获取文件的 iNode

	// 构造主机文件的完整路径，包含文件名
	path = path + "\\" + fileInode->file_name;

	// 打开主机路径的输出文件流
	std::ofstream fout(path, std::ios::out | std::ios::binary);
	if (!fout) {
		Deliver_Message_To_Shell("错误：无法创建主机文件。\n");
		return;
	}

	// 使用 for 循环处理文件数据，支持多 iNode 文件
	for (; file_inode_index != Nothing; file_inode_index = fileInode->nextInode_index) {
		fileInode = GetInode(file_inode_index);
		if (!fileInode) {
			Deliver_Message_To_Shell("错误：无效的 iNode 索引。\n");
			fout.close();
			return;
		}

		// 遍历当前 iNode 的所有数据块
		for (int i = 0; i < fileInode->block_num; i++) {
			unsigned int blockposition = fileInode->blockPointer[i];  // 获取数据块编号
			blockposition = GetDataBlockPosition(blockposition);      // 转换为数据块物理位置

			// 加载数据块内容到内存
			dataBlock.loadBlockData(blockposition);

			// 遍历数据块中的每个单元（每单元含 4 字节数据）
			for (int byteIndex = 0; byteIndex < 256; byteIndex++) {
				std::string byte(4, '\0');  // 初始化长度为 4 的字符串

				// 提取单元中的 4 个字节数据，检查是否有效
				bool valid = ExtractBytes(dataBlock.data[byteIndex], byte);
				if (valid) {
					// 如果有效，将字节数据写入主机文件
					fout.write(byte.c_str(), 4);
				}
				else {
					// 如果无效（是文件末尾），停止写入
					break;
				}
			}
		}
	}

	// 关闭输出文件流，完成文件写入
	fout.close();

	// 向 Shell 发送成功信息
	Deliver_Message_To_Shell("  CopyHost 命令执行成功!\n");
	return;
}

// 将主机文件复制到虚拟文件系统中
void copy_File_From_Host_To_Simdisk(string filename, string hostpath, unsigned int directory_inode_index) {
	// 将主机文件的数据加载到缓冲区
	load_HostFile_To_Cache(hostpath);

	// 计算拷贝所需的数据块数量和 iNode 数量
	unsigned int need_block = blockCache.size();  // 所需的数据块数
	unsigned int need_inode = need_block / MAX_BLOCK_POINTER;  // 每个 iNode 最多支持 MAX_BLOCK_POINTER 数据块
	if (need_block % MAX_BLOCK_POINTER != 0) {
		need_inode++;  // 如果有剩余数据块，增加一个 iNode
	}

	// 检查虚拟文件系统是否有足够的资源，如果不足，发送错误信息并停止操作
	if (!send_Copy_Error_Information(need_block, need_inode)) {
		return;  // 资源不足，拷贝失败
	}

	// 在目标目录下创建一个新文件
	unsigned int new_inode = Create_New_File(directory_inode_index, filename);

	// 将缓冲区中的数据块写入虚拟文件系统
	Write_Cache_To_Simdisk(new_inode);

	// 向 Shell 发送成功信息
	string message_to_shell = "  拷贝命令执行成功!\n";
	Deliver_Message_To_Shell(message_to_shell);

	// 保存文件系统的最新状态
	Save_Simdisk();

	// 清空缓冲区，释放内存
	while (!blockCache.empty()) {
		blockCache.pop();
	}

	return;
}

// 在虚拟文件系统 (VFS) 内部进行文件拷贝
void Copy_within_VFS(const string& filename, unsigned int file_inode_index, unsigned int directory_inode_index) {
	// 统计所需的数据块和 iNode 数量
	unsigned int need_block = 0, need_inode = 0;

	// 计算文件拷贝所需的资源
	if (!Calculate_Required_Resources(file_inode_index, need_block, need_inode)) {
		Deliver_Message_To_Shell("错误：无法计算所需资源。\n");
		return;
	}

	// 检查文件系统是否有足够的资源
	if (!send_Copy_Error_Information(need_block, need_inode)) {
		Deliver_Message_To_Shell("错误：资源不足，无法完成拷贝操作。\n");
		return;
	}

	// 创建新文件并返回其 iNode 编号
	unsigned int new_inode_index = Create_New_File(directory_inode_index, filename);
	if (new_inode_index == Nothing) {
		Deliver_Message_To_Shell("错误：创建新文件失败。\n");
		return;
	}

	// 执行文件内容的复制
	Transfer_File_Within_Simdisk(file_inode_index, new_inode_index);

	// 向 Shell 端发送成功信息
	Deliver_Message_To_Shell("拷贝命令执行成功！\n");

	// 保存文件系统的最新状态
	Save_Simdisk();
}

// 计算文件拷贝所需的资源
bool Calculate_Required_Resources(unsigned int file_inode_index, unsigned int& need_block, unsigned int& need_inode) {
	need_block = 0;
	need_inode = 0;

	// 遍历文件链表，统计资源需求
	for (unsigned int current_inode_index = file_inode_index; current_inode_index != Nothing;) {
		// 使用 GetInode 函数获取当前 iNode
		iNode* current_inode = GetInode(current_inode_index);
		if (current_inode == nullptr) {
			return false; // 无效的 iNode 索引，返回失败
		}

		// 累计资源需求
		need_block += current_inode->block_num;
		need_inode++;

		// 获取下一个 iNode 索引
		current_inode_index = current_inode->nextInode_index;
	}

	return true; // 成功统计资源需求
}

// 初始化文件系统
bool Initial_FileSystem() {
	string message_to_shell;

	// 检查是否可以打开已有的虚拟文件系统
	bool flag = Open_Simdisk();

	// 如果虚拟文件系统不存在，则创建新的文件系统
	if (!flag) {
		Create_Simdisk();  // 创建新的文件系统
	}

	// 向 Shell 发送进入文件系统的提示信息
	message_to_shell = "  正在进入YDFS文件系统...\n";
	Deliver_Message_To_Shell(message_to_shell);

	// 加载虚拟文件系统到内存
	Load_Simdisk();

	return true;  // 返回 true 表示初始化成功
}

// 显示文件系统的基本信息，包括磁盘使用情况和结构信息
void display_Simdisk_Information() {
	iNode* rootInode = GetInode(0);  // 获取根目录的 iNode
	unsigned int total_used_size = CalculateFileSize(rootInode);  // 计算已使用的磁盘空间（字节）
	total_used_size = total_used_size / BLOCK_SIZE;  // 转换为磁盘块数量
	unsigned int free_size = 100 * BLOCK_SIZE - total_used_size;  // 计算剩余磁盘空间（块）

	// 定义输出信息
	string message_to_shell[30];
	message_to_shell[0] = "--------------------------------------------------------------------------------\n";
	message_to_shell[1] = "YDFS                                                                 Information\n";
	send_Messages(message_to_shell, 2);
	message_to_shell[0] = "--------------------------------------------------------------------------------\n\n";
	message_to_shell[1] = "文件系统信息:\n";
	message_to_shell[2] = "    总磁盘空间:                            100MB\n";
	message_to_shell[3] = "    磁盘块大小:                            " + to_string(BLOCK_SIZE) + "B\n";
	message_to_shell[4] = "    iNode大小:                             " + to_string(INODE_SIZE) + "B\n";
	message_to_shell[5] = "    总磁盘块数:                            " + to_string(TOTAL_BLOCK_NUM) + "\n";
	message_to_shell[6] = "    引导块数:                              " + to_string(BOOTBLOCK_NUM) + "\n";
	message_to_shell[7] = "    超级块数:                              " + to_string(SUPERBLOCK_NUM) + "\n";
	message_to_shell[8] = "    组描述块所占磁盘块数量:                " + to_string(group_desciption.groupDescriptionSize()) + "\n";
	message_to_shell[9] = "    数据块位图所占磁盘块数量:              " + to_string(group_desciption.blockBitMapSize()) + "\n";
	message_to_shell[10] = "    iNode位图所占磁盘块数量:               " + to_string(group_desciption.iNodeBitMapSize()) + "\n";
	message_to_shell[11] = "    iNode表所占磁盘块数量:                 " + to_string(bootblock.iNodeArrayNum()) + "\n";
	message_to_shell[12] = "    数据块数量:                            " + to_string(bootblock.BlockNum()) + "\n";
	message_to_shell[13] = "\n";
	message_to_shell[14] = "    已使用的iNode数量:                     " + to_string(superblock.used_inode) + "\n";
	message_to_shell[15] = "    已使用的数据块数量:                    " + to_string(superblock.used_datablock) + "\n";
	message_to_shell[16] = "    已使用磁盘空间:                        " + to_string(total_used_size) + "KB\n";
	message_to_shell[17] = "    可使用磁盘空间:                        " + to_string(free_size) + "KB\n";
	message_to_shell[18] = "\n";

	// 将信息发送到 Shell
	send_Messages(message_to_shell, 19);
	return;
}

// 显示指定目录下所有子文件和子目录的信息
void DirectoryInformation(iNode* dirInode) {
	unsigned int block_position = dirInode->blockPointer[0];  // 获取目录数据块的位置
	block_position = GetDataBlockPosition(block_position);  // 转换为物理地址
	Block dataBlock;

	// 加载目录数据块
	dataBlock.loadBlockData(block_position);
	string message_to_shell;

	// 遍历目录中的每个文件和子目录
	for (int i = 0; i < dirInode->files_num; i++) {
		unsigned int file_index = dataBlock.data[i];  // 获取文件的 iNode 编号
		iNode* fileInode = GetInode(file_index);  // 获取文件对应的 iNode

		// 生成子文件/目录的信息字符串
		message_to_shell = SubFileInformation(fileInode);
		message_to_shell = "    " + message_to_shell;  // 格式化输出缩进
		Deliver_Message_To_Shell(message_to_shell);  // 发送信息到 Shell
	}
}

// 显示指定目录的信息，并可选显示其子目录和子文件的详细信息
void display_Directory_Information(unsigned int directory_inode_index, bool isShowSubDirecotry = false) {
	// 获取目录的 iNode
	iNode* dirInode = GetInode(directory_inode_index);

	// 检查目录是否具有读取权限
	if (!dirInode->isRead_Available()) {
		string message_to_shell = "  执行失败: 该目录不可读!\n";
		Deliver_Message_To_Shell(message_to_shell);  // 向 Shell 发送错误信息
		return;  // 直接返回
	}

	// 显示目录的基本信息
	string message_to_shell[10];
	message_to_shell[0] = "   目录名:    " + string(dirInode->file_name) + "\n";
	message_to_shell[1] = "   所占块的位置:    " + to_string(GetDataBlockPosition(dirInode->blockPointer[0])) + "\n";
	message_to_shell[2] = "   目录长度:        " + to_string(CalculateFileSize(dirInode)) + "B\n\n";

	// 将信息发送到 Shell
	send_Messages(message_to_shell, 3);

	// 更新当前目录 iNode
	Current_Directory_iNode = directory_inode_index;

	// 如果不需要显示子目录和子文件的信息，直接返回
	if (!isShowSubDirecotry) return;

	// 显示子目录和子文件的数量
	message_to_shell[0] = "    子目录/子文件的数量:    " + to_string(dirInode->files_num) + "\n";

	// 设置子文件/子目录的信息表头
	SetMessageHeader(message_to_shell[1]);
	message_to_shell[1] = "    " + message_to_shell[1];  // 添加缩进，格式化输出

	// 将子目录/子文件数量和表头信息发送到 Shell
	send_Messages(message_to_shell, 2);

	// 显示目录中的所有子文件和子目录的信息
	DirectoryInformation(dirInode);

	// 显示完毕后插入换行
	Deliver_Message_To_Shell("\n\n");

	return;
}


// 显示指定目录的树形结构
// 使用缩进展示子目录层级关系
void DisplayDirectoryTree(iNode* dirInode, int indentLevel = 0) {
	// 根据层级生成缩进字符串
	std::string indent(indentLevel * 8, ' '); // 每一级增加 8 个空格

	// 获取并显示当前目录或文件的信息
	std::string dirInfo = indent + SubFileInformation(dirInode);
	Deliver_Message_To_Shell(dirInfo);

	// 检查当前 iNode 是否是目录类型
	if (dirInode->isDirectoryType()) {
		// 获取目录数据块的位置并加载数据块内容
		unsigned int blockPosition = dirInode->blockPointer[0];
		blockPosition = GetDataBlockPosition(blockPosition);
		Block dirBlock;
		dirBlock.loadBlockData(blockPosition);

		// 遍历目录下的所有子文件或子目录
		for (unsigned int i = 0; i < dirInode->files_num; ++i) {
			unsigned int child_inode_index = dirBlock.data[i];
			iNode* childInode = GetInode(child_inode_index);

			// 递归调用，增加缩进级别
			DisplayDirectoryTree(childInode, indentLevel + 1);
		}
	}
}

// 显示指定目录的树形结构，不带缩进
// 子目录不显示层级关系
void DisplayDirectoryTree(iNode* dirInode, string s) {
	// 获取并显示当前目录或文件的信息
	std::string dirInfo = SubFileInformation(dirInode);
	Deliver_Message_To_Shell(dirInfo);

	// 检查当前 iNode 是否是目录类型
	if (dirInode->isDirectoryType()) {
		// 获取目录数据块的位置并加载数据块内容
		unsigned int blockPosition = dirInode->blockPointer[0];
		blockPosition = GetDataBlockPosition(blockPosition);
		Block dirBlock;
		dirBlock.loadBlockData(blockPosition);

		// 遍历目录下的所有子文件或子目录
		for (unsigned int i = 0; i < dirInode->files_num; ++i) {
			unsigned int child_inode_index = dirBlock.data[i];
			iNode* childInode = GetInode(child_inode_index);

			// 递归调用，无缩进
			DisplayDirectoryTree(childInode, "");
		}
	}
}

// 显示当前目录的绝对路径
void Display_CurrentDirectoryPath() {
	string absolutePath = "";  // 用于存储当前目录的绝对路径

	// 调用函数获取当前目录的绝对路径
	absolutePath = Get_DirectoryAbsolutePath();

	// 确保路径以 '/' 开头
	if (absolutePath[0] != '/') {
		absolutePath = "/" + absolutePath;
	}

	// 格式化路径显示，增加提示符
	absolutePath = "  [@" + std::string(Current_User)+ "]  " + absolutePath + "># ";

	// 将路径信息发送到 Shell
	Deliver_Message_To_Shell(absolutePath);

	return;
}


//IPC

//basic function
//借鉴数据库中的二级封锁协议

// 显示文件系统的基本信息
void Info() {
	// 加共享 S 锁，确保读取操作的安全性
	rw_Controller.Set_Reader_SLock();

	// 加载文件系统的最新状态到内存
	Load_Simdisk();

	// 显示文件系统的基本信息，包括磁盘使用情况和结构信息
	display_Simdisk_Information();

	// 释放共享 S 锁
	rw_Controller.Release_Reader_SLock();

	return;
}

// 改变当前工作目录 (cd 命令)
void Cd() {
	// 加共享 S 锁，确保读取操作的安全性
	rw_Controller.Set_Reader_SLock();

	// 加载文件系统的最新状态到内存
	Load_Simdisk();

	// 将用户输入的路径转化为绝对路径
	Change_To_AbsolutePath(command.cmd[1]);

	// 改变当前目录到指定的绝对路径
	Change_Directory(command.cmd[1]);

	// 释放共享 S 锁
	rw_Controller.Release_Reader_SLock();

	return;
}

// 显示目录内容（支持当前目录和指定目录，可选显示子目录）
void Dir() {
	rw_Controller.Set_Reader_SLock(); // 设置读锁，确保读取操作安全

	Load_Simdisk(); // 加载文件系统的最新状态，确保数据一致性

	int count = command.cmd_count; // 获取命令参数的数量
	string input_cmd[3];
	for (int i = 0; i < 3; i++) {
		input_cmd[i] = command.cmd[i];
	}

	// 根据命令参数数量和内容判断执行的功能
	if (count == 1) {
		// 命令: dir
		display_Directory_Information(Current_Directory_iNode, false); // 显示当前目录信息，不显示子目录
	}
	else if (count == 2 && input_cmd[1] == "s") {
		// 命令: dir s
		display_Directory_Information(Current_Directory_iNode, true); // 显示当前目录及其子目录
	}
	else if (count == 2 && input_cmd[1] != "s") {
		// 命令: dir path
		Change_To_AbsolutePath(input_cmd[1]); // 将路径转换为绝对路径
		unsigned int directory_inode_index = Search_File_INode(input_cmd[1]); // 搜索路径对应的 iNode

		if (isPath_Exist(directory_inode_index)) {
			display_Directory_Information(directory_inode_index, false); // 显示指定目录的信息，不显示子目录
		}
	}
	else if (count == 3 && input_cmd[2] == "s") {
		// 命令: dir path s
		Change_To_AbsolutePath(input_cmd[1]); // 将路径转换为绝对路径
		unsigned int directory_inode_index = Search_File_INode(input_cmd[1]); // 搜索路径对应的 iNode

		if (isPath_Exist(directory_inode_index)) {
			display_Directory_Information(directory_inode_index, true); // 显示指定目录及其子目录的信息
		}
	}
	else {
		Deliver_Message_To_Shell(" 命令错误!\n"); // 提示用户命令输入有误
	}

	rw_Controller.Release_Reader_SLock(); // 释放读锁
	return;
}

// 创建新目录（支持当前目录或指定路径下创建）
void Md() {
	rw_Controller.Set_Writer_XLock(); // 设置写锁，确保写操作安全
	string message_to_shell; // 用于存储发送到 Shell 的消息

	Load_Simdisk(); // 加载文件系统的最新状态，确保数据一致性

	// 辅助函数，用于发送消息到 Shell 并释放写锁
	auto deliverMessage = [&](const string& msg) {
		message_to_shell = msg; // 设置消息内容
		Deliver_Message_To_Shell(message_to_shell); // 发送消息
		rw_Controller.Release_Writer_XLock(); // 释放写锁
	};

	if (command.cmd_count == 3) {
		// 命令: md dirname 0/1/2
		if (is_File_Exist(Current_Directory_iNode, command.cmd[1])) {
			deliverMessage("  执行失败: 该目录已存在!\n");
			return;
		}

		iNode* currentInode = GetInode(Current_Directory_iNode);
		if (!currentInode->isWrite_Available()) {
			deliverMessage("  执行失败: 该路径无权被写入!\n");
			return;
		}

		Current_Permission = Parse_Permission(command.cmd[2]); // 设置目录权限
		Create_New_Directory(Current_Directory_iNode, command.cmd[1]); // 创建新目录
		deliverMessage("  md 命令执行成功!\n");
	}
	else {
		// 命令: md dirname path 0/1/2
		Change_To_AbsolutePath(command.cmd[2]); // 将路径转换为绝对路径
		unsigned int directory_inode_index = Search_File_INode(command.cmd[2]); // 搜索路径对应的 iNode

		if (isPath_Exist(directory_inode_index)) {
			iNode* dirInode = GetInode(directory_inode_index);

			if (!dirInode->isWrite_Available()) {
				deliverMessage("  执行失败: 该路径无权写入!\n");
				return;
			}

			if (is_File_Exist(directory_inode_index, command.cmd[1])) {
				deliverMessage("  执行失败: 该目录已存在!\n");
				return;
			}

			Current_Permission = Parse_Permission(command.cmd[3]); // 设置目录权限
			Create_New_Directory(directory_inode_index, command.cmd[1]); // 创建新目录
			deliverMessage("  md 命令执行成功!\n");
		}
		else {
			rw_Controller.Release_Writer_XLock(); // 释放写锁
		}
	}
}

// 删除目录 (rd path)
void Rd() {
	rw_Controller.Set_Writer_XLock(); // 设置写锁，确保删除操作安全
	Load_Simdisk(); // 加载最新的文件系统状态
	Change_To_AbsolutePath(command.cmd[1]); // 将用户输入的路径转换为绝对路径

	unsigned int directory_inode_index = Search_File_INode(command.cmd[1]); // 获取指定路径的 iNode
	if (directory_inode_index == Nothing) {
		Deliver_Message_To_Shell("  执行失败: 该路径不存在!\n"); // 提示路径不存在
		rw_Controller.Release_Writer_XLock(); // 释放写锁
		return;
	}

	iNode* dirInode = GetInode(directory_inode_index); // 获取目录对应的 iNode
	if (!dirInode->isDirectoryType()) {
		Deliver_Message_To_Shell("  该文件不是目录，删除请使用 del 命令\n"); // 非目录类型提示
		rw_Controller.Release_Writer_XLock(); // 释放写锁
		return;
	}

	if (directory_inode_index == 0 && Current_User_Id != 0) {
		Deliver_Message_To_Shell("   无管理员权限，无权删除根目录!\n"); // 检查根目录删除权限
		rw_Controller.Release_Writer_XLock(); // 释放写锁
		return;
	}

	if (!dirInode->isWrite_Available()) {
		Deliver_Message_To_Shell("  执行失败: 该路径无权写入!\n"); // 无写权限提示
		rw_Controller.Release_Writer_XLock(); // 释放写锁
		return;
	}

	unsigned int parentDir_index = dirInode->parent_directory_index; // 获取父目录索引

	if (!dirInode->isDirectoryEmpty()) {
		Deliver_Message_To_Shell(" 目录非空，是否继续删除? [[Y]/N]\n");
		Input_Data_From_ShareMemory(); // 获取用户输入
		string input_cmd = shell_to_simdisk.message[0];
		if (input_cmd == "y" || input_cmd == "Y" || input_cmd == "yes" || input_cmd == "YES") {
			Remove_Directory(directory_inode_index); // 删除非空目录
			Current_Directory_iNode = parentDir_index; // 更新当前路径
			Deliver_Message_To_Shell(" rd 命令执行成功!\n");
		}
		else {
			Deliver_Message_To_Shell(" 退出 rd 命令执行!\n");
		}
	}
	else {
		Remove_Directory(directory_inode_index); // 删除空目录
		Current_Directory_iNode = parentDir_index; // 更新当前路径
		Deliver_Message_To_Shell(" rd 命令执行成功!\n");
	}

	rw_Controller.Release_Writer_XLock(); // 释放写锁
}

// 创建新文件 (newfile filename [path] <0/1/2>)
void Newfile() {
	rw_Controller.Set_Writer_XLock(); // 设置写锁，确保写操作安全
	string message_to_shell; // 用于存储发送到 Shell 的消息

	Load_Simdisk(); // 加载最新的文件系统状态

	auto sendMessage = [&](const string& message) {
		message_to_shell = message;
		Deliver_Message_To_Shell(message_to_shell); // 发送消息到 Shell
	};

	if (command.cmd_count == 3) { // 在当前目录下新建文件
		iNode* currentDirInode = GetInode(Current_Directory_iNode);

		if (is_File_Exist(Current_Directory_iNode, command.cmd[1])) {
			sendMessage("  执行失败: 该文件已存在!\n"); // 文件已存在
		}
		else if (!currentDirInode->isWrite_Available()) {
			sendMessage("  执行失败: 该路径无权写入!\n"); // 无写权限
		}
		else {
			Current_Permission = Parse_Permission(command.cmd[2]); // 获取权限
			Create_New_File(Current_Directory_iNode, command.cmd[1]); // 创建文件
			sendMessage("  newfile 命令执行成功!\n"); // 成功提示
		}
	}
	else if (command.cmd_count == 4) { // 在指定路径下新建文件
		Change_To_AbsolutePath(command.cmd[2]); // 转换为绝对路径
		unsigned int directory_inode_index = Search_File_INode(command.cmd[2]);

		if (isPath_Exist(directory_inode_index)) {
			iNode* dirInode = GetInode(directory_inode_index);

			if (is_File_Exist(directory_inode_index, command.cmd[1])) {
				sendMessage("  执行失败: 该文件已存在!\n"); // 文件已存在
			}
			else if (!dirInode->isWrite_Available()) {
				sendMessage("  执行失败: 该路径无权写入!\n"); // 无写权限
			}
			else {
				Current_Permission = Parse_Permission(command.cmd[3]); // 获取权限
				Create_New_File(directory_inode_index, command.cmd[1]); // 创建文件
				sendMessage("  newfile 命令执行成功!\n");
			}
		}
	}
	else {
		sendMessage("  命令错误!\n");
	}

	rw_Controller.Release_Writer_XLock(); // 释放写锁
	return;
}

// 显示文件内容 (cat path)
void Cat() {
	rw_Controller.Set_Reader_SLock(); // 设置读锁，确保读取操作安全
	Load_Simdisk(); // 加载最新的文件系统状态
	string message_to_shell; // 用于存储发送到 Shell 的消息

	// 检查命令参数数量是否正确
	if (command.cmd_count != 2) {
		message_to_shell = "  命令错误!\n";
		Deliver_Message_To_Shell(message_to_shell);
		rw_Controller.Release_Reader_SLock(); // 释放读锁
		return;
	}

	string input_Path = command.cmd[1];
	Change_To_AbsolutePath(input_Path); // 将用户输入的路径转换为绝对路径
	unsigned int inode_index = Search_File_INode(input_Path); // 获取路径对应的 iNode
	iNode* fileInode = GetInode(inode_index);

	// 在确认 inode 存在之前释放读锁
	rw_Controller.Release_Reader_SLock();

	// 检查路径是否存在
	if (isPath_Exist(inode_index)) {
		// 检查是否为目录类型
		if (fileInode->isDirectoryType()) {
			Deliver_Message_To_Shell("  该路径是目录，无法使用 cat 命令!\n");
			return;
		}

		// 检查读取权限
		if (!fileInode->isRead_Available()) {
			message_to_shell = "  执行失败: 该路径无权读取!\n";
			Deliver_Message_To_Shell(message_to_shell);
			return;
		}

		rw_Controller.Set_Reader_SLock(); // 重新设置读锁，确保安全读取
		Output_File_Data(inode_index); // 读取文件内容并输出
		Current_Directory_iNode = fileInode->parent_directory_index; // 更新当前工作目录
	}

	rw_Controller.Release_Reader_SLock(); // 释放读锁
	return;
}

// 实现文件拷贝功能 (支持主机与模拟文件系统之间的拷贝，以及文件系统内部拷贝)
void Copy() {
	rw_Controller.Set_Writer_XLock(); // 设置写锁，确保写操作安全，防止其他线程或进程同时进行写操作

	Load_Simdisk(); // 加载最新的文件系统状态，确保文件系统数据一致
	Change_To_AbsolutePath(command.cmd[2]); // 将目标路径（第三个参数）转换为绝对路径，便于后续处理

	string message_to_shell; // 用于存储发送到 Shell 的消息，用于向用户反馈操作结果

	if (command.cmd[0] == "copy<host>") { // 处理主机与文件系统之间的拷贝操作
		if (command.cmd_count != 5) { // 判断参数数量是否正确
			message_to_shell = "  命令错误!\n";
			Deliver_Message_To_Shell(message_to_shell); // 发送错误消息到 Shell
			rw_Controller.Release_Writer_XLock(); // 释放写锁
			return; // 返回，终止当前命令
		}

		if (command.cmd[3] == "0") { // 从主机拷贝到文件系统
			string filename; // 用于存储从主机路径中提取的文件名
			const string& filepath = command.cmd[1]; // 主机文件的完整路径

			// 提取文件名：从路径中找到最后一个反斜杠后的部分
			for (size_t i = filepath.size(); i > 0; --i) {
				if (filepath[i - 1] == '\\') {
					filename = filepath.substr(i);
					break;
				}
			}

			// 查找目标目录在文件系统中的 inode 索引
			unsigned int dest_directory_inode_index = Search_File_INode(command.cmd[2]);

			if (isPath_Exist(dest_directory_inode_index)) { // 确认目标路径是否存在
				iNode* dirInode = GetInode(dest_directory_inode_index); // 获取目标目录的 inode 信息

				if (is_File_Exist(dest_directory_inode_index, filename)) { // 检查目标目录下是否已存在同名文件
					message_to_shell = "  执行失败: 该文件已存在!\n"; // 提示文件已存在
				}
				else if (!dirInode->isWrite_Available()) { // 检查目标目录是否有写权限
					message_to_shell = "  执行失败: 该路径无权写入!\n"; // 提示无写权限
				}
				else {
					Current_Permission = Parse_Permission(command.cmd[4]); // 设置文件的权限模式
					copy_File_From_Host_To_Simdisk(filename, command.cmd[1], dest_directory_inode_index); // 从主机拷贝文件到文件系统
					message_to_shell = "  文件成功拷贝至YDFS!\n"; // 提示操作成功
				}
			}
		}
		else if (command.cmd[3] == "1") { // 从文件系统拷贝到主机
			unsigned int src_inode_index = Search_File_INode(command.cmd[2]); // 查找源文件在文件系统中的 inode 索引

			if (isPath_Exist(src_inode_index)) { // 确认源文件路径是否存在
				iNode* srcInode = GetInode(src_inode_index); // 获取源文件的 inode 信息
				if (!srcInode->isRead_Available()) { // 检查源文件是否有读权限
					message_to_shell = "  执行失败: 该路径无权读取!\n"; // 提示无读权限
				}
				else {
					Copy_File_From_VFS_To_Host(src_inode_index, command.cmd[1]); // 从文件系统拷贝文件到主机
					message_to_shell = "  文件成功复制到主机!\n"; // 提示操作成功
				}
			}
		}
		else {
			message_to_shell = "  命令错误!\n"; // 提示命令格式错误
		}
	}
	else if (command.cmd[0] == "copy<ydfs>") { // 处理文件系统内部的拷贝操作
		if (command.cmd_count != 4) { // 判断参数数量是否正确
			message_to_shell = "  命令错误!\n";
			Deliver_Message_To_Shell(message_to_shell);
			rw_Controller.Release_Writer_XLock();
			return;
		}

		string src_filename; // 用于存储从源路径中提取的文件名
		const string& src_filepath = command.cmd[1]; // 源文件的完整路径

		// 提取源文件名：从路径中找到最后一个斜杠后的部分
		for (size_t i = src_filepath.size(); i > 0; --i) {
			if (src_filepath[i - 1] == '/') {
				src_filename = src_filepath.substr(i);
				break;
			}
		}

		Change_To_AbsolutePath(command.cmd[1]); // 将源路径转换为绝对路径
		unsigned int src_inode_index = Search_File_INode(command.cmd[1]); // 查找源文件的 inode 索引
		unsigned int dest_inode_index = Search_File_INode(command.cmd[2]); // 查找目标目录的 inode 索引
		iNode* srcInode = GetInode(src_inode_index); // 获取源文件的 inode 信息
		iNode* destInode = GetInode(dest_inode_index); // 获取目标目录的 inode 信息

		if (isPath_Exist(src_inode_index) || isPath_Exist(dest_inode_index)) { // 确认路径是否存在
			if (is_File_Exist(dest_inode_index, src_filename)) { // 检查目标目录下是否已存在同名文件
				message_to_shell = "  执行失败: 该文件已存在!\n"; // 提示文件已存在
			}
			else if (!srcInode->isRead_Available()) { // 检查源文件是否有读权限
				message_to_shell = "  执行失败: 该路径无权读取!\n"; // 提示无读权限
			}
			else if (!destInode->isWrite_Available()) { // 检查目标目录是否有写权限
				message_to_shell = "  执行失败: 该路径无权写入!\n"; // 提示无写权限
			}
			else {
				Current_Permission = Parse_Permission(command.cmd[3]); // 设置新文件的权限模式
				Copy_within_VFS(src_filename, src_inode_index, dest_inode_index); // 在文件系统内部拷贝文件
				message_to_shell = "  文件拷贝成功!\n"; // 提示操作成功
			}
		}
	}

	Deliver_Message_To_Shell(message_to_shell); // 将结果消息发送到 Shell
	rw_Controller.Release_Writer_XLock(); // 释放写锁，允许其他操作继续执行
	return; // 返回，结束函数
}

// 删除指定路径的文件 (支持普通文件删除，目录需使用 rd 命令)
void Del() {
	rw_Controller.Set_Writer_XLock(); // 设置写锁，确保删除操作的线程安全
	string message_to_shell; // 用于存储发送到 shell 的消息

	// 检查命令参数数量是否正确
	if (command.cmd_count != 2) {
		message_to_shell = "  命令错误!\n"; // 参数数量错误
		Deliver_Message_To_Shell(message_to_shell); // 发送错误消息到 shell
		rw_Controller.Release_Writer_XLock(); // 释放写锁
		return; // 结束函数
	}

	Load_Simdisk(); // 加载文件系统状态，确保数据一致性
	Change_To_AbsolutePath(command.cmd[1]); // 将指定路径转换为绝对路径
	unsigned int file_inode_index = Search_File_INode(command.cmd[1]); // 查找路径对应的 iNode 索引

	// 检查路径是否存在
	if (isPath_Exist(file_inode_index)) {
		iNode* fileInode = GetInode(file_inode_index); // 获取文件的 iNode 信息

		// 检查写权限
		if (!fileInode->isWrite_Available()) {
			message_to_shell = "  执行失败: 该路径无权写入!\n"; // 无写权限
		}
		// 检查是否是目录类型文件
		else if (fileInode->isDirectoryType() == true) {
			message_to_shell = "  该文件是目录，删除请用 rd 命令!\n"; // 目录需使用 rd 命令删除
		}
		// 删除普通文件
		else {
			unsigned int parentDirectory_index = fileInode->parent_directory_index; // 获取父目录索引
			Delete_File(file_inode_index); // 调用文件删除函数
			message_to_shell = "  Del 命令执行成功!\n"; // 删除成功消息
			Current_Directory_iNode = parentDirectory_index; // 更新当前工作目录为父目录
		}
	}

	Deliver_Message_To_Shell(message_to_shell); // 发送操作结果消息到 shell
	rw_Controller.Release_Writer_XLock(); // 释放写锁
	return; // 结束函数
}

// 检查文件系统的一致性并修复潜在的问题
void Check() {
	rw_Controller.Set_Writer_XLock(); // 设置写锁，确保检查操作的安全性，避免其他线程/进程同时写入

	// 向 shell 端发送消息，提示正在检查文件系统一致性
	Deliver_Message_To_Shell("  检查文件系统一致性...\n");

	// 调用一致性检查函数，判断文件系统状态
	bool flag = Compare_With_File();
	if (flag) {
		// 文件系统一致性正常，无需修复
		Deliver_Message_To_Shell("  文件一致性正常.....\n");
	}
	else {
		// 文件系统一致性异常，提示修复过程
		Deliver_Message_To_Shell("  文件一致性异常.....");
		Deliver_Message_To_Shell("  修复文件系统一致性中...\n");
	}

	Load_Simdisk(); // 加载文件系统，更新文件系统状态，确保一致性
	Save_Simdisk(); // 保存文件系统状态，将修复后的数据写回文件系统

	// 提示操作完成，并发送成功消息到 shell
	string message_to_shell = "  Check 命令执行成功!\n";
	Deliver_Message_To_Shell(message_to_shell);

	rw_Controller.Release_Writer_XLock(); // 释放写锁，允许其他写操作
	return; // 返回，结束函数
}

// 列出文件系统中的所有目录和文件信息
void Ls() {
	rw_Controller.Set_Reader_SLock(); // 设置读锁，确保读操作的线程安全

	Load_Simdisk(); // 加载文件系统，确保数据一致性

	string message_to_shell;
	SetMessageHeader(message_to_shell); // 设置列头信息（文件名、类型、权限等）
	Deliver_Message_To_Shell(message_to_shell); // 向 shell 端发送表头

	iNode* rootInode = GetInode(0); // 获取根目录的 iNode
	DisplayDirectoryTree(rootInode, ""); // 递归列出目录树中的所有文件和子目录信息

	rw_Controller.Release_Reader_SLock(); // 释放读锁，允许其他读写操作
	return; // 返回，结束函数
}

// 主运行函数：模拟文件系统的核心控制逻辑
void RunSimdisk() {
	string message_to_shell = ""; // 用于存储发送到 shell 的消息

	cout << "建立与 shell 端的连接中...." << endl;

	sharememory_manager.CreateShareMemory(hMapFileCommFlags, pCommFlags, CommFlagsFileName);
	pc_Controller->V(pc_Controller->getConnectSem()); // 通知 shell 端连接已建立

	message_to_shell = "用户: " + std::string(Current_User) +
		" (ID: " + std::to_string(Current_User_Id) + ") 欢迎来到 YDAI VFS!\n";
	Deliver_Message_To_Shell(message_to_shell); // 发送欢迎信息到 shell

	bool flag = Initial_FileSystem(); // 检查或初始化文件系统

	Display_HelpMenu(); // 显示命令提示
	Current_Directory_iNode = Root_Directory; // 将当前工作路径设为根目录

	while (true) {
		Display_CurrentDirectoryPath(); // 显示当前目录路径
		bool flag = Command_Interpreter(); // 执行用户输入的命令
		if (!flag) { // 如果接收到退出命令，跳出循环
			break;
		}
	}

	message_to_shell = "  退出命令执行成功!\n";
	Deliver_Message_To_Shell(message_to_shell); // 发送退出成功消息

	sharememory_manager.CloseShareMemory(hMapFileCommFlags, pCommFlags);
	return;
}



