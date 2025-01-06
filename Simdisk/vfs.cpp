#include "vfs.h"

// ����Ƿ���Խ��п��������������Դ���㣬�� shell �˷��ʹ�����Ϣ
bool send_Copy_Error_Information(unsigned int need_block, unsigned int need_inode) {
	bool flag = true;  // ����Ƿ����㿽������

	// �ж��Ƿ����㹻�����ݿ�
	if (superblock.used_datablock + need_block > superblock.total_datablock) {
		// ���ݿ��������㣬����ʧ��
		string message_to_shell = "  ���ݿ��Ѿ��þ�������ʧ��!\n";
		Deliver_Message_To_Shell(message_to_shell);  // �� shell �˷��ʹ�����Ϣ
		flag = false;
	}

	// �ж��Ƿ����㹻�� iNode
	if (superblock.used_inode + need_inode > superblock.total_inode) {
		// iNode �������㣬����ʧ��
		string message_to_shell = "  iNode�Ѿ��þ�������ʧ��!\n";
		Deliver_Message_To_Shell(message_to_shell);  // �� shell �˷��ʹ�����Ϣ
		flag = false;
	}

	return flag;  // �����Ƿ���Կ���
}

// ���ͺ�ִ���û����������
bool Command_Interpreter() {
	// �����û����������
	Parse_Command(command);

	// ������������ִ����Ӧ����
	if (command.type == 0) {  // Exit ����
		Deliver_Message_To_Shell("  �����˳��ļ�ϵͳ...\n");
		return false;  // ���� false�������ļ�ϵͳ����
	}
	else if (command.type == 1) {  // info ����
		Info();  // ��ʾ�ļ�ϵͳ�Ļ�����Ϣ
	}
	else if (command.type == 2) {  // cd ����
		Cd();  // �ı乤��Ŀ¼
	}
	else if (command.type == 3) {  // dir ����
		Dir();  // չʾĿ¼����
	}
	else if (command.type == 4) {  // md ����
		Md();  // ����Ŀ¼
	}
	else if (command.type == 5) {  // rd ����
		Rd();  // ɾ��Ŀ¼
	}
	else if (command.type == 6) {  // newfile ����
		Newfile();  // �������ļ�
	}
	else if (command.type == 7) {  // cat ����
		Cat();  // �鿴�ļ�����
	}
	else if (command.type == 8) {  // copy ����
		Copy();  // �����ļ���Ŀ¼
	}
	else if (command.type == 9) {  // del ����
		Del();  // ɾ���ļ�
	}
	else if (command.type == 10) {  // check ����
		Check();  // ��鲢�޸��ļ�һ����
	}
	else if (command.type == 11) {  // ls ����
		Ls();  // �г���ǰĿ¼����
	}
	else if (command.type == 12) {  // help ����
		Display_HelpMenu();  // ��ʾ�����˵�
	}
	else {
		// ��������޷�ʶ���� shell �˷��ʹ�����Ϣ
		Deliver_Message_To_Shell("  �������, ��������������!...\n");
	}

	return true;  // ���� true�������ļ�ϵͳ����
}

// �ı䵱ǰ����Ŀ¼������Ϊ����·��
void Change_Directory(string absolute_path) {
	// ���Ҿ���·����Ӧ��Ŀ¼ iNode ���
	unsigned int directory_inode_index = Search_File_INode(absolute_path);
	iNode* dirInode = GetInode(directory_inode_index); // ��ȡĿ¼�� iNode ָ��
	string message_to_shell;

	// ���Ŀ¼�Ƿ����
	if (isPath_Exist(directory_inode_index)) {
		// ����Ƿ��ж�ȡȨ��
		if (!dirInode->isRead_Available()) {
			message_to_shell = "  ��Ŀ¼���ɶ�!\n"; // �����Ȩ�ޣ���ʾ���ɶ�
		}
		else {
			Current_Directory_iNode = directory_inode_index; // ���µ�ǰ����Ŀ¼�� iNode
			message_to_shell = "  cd����ִ�гɹ�!\n"; // ��ʾ�л��ɹ�
		}
	}

	else {
		message_to_shell = "  δ�ҵ���Ŀ¼!\n"; // ���Ŀ¼�����ڣ���ʾδ�ҵ�
	}
	
	// ����ʾ��Ϣ���͵� Shell
	Deliver_Message_To_Shell(message_to_shell);
	return;
}

// ���ָ���ļ���������������
void Output_File_Data(unsigned int currentFileInode_index) {
	Block dataBlock;

	// ʹ�� for ѭ����ȡ�ļ����������ݿ�
	for (; currentFileInode_index != Nothing; ) {
		// ��ȡ��ǰ�ļ��� iNode
		iNode* fileInode = GetInode(currentFileInode_index);
		if (!fileInode) {
			Deliver_Message_To_Shell("������Ч�� iNode ������\n");
			return;
		}

		// ������ǰ iNode �µ��������ݿ�
		for (int blockIndex = 0; blockIndex < fileInode->block_num; blockIndex++) {
			unsigned int blockPosition = fileInode->blockPointer[blockIndex]; // ��ȡ���ݿ�λ��
			blockPosition = GetDataBlockPosition(blockPosition);

			dataBlock.loadBlockData(blockPosition); // �������ݿ�����

			// �������ݿ��е�ÿ�����ݵ�Ԫ
			for (int byteIndex = 0; byteIndex < 256; byteIndex++) {
				std::string bytes(4, '\0'); // ��ʼ��һ������Ϊ 4 ���ַ��������Ϊ '\0'

				// ��ȡ 4 ���ֽڲ�����Ƿ�ȫ��Ϊ���ַ�
				bool valid = ExtractBytes(dataBlock.data[byteIndex], bytes);
				if (valid) {
					// ����ֽ���Ч������д�빲���ڴ沢���͵� Shell
					Deliver_Message_To_Shell(bytes);
				}
				else {
					// �����Ч����ʾ�ļ���������д�뻻�з����˳�
					Deliver_Message_To_Shell("\n");
					break;
				}
			}
		}

		// ����Ϊ��һ�� iNode
		currentFileInode_index = fileInode->nextInode_index;
	}
}

// �������ļ�ϵͳ�е��ļ����Ƶ�����·��
void Copy_File_From_VFS_To_Host(unsigned int file_inode_index, string path) {
	Block dataBlock;  // ����һ�� Block ʵ�����ڲ������ݿ�
	iNode* fileInode = GetInode(file_inode_index);  // ���� iNode ��Ż�ȡ�ļ��� iNode

	// ���������ļ�������·���������ļ���
	path = path + "\\" + fileInode->file_name;

	// ������·��������ļ���
	std::ofstream fout(path, std::ios::out | std::ios::binary);
	if (!fout) {
		Deliver_Message_To_Shell("�����޷����������ļ���\n");
		return;
	}

	// ʹ�� for ѭ�������ļ����ݣ�֧�ֶ� iNode �ļ�
	for (; file_inode_index != Nothing; file_inode_index = fileInode->nextInode_index) {
		fileInode = GetInode(file_inode_index);
		if (!fileInode) {
			Deliver_Message_To_Shell("������Ч�� iNode ������\n");
			fout.close();
			return;
		}

		// ������ǰ iNode ���������ݿ�
		for (int i = 0; i < fileInode->block_num; i++) {
			unsigned int blockposition = fileInode->blockPointer[i];  // ��ȡ���ݿ���
			blockposition = GetDataBlockPosition(blockposition);      // ת��Ϊ���ݿ�����λ��

			// �������ݿ����ݵ��ڴ�
			dataBlock.loadBlockData(blockposition);

			// �������ݿ��е�ÿ����Ԫ��ÿ��Ԫ�� 4 �ֽ����ݣ�
			for (int byteIndex = 0; byteIndex < 256; byteIndex++) {
				std::string byte(4, '\0');  // ��ʼ������Ϊ 4 ���ַ���

				// ��ȡ��Ԫ�е� 4 ���ֽ����ݣ�����Ƿ���Ч
				bool valid = ExtractBytes(dataBlock.data[byteIndex], byte);
				if (valid) {
					// �����Ч�����ֽ�����д�������ļ�
					fout.write(byte.c_str(), 4);
				}
				else {
					// �����Ч�����ļ�ĩβ����ֹͣд��
					break;
				}
			}
		}
	}

	// �ر�����ļ���������ļ�д��
	fout.close();

	// �� Shell ���ͳɹ���Ϣ
	Deliver_Message_To_Shell("  CopyHost ����ִ�гɹ�!\n");
	return;
}

// �������ļ����Ƶ������ļ�ϵͳ��
void copy_File_From_Host_To_Simdisk(string filename, string hostpath, unsigned int directory_inode_index) {
	// �������ļ������ݼ��ص�������
	load_HostFile_To_Cache(hostpath);

	// ���㿽����������ݿ������� iNode ����
	unsigned int need_block = blockCache.size();  // ��������ݿ���
	unsigned int need_inode = need_block / MAX_BLOCK_POINTER;  // ÿ�� iNode ���֧�� MAX_BLOCK_POINTER ���ݿ�
	if (need_block % MAX_BLOCK_POINTER != 0) {
		need_inode++;  // �����ʣ�����ݿ飬����һ�� iNode
	}

	// ��������ļ�ϵͳ�Ƿ����㹻����Դ��������㣬���ʹ�����Ϣ��ֹͣ����
	if (!send_Copy_Error_Information(need_block, need_inode)) {
		return;  // ��Դ���㣬����ʧ��
	}

	// ��Ŀ��Ŀ¼�´���һ�����ļ�
	unsigned int new_inode = Create_New_File(directory_inode_index, filename);

	// ���������е����ݿ�д�������ļ�ϵͳ
	Write_Cache_To_Simdisk(new_inode);

	// �� Shell ���ͳɹ���Ϣ
	string message_to_shell = "  ��������ִ�гɹ�!\n";
	Deliver_Message_To_Shell(message_to_shell);

	// �����ļ�ϵͳ������״̬
	Save_Simdisk();

	// ��ջ��������ͷ��ڴ�
	while (!blockCache.empty()) {
		blockCache.pop();
	}

	return;
}

// �������ļ�ϵͳ (VFS) �ڲ������ļ�����
void Copy_within_VFS(const string& filename, unsigned int file_inode_index, unsigned int directory_inode_index) {
	// ͳ����������ݿ�� iNode ����
	unsigned int need_block = 0, need_inode = 0;

	// �����ļ������������Դ
	if (!Calculate_Required_Resources(file_inode_index, need_block, need_inode)) {
		Deliver_Message_To_Shell("�����޷�����������Դ��\n");
		return;
	}

	// ����ļ�ϵͳ�Ƿ����㹻����Դ
	if (!send_Copy_Error_Information(need_block, need_inode)) {
		Deliver_Message_To_Shell("������Դ���㣬�޷���ɿ���������\n");
		return;
	}

	// �������ļ��������� iNode ���
	unsigned int new_inode_index = Create_New_File(directory_inode_index, filename);
	if (new_inode_index == Nothing) {
		Deliver_Message_To_Shell("���󣺴������ļ�ʧ�ܡ�\n");
		return;
	}

	// ִ���ļ����ݵĸ���
	Transfer_File_Within_Simdisk(file_inode_index, new_inode_index);

	// �� Shell �˷��ͳɹ���Ϣ
	Deliver_Message_To_Shell("��������ִ�гɹ���\n");

	// �����ļ�ϵͳ������״̬
	Save_Simdisk();
}

// �����ļ������������Դ
bool Calculate_Required_Resources(unsigned int file_inode_index, unsigned int& need_block, unsigned int& need_inode) {
	need_block = 0;
	need_inode = 0;

	// �����ļ�����ͳ����Դ����
	for (unsigned int current_inode_index = file_inode_index; current_inode_index != Nothing;) {
		// ʹ�� GetInode ������ȡ��ǰ iNode
		iNode* current_inode = GetInode(current_inode_index);
		if (current_inode == nullptr) {
			return false; // ��Ч�� iNode ����������ʧ��
		}

		// �ۼ���Դ����
		need_block += current_inode->block_num;
		need_inode++;

		// ��ȡ��һ�� iNode ����
		current_inode_index = current_inode->nextInode_index;
	}

	return true; // �ɹ�ͳ����Դ����
}

// ��ʼ���ļ�ϵͳ
bool Initial_FileSystem() {
	string message_to_shell;

	// ����Ƿ���Դ����е������ļ�ϵͳ
	bool flag = Open_Simdisk();

	// ��������ļ�ϵͳ�����ڣ��򴴽��µ��ļ�ϵͳ
	if (!flag) {
		Create_Simdisk();  // �����µ��ļ�ϵͳ
	}

	// �� Shell ���ͽ����ļ�ϵͳ����ʾ��Ϣ
	message_to_shell = "  ���ڽ���YDFS�ļ�ϵͳ...\n";
	Deliver_Message_To_Shell(message_to_shell);

	// ���������ļ�ϵͳ���ڴ�
	Load_Simdisk();

	return true;  // ���� true ��ʾ��ʼ���ɹ�
}

// ��ʾ�ļ�ϵͳ�Ļ�����Ϣ����������ʹ������ͽṹ��Ϣ
void display_Simdisk_Information() {
	iNode* rootInode = GetInode(0);  // ��ȡ��Ŀ¼�� iNode
	unsigned int total_used_size = CalculateFileSize(rootInode);  // ������ʹ�õĴ��̿ռ䣨�ֽڣ�
	total_used_size = total_used_size / BLOCK_SIZE;  // ת��Ϊ���̿�����
	unsigned int free_size = 100 * BLOCK_SIZE - total_used_size;  // ����ʣ����̿ռ䣨�飩

	// ���������Ϣ
	string message_to_shell[30];
	message_to_shell[0] = "--------------------------------------------------------------------------------\n";
	message_to_shell[1] = "YDFS                                                                 Information\n";
	send_Messages(message_to_shell, 2);
	message_to_shell[0] = "--------------------------------------------------------------------------------\n\n";
	message_to_shell[1] = "�ļ�ϵͳ��Ϣ:\n";
	message_to_shell[2] = "    �ܴ��̿ռ�:                            100MB\n";
	message_to_shell[3] = "    ���̿��С:                            " + to_string(BLOCK_SIZE) + "B\n";
	message_to_shell[4] = "    iNode��С:                             " + to_string(INODE_SIZE) + "B\n";
	message_to_shell[5] = "    �ܴ��̿���:                            " + to_string(TOTAL_BLOCK_NUM) + "\n";
	message_to_shell[6] = "    ��������:                              " + to_string(BOOTBLOCK_NUM) + "\n";
	message_to_shell[7] = "    ��������:                              " + to_string(SUPERBLOCK_NUM) + "\n";
	message_to_shell[8] = "    ����������ռ���̿�����:                " + to_string(group_desciption.groupDescriptionSize()) + "\n";
	message_to_shell[9] = "    ���ݿ�λͼ��ռ���̿�����:              " + to_string(group_desciption.blockBitMapSize()) + "\n";
	message_to_shell[10] = "    iNodeλͼ��ռ���̿�����:               " + to_string(group_desciption.iNodeBitMapSize()) + "\n";
	message_to_shell[11] = "    iNode����ռ���̿�����:                 " + to_string(bootblock.iNodeArrayNum()) + "\n";
	message_to_shell[12] = "    ���ݿ�����:                            " + to_string(bootblock.BlockNum()) + "\n";
	message_to_shell[13] = "\n";
	message_to_shell[14] = "    ��ʹ�õ�iNode����:                     " + to_string(superblock.used_inode) + "\n";
	message_to_shell[15] = "    ��ʹ�õ����ݿ�����:                    " + to_string(superblock.used_datablock) + "\n";
	message_to_shell[16] = "    ��ʹ�ô��̿ռ�:                        " + to_string(total_used_size) + "KB\n";
	message_to_shell[17] = "    ��ʹ�ô��̿ռ�:                        " + to_string(free_size) + "KB\n";
	message_to_shell[18] = "\n";

	// ����Ϣ���͵� Shell
	send_Messages(message_to_shell, 19);
	return;
}

// ��ʾָ��Ŀ¼���������ļ�����Ŀ¼����Ϣ
void DirectoryInformation(iNode* dirInode) {
	unsigned int block_position = dirInode->blockPointer[0];  // ��ȡĿ¼���ݿ��λ��
	block_position = GetDataBlockPosition(block_position);  // ת��Ϊ�����ַ
	Block dataBlock;

	// ����Ŀ¼���ݿ�
	dataBlock.loadBlockData(block_position);
	string message_to_shell;

	// ����Ŀ¼�е�ÿ���ļ�����Ŀ¼
	for (int i = 0; i < dirInode->files_num; i++) {
		unsigned int file_index = dataBlock.data[i];  // ��ȡ�ļ��� iNode ���
		iNode* fileInode = GetInode(file_index);  // ��ȡ�ļ���Ӧ�� iNode

		// �������ļ�/Ŀ¼����Ϣ�ַ���
		message_to_shell = SubFileInformation(fileInode);
		message_to_shell = "    " + message_to_shell;  // ��ʽ���������
		Deliver_Message_To_Shell(message_to_shell);  // ������Ϣ�� Shell
	}
}

// ��ʾָ��Ŀ¼����Ϣ������ѡ��ʾ����Ŀ¼�����ļ�����ϸ��Ϣ
void display_Directory_Information(unsigned int directory_inode_index, bool isShowSubDirecotry = false) {
	// ��ȡĿ¼�� iNode
	iNode* dirInode = GetInode(directory_inode_index);

	// ���Ŀ¼�Ƿ���ж�ȡȨ��
	if (!dirInode->isRead_Available()) {
		string message_to_shell = "  ִ��ʧ��: ��Ŀ¼���ɶ�!\n";
		Deliver_Message_To_Shell(message_to_shell);  // �� Shell ���ʹ�����Ϣ
		return;  // ֱ�ӷ���
	}

	// ��ʾĿ¼�Ļ�����Ϣ
	string message_to_shell[10];
	message_to_shell[0] = "   Ŀ¼��:    " + string(dirInode->file_name) + "\n";
	message_to_shell[1] = "   ��ռ���λ��:    " + to_string(GetDataBlockPosition(dirInode->blockPointer[0])) + "\n";
	message_to_shell[2] = "   Ŀ¼����:        " + to_string(CalculateFileSize(dirInode)) + "B\n\n";

	// ����Ϣ���͵� Shell
	send_Messages(message_to_shell, 3);

	// ���µ�ǰĿ¼ iNode
	Current_Directory_iNode = directory_inode_index;

	// �������Ҫ��ʾ��Ŀ¼�����ļ�����Ϣ��ֱ�ӷ���
	if (!isShowSubDirecotry) return;

	// ��ʾ��Ŀ¼�����ļ�������
	message_to_shell[0] = "    ��Ŀ¼/���ļ�������:    " + to_string(dirInode->files_num) + "\n";

	// �������ļ�/��Ŀ¼����Ϣ��ͷ
	SetMessageHeader(message_to_shell[1]);
	message_to_shell[1] = "    " + message_to_shell[1];  // �����������ʽ�����

	// ����Ŀ¼/���ļ������ͱ�ͷ��Ϣ���͵� Shell
	send_Messages(message_to_shell, 2);

	// ��ʾĿ¼�е��������ļ�����Ŀ¼����Ϣ
	DirectoryInformation(dirInode);

	// ��ʾ��Ϻ���뻻��
	Deliver_Message_To_Shell("\n\n");

	return;
}


// ��ʾָ��Ŀ¼�����νṹ
// ʹ������չʾ��Ŀ¼�㼶��ϵ
void DisplayDirectoryTree(iNode* dirInode, int indentLevel = 0) {
	// ���ݲ㼶���������ַ���
	std::string indent(indentLevel * 8, ' '); // ÿһ������ 8 ���ո�

	// ��ȡ����ʾ��ǰĿ¼���ļ�����Ϣ
	std::string dirInfo = indent + SubFileInformation(dirInode);
	Deliver_Message_To_Shell(dirInfo);

	// ��鵱ǰ iNode �Ƿ���Ŀ¼����
	if (dirInode->isDirectoryType()) {
		// ��ȡĿ¼���ݿ��λ�ò��������ݿ�����
		unsigned int blockPosition = dirInode->blockPointer[0];
		blockPosition = GetDataBlockPosition(blockPosition);
		Block dirBlock;
		dirBlock.loadBlockData(blockPosition);

		// ����Ŀ¼�µ��������ļ�����Ŀ¼
		for (unsigned int i = 0; i < dirInode->files_num; ++i) {
			unsigned int child_inode_index = dirBlock.data[i];
			iNode* childInode = GetInode(child_inode_index);

			// �ݹ���ã�������������
			DisplayDirectoryTree(childInode, indentLevel + 1);
		}
	}
}

// ��ʾָ��Ŀ¼�����νṹ����������
// ��Ŀ¼����ʾ�㼶��ϵ
void DisplayDirectoryTree(iNode* dirInode, string s) {
	// ��ȡ����ʾ��ǰĿ¼���ļ�����Ϣ
	std::string dirInfo = SubFileInformation(dirInode);
	Deliver_Message_To_Shell(dirInfo);

	// ��鵱ǰ iNode �Ƿ���Ŀ¼����
	if (dirInode->isDirectoryType()) {
		// ��ȡĿ¼���ݿ��λ�ò��������ݿ�����
		unsigned int blockPosition = dirInode->blockPointer[0];
		blockPosition = GetDataBlockPosition(blockPosition);
		Block dirBlock;
		dirBlock.loadBlockData(blockPosition);

		// ����Ŀ¼�µ��������ļ�����Ŀ¼
		for (unsigned int i = 0; i < dirInode->files_num; ++i) {
			unsigned int child_inode_index = dirBlock.data[i];
			iNode* childInode = GetInode(child_inode_index);

			// �ݹ���ã�������
			DisplayDirectoryTree(childInode, "");
		}
	}
}

// ��ʾ��ǰĿ¼�ľ���·��
void Display_CurrentDirectoryPath() {
	string absolutePath = "";  // ���ڴ洢��ǰĿ¼�ľ���·��

	// ���ú�����ȡ��ǰĿ¼�ľ���·��
	absolutePath = Get_DirectoryAbsolutePath();

	// ȷ��·���� '/' ��ͷ
	if (absolutePath[0] != '/') {
		absolutePath = "/" + absolutePath;
	}

	// ��ʽ��·����ʾ��������ʾ��
	absolutePath = "  [@" + std::string(Current_User)+ "]  " + absolutePath + "># ";

	// ��·����Ϣ���͵� Shell
	Deliver_Message_To_Shell(absolutePath);

	return;
}


//IPC

//basic function
//������ݿ��еĶ�������Э��

// ��ʾ�ļ�ϵͳ�Ļ�����Ϣ
void Info() {
	// �ӹ��� S ����ȷ����ȡ�����İ�ȫ��
	rw_Controller.Set_Reader_SLock();

	// �����ļ�ϵͳ������״̬���ڴ�
	Load_Simdisk();

	// ��ʾ�ļ�ϵͳ�Ļ�����Ϣ����������ʹ������ͽṹ��Ϣ
	display_Simdisk_Information();

	// �ͷŹ��� S ��
	rw_Controller.Release_Reader_SLock();

	return;
}

// �ı䵱ǰ����Ŀ¼ (cd ����)
void Cd() {
	// �ӹ��� S ����ȷ����ȡ�����İ�ȫ��
	rw_Controller.Set_Reader_SLock();

	// �����ļ�ϵͳ������״̬���ڴ�
	Load_Simdisk();

	// ���û������·��ת��Ϊ����·��
	Change_To_AbsolutePath(command.cmd[1]);

	// �ı䵱ǰĿ¼��ָ���ľ���·��
	Change_Directory(command.cmd[1]);

	// �ͷŹ��� S ��
	rw_Controller.Release_Reader_SLock();

	return;
}

// ��ʾĿ¼���ݣ�֧�ֵ�ǰĿ¼��ָ��Ŀ¼����ѡ��ʾ��Ŀ¼��
void Dir() {
	rw_Controller.Set_Reader_SLock(); // ���ö�����ȷ����ȡ������ȫ

	Load_Simdisk(); // �����ļ�ϵͳ������״̬��ȷ������һ����

	int count = command.cmd_count; // ��ȡ�������������
	string input_cmd[3];
	for (int i = 0; i < 3; i++) {
		input_cmd[i] = command.cmd[i];
	}

	// ����������������������ж�ִ�еĹ���
	if (count == 1) {
		// ����: dir
		display_Directory_Information(Current_Directory_iNode, false); // ��ʾ��ǰĿ¼��Ϣ������ʾ��Ŀ¼
	}
	else if (count == 2 && input_cmd[1] == "s") {
		// ����: dir s
		display_Directory_Information(Current_Directory_iNode, true); // ��ʾ��ǰĿ¼������Ŀ¼
	}
	else if (count == 2 && input_cmd[1] != "s") {
		// ����: dir path
		Change_To_AbsolutePath(input_cmd[1]); // ��·��ת��Ϊ����·��
		unsigned int directory_inode_index = Search_File_INode(input_cmd[1]); // ����·����Ӧ�� iNode

		if (isPath_Exist(directory_inode_index)) {
			display_Directory_Information(directory_inode_index, false); // ��ʾָ��Ŀ¼����Ϣ������ʾ��Ŀ¼
		}
	}
	else if (count == 3 && input_cmd[2] == "s") {
		// ����: dir path s
		Change_To_AbsolutePath(input_cmd[1]); // ��·��ת��Ϊ����·��
		unsigned int directory_inode_index = Search_File_INode(input_cmd[1]); // ����·����Ӧ�� iNode

		if (isPath_Exist(directory_inode_index)) {
			display_Directory_Information(directory_inode_index, true); // ��ʾָ��Ŀ¼������Ŀ¼����Ϣ
		}
	}
	else {
		Deliver_Message_To_Shell(" �������!\n"); // ��ʾ�û�������������
	}

	rw_Controller.Release_Reader_SLock(); // �ͷŶ���
	return;
}

// ������Ŀ¼��֧�ֵ�ǰĿ¼��ָ��·���´�����
void Md() {
	rw_Controller.Set_Writer_XLock(); // ����д����ȷ��д������ȫ
	string message_to_shell; // ���ڴ洢���͵� Shell ����Ϣ

	Load_Simdisk(); // �����ļ�ϵͳ������״̬��ȷ������һ����

	// �������������ڷ�����Ϣ�� Shell ���ͷ�д��
	auto deliverMessage = [&](const string& msg) {
		message_to_shell = msg; // ������Ϣ����
		Deliver_Message_To_Shell(message_to_shell); // ������Ϣ
		rw_Controller.Release_Writer_XLock(); // �ͷ�д��
	};

	if (command.cmd_count == 3) {
		// ����: md dirname 0/1/2
		if (is_File_Exist(Current_Directory_iNode, command.cmd[1])) {
			deliverMessage("  ִ��ʧ��: ��Ŀ¼�Ѵ���!\n");
			return;
		}

		iNode* currentInode = GetInode(Current_Directory_iNode);
		if (!currentInode->isWrite_Available()) {
			deliverMessage("  ִ��ʧ��: ��·����Ȩ��д��!\n");
			return;
		}

		Current_Permission = Parse_Permission(command.cmd[2]); // ����Ŀ¼Ȩ��
		Create_New_Directory(Current_Directory_iNode, command.cmd[1]); // ������Ŀ¼
		deliverMessage("  md ����ִ�гɹ�!\n");
	}
	else {
		// ����: md dirname path 0/1/2
		Change_To_AbsolutePath(command.cmd[2]); // ��·��ת��Ϊ����·��
		unsigned int directory_inode_index = Search_File_INode(command.cmd[2]); // ����·����Ӧ�� iNode

		if (isPath_Exist(directory_inode_index)) {
			iNode* dirInode = GetInode(directory_inode_index);

			if (!dirInode->isWrite_Available()) {
				deliverMessage("  ִ��ʧ��: ��·����Ȩд��!\n");
				return;
			}

			if (is_File_Exist(directory_inode_index, command.cmd[1])) {
				deliverMessage("  ִ��ʧ��: ��Ŀ¼�Ѵ���!\n");
				return;
			}

			Current_Permission = Parse_Permission(command.cmd[3]); // ����Ŀ¼Ȩ��
			Create_New_Directory(directory_inode_index, command.cmd[1]); // ������Ŀ¼
			deliverMessage("  md ����ִ�гɹ�!\n");
		}
		else {
			rw_Controller.Release_Writer_XLock(); // �ͷ�д��
		}
	}
}

// ɾ��Ŀ¼ (rd path)
void Rd() {
	rw_Controller.Set_Writer_XLock(); // ����д����ȷ��ɾ��������ȫ
	Load_Simdisk(); // �������µ��ļ�ϵͳ״̬
	Change_To_AbsolutePath(command.cmd[1]); // ���û������·��ת��Ϊ����·��

	unsigned int directory_inode_index = Search_File_INode(command.cmd[1]); // ��ȡָ��·���� iNode
	if (directory_inode_index == Nothing) {
		Deliver_Message_To_Shell("  ִ��ʧ��: ��·��������!\n"); // ��ʾ·��������
		rw_Controller.Release_Writer_XLock(); // �ͷ�д��
		return;
	}

	iNode* dirInode = GetInode(directory_inode_index); // ��ȡĿ¼��Ӧ�� iNode
	if (!dirInode->isDirectoryType()) {
		Deliver_Message_To_Shell("  ���ļ�����Ŀ¼��ɾ����ʹ�� del ����\n"); // ��Ŀ¼������ʾ
		rw_Controller.Release_Writer_XLock(); // �ͷ�д��
		return;
	}

	if (directory_inode_index == 0 && Current_User_Id != 0) {
		Deliver_Message_To_Shell("   �޹���ԱȨ�ޣ���Ȩɾ����Ŀ¼!\n"); // ����Ŀ¼ɾ��Ȩ��
		rw_Controller.Release_Writer_XLock(); // �ͷ�д��
		return;
	}

	if (!dirInode->isWrite_Available()) {
		Deliver_Message_To_Shell("  ִ��ʧ��: ��·����Ȩд��!\n"); // ��дȨ����ʾ
		rw_Controller.Release_Writer_XLock(); // �ͷ�д��
		return;
	}

	unsigned int parentDir_index = dirInode->parent_directory_index; // ��ȡ��Ŀ¼����

	if (!dirInode->isDirectoryEmpty()) {
		Deliver_Message_To_Shell(" Ŀ¼�ǿգ��Ƿ����ɾ��? [[Y]/N]\n");
		Input_Data_From_ShareMemory(); // ��ȡ�û�����
		string input_cmd = shell_to_simdisk.message[0];
		if (input_cmd == "y" || input_cmd == "Y" || input_cmd == "yes" || input_cmd == "YES") {
			Remove_Directory(directory_inode_index); // ɾ���ǿ�Ŀ¼
			Current_Directory_iNode = parentDir_index; // ���µ�ǰ·��
			Deliver_Message_To_Shell(" rd ����ִ�гɹ�!\n");
		}
		else {
			Deliver_Message_To_Shell(" �˳� rd ����ִ��!\n");
		}
	}
	else {
		Remove_Directory(directory_inode_index); // ɾ����Ŀ¼
		Current_Directory_iNode = parentDir_index; // ���µ�ǰ·��
		Deliver_Message_To_Shell(" rd ����ִ�гɹ�!\n");
	}

	rw_Controller.Release_Writer_XLock(); // �ͷ�д��
}

// �������ļ� (newfile filename [path] <0/1/2>)
void Newfile() {
	rw_Controller.Set_Writer_XLock(); // ����д����ȷ��д������ȫ
	string message_to_shell; // ���ڴ洢���͵� Shell ����Ϣ

	Load_Simdisk(); // �������µ��ļ�ϵͳ״̬

	auto sendMessage = [&](const string& message) {
		message_to_shell = message;
		Deliver_Message_To_Shell(message_to_shell); // ������Ϣ�� Shell
	};

	if (command.cmd_count == 3) { // �ڵ�ǰĿ¼���½��ļ�
		iNode* currentDirInode = GetInode(Current_Directory_iNode);

		if (is_File_Exist(Current_Directory_iNode, command.cmd[1])) {
			sendMessage("  ִ��ʧ��: ���ļ��Ѵ���!\n"); // �ļ��Ѵ���
		}
		else if (!currentDirInode->isWrite_Available()) {
			sendMessage("  ִ��ʧ��: ��·����Ȩд��!\n"); // ��дȨ��
		}
		else {
			Current_Permission = Parse_Permission(command.cmd[2]); // ��ȡȨ��
			Create_New_File(Current_Directory_iNode, command.cmd[1]); // �����ļ�
			sendMessage("  newfile ����ִ�гɹ�!\n"); // �ɹ���ʾ
		}
	}
	else if (command.cmd_count == 4) { // ��ָ��·�����½��ļ�
		Change_To_AbsolutePath(command.cmd[2]); // ת��Ϊ����·��
		unsigned int directory_inode_index = Search_File_INode(command.cmd[2]);

		if (isPath_Exist(directory_inode_index)) {
			iNode* dirInode = GetInode(directory_inode_index);

			if (is_File_Exist(directory_inode_index, command.cmd[1])) {
				sendMessage("  ִ��ʧ��: ���ļ��Ѵ���!\n"); // �ļ��Ѵ���
			}
			else if (!dirInode->isWrite_Available()) {
				sendMessage("  ִ��ʧ��: ��·����Ȩд��!\n"); // ��дȨ��
			}
			else {
				Current_Permission = Parse_Permission(command.cmd[3]); // ��ȡȨ��
				Create_New_File(directory_inode_index, command.cmd[1]); // �����ļ�
				sendMessage("  newfile ����ִ�гɹ�!\n");
			}
		}
	}
	else {
		sendMessage("  �������!\n");
	}

	rw_Controller.Release_Writer_XLock(); // �ͷ�д��
	return;
}

// ��ʾ�ļ����� (cat path)
void Cat() {
	rw_Controller.Set_Reader_SLock(); // ���ö�����ȷ����ȡ������ȫ
	Load_Simdisk(); // �������µ��ļ�ϵͳ״̬
	string message_to_shell; // ���ڴ洢���͵� Shell ����Ϣ

	// ���������������Ƿ���ȷ
	if (command.cmd_count != 2) {
		message_to_shell = "  �������!\n";
		Deliver_Message_To_Shell(message_to_shell);
		rw_Controller.Release_Reader_SLock(); // �ͷŶ���
		return;
	}

	string input_Path = command.cmd[1];
	Change_To_AbsolutePath(input_Path); // ���û������·��ת��Ϊ����·��
	unsigned int inode_index = Search_File_INode(input_Path); // ��ȡ·����Ӧ�� iNode
	iNode* fileInode = GetInode(inode_index);

	// ��ȷ�� inode ����֮ǰ�ͷŶ���
	rw_Controller.Release_Reader_SLock();

	// ���·���Ƿ����
	if (isPath_Exist(inode_index)) {
		// ����Ƿ�ΪĿ¼����
		if (fileInode->isDirectoryType()) {
			Deliver_Message_To_Shell("  ��·����Ŀ¼���޷�ʹ�� cat ����!\n");
			return;
		}

		// ����ȡȨ��
		if (!fileInode->isRead_Available()) {
			message_to_shell = "  ִ��ʧ��: ��·����Ȩ��ȡ!\n";
			Deliver_Message_To_Shell(message_to_shell);
			return;
		}

		rw_Controller.Set_Reader_SLock(); // �������ö�����ȷ����ȫ��ȡ
		Output_File_Data(inode_index); // ��ȡ�ļ����ݲ����
		Current_Directory_iNode = fileInode->parent_directory_index; // ���µ�ǰ����Ŀ¼
	}

	rw_Controller.Release_Reader_SLock(); // �ͷŶ���
	return;
}

// ʵ���ļ��������� (֧��������ģ���ļ�ϵͳ֮��Ŀ������Լ��ļ�ϵͳ�ڲ�����)
void Copy() {
	rw_Controller.Set_Writer_XLock(); // ����д����ȷ��д������ȫ����ֹ�����̻߳����ͬʱ����д����

	Load_Simdisk(); // �������µ��ļ�ϵͳ״̬��ȷ���ļ�ϵͳ����һ��
	Change_To_AbsolutePath(command.cmd[2]); // ��Ŀ��·����������������ת��Ϊ����·�������ں�������

	string message_to_shell; // ���ڴ洢���͵� Shell ����Ϣ���������û������������

	if (command.cmd[0] == "copy<host>") { // �����������ļ�ϵͳ֮��Ŀ�������
		if (command.cmd_count != 5) { // �жϲ��������Ƿ���ȷ
			message_to_shell = "  �������!\n";
			Deliver_Message_To_Shell(message_to_shell); // ���ʹ�����Ϣ�� Shell
			rw_Controller.Release_Writer_XLock(); // �ͷ�д��
			return; // ���أ���ֹ��ǰ����
		}

		if (command.cmd[3] == "0") { // �������������ļ�ϵͳ
			string filename; // ���ڴ洢������·������ȡ���ļ���
			const string& filepath = command.cmd[1]; // �����ļ�������·��

			// ��ȡ�ļ�������·�����ҵ����һ����б�ܺ�Ĳ���
			for (size_t i = filepath.size(); i > 0; --i) {
				if (filepath[i - 1] == '\\') {
					filename = filepath.substr(i);
					break;
				}
			}

			// ����Ŀ��Ŀ¼���ļ�ϵͳ�е� inode ����
			unsigned int dest_directory_inode_index = Search_File_INode(command.cmd[2]);

			if (isPath_Exist(dest_directory_inode_index)) { // ȷ��Ŀ��·���Ƿ����
				iNode* dirInode = GetInode(dest_directory_inode_index); // ��ȡĿ��Ŀ¼�� inode ��Ϣ

				if (is_File_Exist(dest_directory_inode_index, filename)) { // ���Ŀ��Ŀ¼���Ƿ��Ѵ���ͬ���ļ�
					message_to_shell = "  ִ��ʧ��: ���ļ��Ѵ���!\n"; // ��ʾ�ļ��Ѵ���
				}
				else if (!dirInode->isWrite_Available()) { // ���Ŀ��Ŀ¼�Ƿ���дȨ��
					message_to_shell = "  ִ��ʧ��: ��·����Ȩд��!\n"; // ��ʾ��дȨ��
				}
				else {
					Current_Permission = Parse_Permission(command.cmd[4]); // �����ļ���Ȩ��ģʽ
					copy_File_From_Host_To_Simdisk(filename, command.cmd[1], dest_directory_inode_index); // �����������ļ����ļ�ϵͳ
					message_to_shell = "  �ļ��ɹ�������YDFS!\n"; // ��ʾ�����ɹ�
				}
			}
		}
		else if (command.cmd[3] == "1") { // ���ļ�ϵͳ����������
			unsigned int src_inode_index = Search_File_INode(command.cmd[2]); // ����Դ�ļ����ļ�ϵͳ�е� inode ����

			if (isPath_Exist(src_inode_index)) { // ȷ��Դ�ļ�·���Ƿ����
				iNode* srcInode = GetInode(src_inode_index); // ��ȡԴ�ļ��� inode ��Ϣ
				if (!srcInode->isRead_Available()) { // ���Դ�ļ��Ƿ��ж�Ȩ��
					message_to_shell = "  ִ��ʧ��: ��·����Ȩ��ȡ!\n"; // ��ʾ�޶�Ȩ��
				}
				else {
					Copy_File_From_VFS_To_Host(src_inode_index, command.cmd[1]); // ���ļ�ϵͳ�����ļ�������
					message_to_shell = "  �ļ��ɹ����Ƶ�����!\n"; // ��ʾ�����ɹ�
				}
			}
		}
		else {
			message_to_shell = "  �������!\n"; // ��ʾ�����ʽ����
		}
	}
	else if (command.cmd[0] == "copy<ydfs>") { // �����ļ�ϵͳ�ڲ��Ŀ�������
		if (command.cmd_count != 4) { // �жϲ��������Ƿ���ȷ
			message_to_shell = "  �������!\n";
			Deliver_Message_To_Shell(message_to_shell);
			rw_Controller.Release_Writer_XLock();
			return;
		}

		string src_filename; // ���ڴ洢��Դ·������ȡ���ļ���
		const string& src_filepath = command.cmd[1]; // Դ�ļ�������·��

		// ��ȡԴ�ļ�������·�����ҵ����һ��б�ܺ�Ĳ���
		for (size_t i = src_filepath.size(); i > 0; --i) {
			if (src_filepath[i - 1] == '/') {
				src_filename = src_filepath.substr(i);
				break;
			}
		}

		Change_To_AbsolutePath(command.cmd[1]); // ��Դ·��ת��Ϊ����·��
		unsigned int src_inode_index = Search_File_INode(command.cmd[1]); // ����Դ�ļ��� inode ����
		unsigned int dest_inode_index = Search_File_INode(command.cmd[2]); // ����Ŀ��Ŀ¼�� inode ����
		iNode* srcInode = GetInode(src_inode_index); // ��ȡԴ�ļ��� inode ��Ϣ
		iNode* destInode = GetInode(dest_inode_index); // ��ȡĿ��Ŀ¼�� inode ��Ϣ

		if (isPath_Exist(src_inode_index) || isPath_Exist(dest_inode_index)) { // ȷ��·���Ƿ����
			if (is_File_Exist(dest_inode_index, src_filename)) { // ���Ŀ��Ŀ¼���Ƿ��Ѵ���ͬ���ļ�
				message_to_shell = "  ִ��ʧ��: ���ļ��Ѵ���!\n"; // ��ʾ�ļ��Ѵ���
			}
			else if (!srcInode->isRead_Available()) { // ���Դ�ļ��Ƿ��ж�Ȩ��
				message_to_shell = "  ִ��ʧ��: ��·����Ȩ��ȡ!\n"; // ��ʾ�޶�Ȩ��
			}
			else if (!destInode->isWrite_Available()) { // ���Ŀ��Ŀ¼�Ƿ���дȨ��
				message_to_shell = "  ִ��ʧ��: ��·����Ȩд��!\n"; // ��ʾ��дȨ��
			}
			else {
				Current_Permission = Parse_Permission(command.cmd[3]); // �������ļ���Ȩ��ģʽ
				Copy_within_VFS(src_filename, src_inode_index, dest_inode_index); // ���ļ�ϵͳ�ڲ������ļ�
				message_to_shell = "  �ļ������ɹ�!\n"; // ��ʾ�����ɹ�
			}
		}
	}

	Deliver_Message_To_Shell(message_to_shell); // �������Ϣ���͵� Shell
	rw_Controller.Release_Writer_XLock(); // �ͷ�д��������������������ִ��
	return; // ���أ���������
}

// ɾ��ָ��·�����ļ� (֧����ͨ�ļ�ɾ����Ŀ¼��ʹ�� rd ����)
void Del() {
	rw_Controller.Set_Writer_XLock(); // ����д����ȷ��ɾ���������̰߳�ȫ
	string message_to_shell; // ���ڴ洢���͵� shell ����Ϣ

	// ���������������Ƿ���ȷ
	if (command.cmd_count != 2) {
		message_to_shell = "  �������!\n"; // ������������
		Deliver_Message_To_Shell(message_to_shell); // ���ʹ�����Ϣ�� shell
		rw_Controller.Release_Writer_XLock(); // �ͷ�д��
		return; // ��������
	}

	Load_Simdisk(); // �����ļ�ϵͳ״̬��ȷ������һ����
	Change_To_AbsolutePath(command.cmd[1]); // ��ָ��·��ת��Ϊ����·��
	unsigned int file_inode_index = Search_File_INode(command.cmd[1]); // ����·����Ӧ�� iNode ����

	// ���·���Ƿ����
	if (isPath_Exist(file_inode_index)) {
		iNode* fileInode = GetInode(file_inode_index); // ��ȡ�ļ��� iNode ��Ϣ

		// ���дȨ��
		if (!fileInode->isWrite_Available()) {
			message_to_shell = "  ִ��ʧ��: ��·����Ȩд��!\n"; // ��дȨ��
		}
		// ����Ƿ���Ŀ¼�����ļ�
		else if (fileInode->isDirectoryType() == true) {
			message_to_shell = "  ���ļ���Ŀ¼��ɾ������ rd ����!\n"; // Ŀ¼��ʹ�� rd ����ɾ��
		}
		// ɾ����ͨ�ļ�
		else {
			unsigned int parentDirectory_index = fileInode->parent_directory_index; // ��ȡ��Ŀ¼����
			Delete_File(file_inode_index); // �����ļ�ɾ������
			message_to_shell = "  Del ����ִ�гɹ�!\n"; // ɾ���ɹ���Ϣ
			Current_Directory_iNode = parentDirectory_index; // ���µ�ǰ����Ŀ¼Ϊ��Ŀ¼
		}
	}

	Deliver_Message_To_Shell(message_to_shell); // ���Ͳ��������Ϣ�� shell
	rw_Controller.Release_Writer_XLock(); // �ͷ�д��
	return; // ��������
}

// ����ļ�ϵͳ��һ���Բ��޸�Ǳ�ڵ�����
void Check() {
	rw_Controller.Set_Writer_XLock(); // ����д����ȷ���������İ�ȫ�ԣ����������߳�/����ͬʱд��

	// �� shell �˷�����Ϣ����ʾ���ڼ���ļ�ϵͳһ����
	Deliver_Message_To_Shell("  ����ļ�ϵͳһ����...\n");

	// ����һ���Լ�麯�����ж��ļ�ϵͳ״̬
	bool flag = Compare_With_File();
	if (flag) {
		// �ļ�ϵͳһ���������������޸�
		Deliver_Message_To_Shell("  �ļ�һ��������.....\n");
	}
	else {
		// �ļ�ϵͳһ�����쳣����ʾ�޸�����
		Deliver_Message_To_Shell("  �ļ�һ�����쳣.....");
		Deliver_Message_To_Shell("  �޸��ļ�ϵͳһ������...\n");
	}

	Load_Simdisk(); // �����ļ�ϵͳ�������ļ�ϵͳ״̬��ȷ��һ����
	Save_Simdisk(); // �����ļ�ϵͳ״̬�����޸��������д���ļ�ϵͳ

	// ��ʾ������ɣ������ͳɹ���Ϣ�� shell
	string message_to_shell = "  Check ����ִ�гɹ�!\n";
	Deliver_Message_To_Shell(message_to_shell);

	rw_Controller.Release_Writer_XLock(); // �ͷ�д������������д����
	return; // ���أ���������
}

// �г��ļ�ϵͳ�е�����Ŀ¼���ļ���Ϣ
void Ls() {
	rw_Controller.Set_Reader_SLock(); // ���ö�����ȷ�����������̰߳�ȫ

	Load_Simdisk(); // �����ļ�ϵͳ��ȷ������һ����

	string message_to_shell;
	SetMessageHeader(message_to_shell); // ������ͷ��Ϣ���ļ��������͡�Ȩ�޵ȣ�
	Deliver_Message_To_Shell(message_to_shell); // �� shell �˷��ͱ�ͷ

	iNode* rootInode = GetInode(0); // ��ȡ��Ŀ¼�� iNode
	DisplayDirectoryTree(rootInode, ""); // �ݹ��г�Ŀ¼���е������ļ�����Ŀ¼��Ϣ

	rw_Controller.Release_Reader_SLock(); // �ͷŶ���������������д����
	return; // ���أ���������
}

// �����к�����ģ���ļ�ϵͳ�ĺ��Ŀ����߼�
void RunSimdisk() {
	string message_to_shell = ""; // ���ڴ洢���͵� shell ����Ϣ

	cout << "������ shell �˵�������...." << endl;

	sharememory_manager.CreateShareMemory(hMapFileCommFlags, pCommFlags, CommFlagsFileName);
	pc_Controller->V(pc_Controller->getConnectSem()); // ֪ͨ shell �������ѽ���

	message_to_shell = "�û�: " + std::string(Current_User) +
		" (ID: " + std::to_string(Current_User_Id) + ") ��ӭ���� YDAI VFS!\n";
	Deliver_Message_To_Shell(message_to_shell); // ���ͻ�ӭ��Ϣ�� shell

	bool flag = Initial_FileSystem(); // �����ʼ���ļ�ϵͳ

	Display_HelpMenu(); // ��ʾ������ʾ
	Current_Directory_iNode = Root_Directory; // ����ǰ����·����Ϊ��Ŀ¼

	while (true) {
		Display_CurrentDirectoryPath(); // ��ʾ��ǰĿ¼·��
		bool flag = Command_Interpreter(); // ִ���û����������
		if (!flag) { // ������յ��˳��������ѭ��
			break;
		}
	}

	message_to_shell = "  �˳�����ִ�гɹ�!\n";
	Deliver_Message_To_Shell(message_to_shell); // �����˳��ɹ���Ϣ

	sharememory_manager.CloseShareMemory(hMapFileCommFlags, pCommFlags);
	return;
}



