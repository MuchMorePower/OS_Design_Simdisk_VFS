#include "Simdisk.h"

BootBlock bootblock;
SuperBlock superblock;
GroupDescription group_desciption;
BlockBitMap block_bitmap;
iNodeBitMap inode_bitmap;
iNode_Array inode_array;

Path path;
queue<Block> blockCache; //��������ϵͳ��ģ���ļ�ϵͳYDFS֮������ݿ���                     



// ��ȡָ�������� iNode
iNode* GetInode(unsigned int inode_index) {
	// �ж� iNode �����Ƿ���Ч
	if (inode_index < 32768) {  // ���֧�� 32768 �� iNode
		// ���ض�Ӧ������ iNode
		return &inode_array.inode[inode_index];
	}
	else {
		// ���������Ч�����ؿ�ָ��
		return nullptr;
	}
}

// �ݹ�����ļ���Ŀ¼���ܴ�С������ iNode ������
unsigned int CalculateFileSize(unsigned int file_inode_index) {
	// ��ȡָ�������� iNode
	iNode* fileInode = GetInode(file_inode_index);
	unsigned int totalSize = 0;

	// �����Ŀ¼����
	if (fileInode->isDirectoryType()) {
		// ����Ŀ¼�ļ�����Ĵ�С
		totalSize += fileInode->CalculateTempSize();

		// ����Ŀ¼�µ�ÿ���ļ������ļ���
		for (unsigned int i = 0; i < fileInode->files_num; ++i) {
			// blockPointer �洢�������ļ��� iNode ���
			unsigned int child_inode_index = fileInode->blockPointer[i];
			// �ݹ�������ļ��Ĵ�С
			totalSize += CalculateFileSize(child_inode_index);
		}
	}
	else {
		// ��ͨ�ļ���ֱ�Ӽ������С
		totalSize += fileInode->CalculateTempSize();
		// ���������ļ���������ļ�����һ�� iNode����ݹ����
		while (fileInode->nextInode_index != Nothing) {
			// ��ȡ��һ�� iNode
			fileInode = GetInode(fileInode->nextInode_index);
			// �ݹ������һ�� iNode �Ĵ�С
			totalSize += fileInode->CalculateTempSize();
		}
	}
	// ���ؼ�����ܴ�С
	return totalSize;
}

// �ݹ�����ļ���Ŀ¼���ܴ�С�����ݴ���� iNode ����
unsigned int CalculateFileSize(iNode* fileInode) {
	unsigned int totalSize = 0;

	// �����Ŀ¼����
	if (fileInode->isDirectoryType()) {
		// ����Ŀ¼�ļ�����Ĵ�С
		totalSize += fileInode->CalculateTempSize();
		// ��ȡĿ¼�ĵ�һ�����ݿ��λ��
		unsigned int blockpostion = fileInode->blockPointer[0];
		blockpostion = GetDataBlockPosition(blockpostion);  // ��ȡ���ݿ��ʵ��λ��
		Block dirBlock;
		// ����Ŀ¼������
		dirBlock.loadBlockData(blockpostion);

		// ����Ŀ¼�µ�ÿ���ļ������ļ���
		for (unsigned int i = 0; i < fileInode->files_num; ++i) {
			// ����Ŀ¼���д洢�������ļ��� iNode ���
			unsigned int child_inode_index = dirBlock.data[i];
			// �ݹ�������ļ��Ĵ�С
			totalSize += CalculateFileSize(GetInode(child_inode_index));
		}
	}
	else {
		// ��ͨ�ļ���ֱ�Ӽ������С
		totalSize += fileInode->CalculateTempSize();
		// ���������ļ���������ļ�����һ�� iNode����ݹ����
		while (fileInode->nextInode_index != Nothing) {
			// ��ȡ��һ�� iNode
			fileInode = GetInode(fileInode->nextInode_index);
			// �ݹ������һ�� iNode �Ĵ�С
			totalSize += fileInode->CalculateTempSize();
		}
	}
	// ���ؼ�����ܴ�С
	return totalSize;
}

// ���ڻ�ȡ�ļ���Ŀ¼����ϸ��Ϣ������ʽ�����Ϊ�ַ���
string SubFileInformation(iNode* fileInode) {
	std::stringstream message;

	// ����������п��
	int fileNameWidth = 36;      // �ļ������
	int typeWidth = 15;          // ���Ϳ��
	int permissionWidth = 20;    // Ȩ�޿��
	int sizeWidth = 20;          // ��С��ȣ�������λ��
	int parentDirWidth = 20;     // ��Ŀ¼���

	// �ļ���������벢�����п�
	message << std::left << std::setw(fileNameWidth) << fileInode->file_name;

	// �ж��ļ����Ͳ���������Ϣ
	if (fileInode->isDirectoryType()) {
		// �����Ŀ¼����
		// ��ȡ��Ŀ¼���ƣ���û�и�Ŀ¼��Ĭ����ʾ "-"
		std::string parentDirName = "-";  // Ĭ��ֵ
		if (fileInode->parent_directory_index != Nothing) {  // ����Ƿ��и�Ŀ¼
			// ��ȡ��Ŀ¼�� iNode
			iNode* parentInode = GetInode(fileInode->parent_directory_index);
			if (parentInode) {
				// ��ȡ��Ŀ¼���ļ���
				parentDirName = parentInode->file_name;
			}
		}

		// ��ʽ�������Ŀ¼�����͡�Ȩ�ޡ���С����λΪ�ֽڣ��͸�Ŀ¼��
		message << std::setw(typeWidth) << "[DIR]"
			<< std::setw(permissionWidth) << fileInode->getPermission()  // �ļ�Ȩ��
			<< std::setw(sizeWidth) << (std::to_string(CalculateFileSize(fileInode)) + " B")  // �ļ���С����λ���ֽڣ�
			<< std::setw(parentDirWidth) << parentDirName << "\n";  // ��Ŀ¼����
	}
	else {
		// �������ͨ�ļ�����
		// ��ʽ��������ļ������͡�Ȩ�ޡ���С����λΪ�ֽڣ�
		message << std::setw(typeWidth) << "[FILE]"
			<< std::setw(permissionWidth) << fileInode->getPermission()
			<< std::setw(sizeWidth) << (std::to_string(CalculateFileSize(fileInode)) + " B");

		// ��ȡ��Ŀ¼���������
		iNode* parentInode = GetInode(fileInode->parent_directory_index);
		std::string parentDirName = parentInode ? parentInode->file_name : "-";  // ��û�и�Ŀ¼����ʾ "-"
		message << std::setw(parentDirWidth) << parentDirName << "\n";
	}

	if (fileInode->file_name == "/") {
		return "";
	}
	// ���ظ�ʽ�������Ϣ�ַ���
	return message.str();
}

// �������ñ�ͷ��Ϣ����ʽ�����Ϊ�ַ�����ͨ��������ʾ�ļ�ϵͳ���ݵ�ͷ��
void SetMessageHeader(std::string& message_to_shell) {
	std::stringstream header;

	// �����ͷ���п��
	int fileNameWidth = 36;      // �ļ������
	int typeWidth = 15;          // ���Ϳ��
	int permissionWidth = 20;    // Ȩ�޿��
	int sizeWidth = 15;          // ��С���
	int parentDirWidth = 20;     // ��Ŀ¼���

	// ʹ����ͬ���п�Ͷ��뷽ʽ���ñ�ͷ
	header << std::left << std::setw(fileNameWidth) << "��Ŀ¼/���ļ���"
		<< std::setw(typeWidth) << "�ļ�����"
		<< std::setw(permissionWidth) << "�ļ�Ȩ��"
		<< std::setw(sizeWidth) << "�ļ���С"
		<< std::setw(parentDirWidth) << "��Ŀ¼" << "\n";

	// ����ʽ����ı�ͷ��Ϣ���� message_to_shell
	message_to_shell = header.str();
} 

// ��ȡ��ǰĿ¼�ľ���·��
string Get_DirectoryAbsolutePath() {
	string absolutePath = "";  // ��ʼ��Ϊ���ַ��������ڴ洢����·��
	unsigned int current_directory_inode_index = Current_Directory_iNode;  // ��ǰĿ¼�� inode ����
	iNode* dirInode = GetInode(current_directory_inode_index); // ��ȡ��ǰĿ¼�� iNode

	// �ӵ�ǰĿ¼���ϱ�������㹹������·��
	for (;; current_directory_inode_index = dirInode->parent_directory_index) { // ����ѭ����ֱ�������Ŀ¼
		dirInode = GetInode(current_directory_inode_index); // ��ȡ��ǰĿ¼�� iNode
		string dirName = dirInode->file_name;  // ��ȡ��ǰĿ¼������

		// ��������Ŀ¼�����ļ���Ϊ "/"�����˳�ѭ��
		if (dirName == "/") {
			break;  // �˳�ѭ��
		}

		// ����ǰĿ¼����ӵ�·����ǰ��
		absolutePath = "/" + dirName + absolutePath;  // ƴ��Ŀ¼����·��
	}

	return absolutePath;  // �������յľ���·��
}

// ���Դ�ģ������ļ�������ļ��Ƿ�ɷ���
bool Open_Simdisk() {
	// Step 1: ������ֻ��������ģʽ���ļ�ϵͳ�Ķ������ļ�
	file_IO.open(My_File_System_Name, ios::in | ios::binary);

	// Step 2: ����ļ��Ƿ�ɹ���
	if (!file_IO) {
		// ����ļ���ʧ�ܣ���ӡ������Ϣ������ false
		cerr << "����: ���ļ�ϵͳ '" << My_File_System_Name << "' ʧ��" << endl;
		return false;  // �ļ���ʧ��
	}

	// Step 3: �ļ��ɹ��򿪺󣬹ر��ļ��������� true
	file_IO.close();  // �ر��ļ���
	return true;  // ���� true����ʾ�ļ�ϵͳ�ɹ���
}

// �ж��Ƿ�����������������Ƿ���Լ��������µ� inode �����ݿ飩
bool is_Allocation_Available(unsigned int father_directory_inode_index) {
	// ��ȡ��Ŀ¼�� iNode
	iNode* parentDir = GetInode(father_directory_inode_index);

	// ������·��������Ƿ����㣺
	// 1. ����ļ�ϵͳ���� inode �����ﵽ��� inode ����
	// 2. ����ļ�ϵͳ�������ݿ������ﵽ������ݿ�����
	// 3. �����Ŀ¼���ļ������Ѵﵽ����ļ�����
	if (superblock.used_inode == superblock.total_inode ||   // inode ʹ�����
		superblock.used_datablock == superblock.total_datablock || // ���ݿ�ʹ�����
		parentDir->files_num == superblock.file_num_directory) {  // ��Ŀ¼�ļ���������
		return false;  // ���������������ܽ��з���
	}

	return true;  // �������������Խ��з���
}

// ��ȡһ�����е����ݿ�����
unsigned int Get_FreeBlockIndex() {
	// ���Է���һ�����е����ݿ�
	unsigned int bit_position = block_bitmap.Allocate_FreeBlock(); // ����λͼ���亯��������һ�����п�
	if (bit_position != Nothing) {
		// ����ɹ����䣬���³������е���ʹ�����ݿ���
		superblock.used_datablock++; // ���³���������ݿ�������
		return bit_position;  // ���ط��䵽�Ŀ������ݿ������
	}
	else {
		// �������ʧ�ܣ����� Nothing
		return Nothing;  // �޿������ݿ飬���� Nothing
	}
}

// ��ȡһ�����е� iNode ����
unsigned int Get_FreeINodeIndex() {
	// ���Է���һ�����е� iNode
	unsigned int bit_position = inode_bitmap.Allocate_FreeINode(); // ����λͼ���亯��������һ������ iNode
	if (bit_position != Nothing) {
		// ����ɹ����䣬���³������е���ʹ�� iNode ��
		superblock.used_inode++; // ���³������ iNode ������
		return bit_position;  // ���ط��䵽�Ŀ��� iNode ������
	}
	else {
		// �������ʧ�ܣ����� Nothing
		return Nothing;  // �޿��� iNode������ Nothing
	}
}

// д��/��ȡ �ļ�ϵͳ����
void YDFS_IO(std::fstream& file, bool isWrite) {
	if (isWrite) {
		// д�����
		file.write((char*)&bootblock, sizeof(BootBlock));       // д��������
		file.write((char*)&superblock, sizeof(SuperBlock));     // д�볬����
		file.write((char*)&group_desciption, sizeof(GroupDescription)); // д����������
		file.write((char*)&block_bitmap, sizeof(BlockBitMap));       // д���λͼ
		file.write((char*)&inode_bitmap, sizeof(iNodeBitMap));       // д��iNodeλͼ
		file.write((char*)&inode_array, sizeof(iNode_Array));        // д��iNode��
	}
	else {
		// ��ȡ����
		file.read((char*)&bootblock, sizeof(BootBlock));       // ��ȡ������
		file.read((char*)&superblock, sizeof(SuperBlock));     // ��ȡ������
		file.read((char*)&group_desciption, sizeof(GroupDescription)); // ��ȡ��������
		file.read((char*)&block_bitmap, sizeof(BlockBitMap));       // ��ȡ��λͼ
		file.read((char*)&inode_bitmap, sizeof(iNodeBitMap));       // ��ȡiNodeλͼ
		file.read((char*)&inode_array, sizeof(iNode_Array));        // ��ȡiNode��
	}
}

// �����ļ�ϵͳ��д��
void Create_Simdisk() {
	file_IO.open(My_File_System_Name, ios::out | ios::binary); // ���ļ�����д��

	YDFS_IO(file_IO, true);  // д������

	// д�����ݿ�
	Block block;
	for (int i = 0; i < DataBlockNum; i++) {
		file_IO.write((char*)&block, sizeof(Block));  // д�����ݿ�
	}

	file_IO.close();
}

// �����ļ�ϵͳ����
void Load_Simdisk() {
	file_IO.open(My_File_System_Name, ios::in | ios::binary);  // ���ļ����ڶ�ȡ

	YDFS_IO(file_IO, false);  // ��ȡ����

	file_IO.close();
}

// �����ļ�ϵͳ����
void Save_Simdisk() {
	file_IO.open(My_File_System_Name, ios::in | ios::out | ios::binary); // ���ļ����ڶ�ȡ��д��
	file_IO.seekp(0, ios::beg); // ��дָ���ƶ����ļ���ʼ

	YDFS_IO(file_IO, true);  // д������

	file_IO.close();
}

// �˶��ļ�ϵͳ����һ����
bool Compare_With_File() {
	// �ڶ��Ϸ�����ʱ�������ڴ洢���ļ���ȡ������
	auto tempBootBlock = new BootBlock;           // ���� BootBlock ��ʱ����
	auto tempSuperBlock = new SuperBlock;         // ���� SuperBlock ��ʱ����
	auto tempGroupDescription = new GroupDescription; // ���� GroupDescription ��ʱ����
	auto tempBlockBitmap = new BlockBitMap;       // ���� BlockBitMap ��ʱ����
	auto tempInodeBitmap = new iNodeBitMap;       // ���� iNodeBitMap ��ʱ����
	auto tempInodeArray = new iNode_Array;        // ���� iNode_Array ��ʱ����

	// ���ļ���׼����ȡ�ļ�ϵͳ�Ķ���������
	file_IO.open(My_File_System_Name, ios::in | ios::binary); // �Զ����ƶ�ģʽ���ļ�

	// Lambda ��������ȡ���ݵ�ָ������
	auto readData = [&](auto* obj, size_t size) {
		file_IO.read(reinterpret_cast<char*>(obj), size); // ���ļ��ж�ȡָ����С�����ݵ� obj ָ��Ķ���
	};

	// ʹ�� lambda ��һ��ȡ�����ṹ�������
	readData(tempBootBlock, sizeof(BootBlock));           // ��ȡ BootBlock ����
	readData(tempSuperBlock, sizeof(SuperBlock));         // ��ȡ SuperBlock ����
	readData(tempGroupDescription, sizeof(GroupDescription)); // ��ȡ GroupDescription ����
	readData(tempBlockBitmap, sizeof(BlockBitMap));       // ��ȡ BlockBitMap ����
	readData(tempInodeBitmap, sizeof(iNodeBitMap));       // ��ȡ iNodeBitMap ����
	readData(tempInodeArray, sizeof(iNode_Array));        // ��ȡ iNode_Array ����

	// �ر��ļ���
	file_IO.close();  // ��ɶ�ȡ��ر��ļ�

	// Lambda �������Ƚ�����������ڴ������Ƿ����
	auto isEqual = [](const auto* obj1, const auto* obj2, size_t size) {
		return std::memcmp(obj1, obj2, size) == 0; // ʹ�� memcmp �Ƚ� obj1 �� obj2 ָ����ڴ������Ƿ���ͬ
	};

	// ʹ�� lambda ����Ƚ��ڴ�����
	bool result = isEqual(&bootblock, tempBootBlock, sizeof(BootBlock)) && // �Ƚ� BootBlock
		isEqual(&superblock, tempSuperBlock, sizeof(SuperBlock)) &&       // �Ƚ� SuperBlock
		isEqual(&group_desciption, tempGroupDescription, sizeof(GroupDescription)) && // �Ƚ� GroupDescription
		isEqual(&block_bitmap, tempBlockBitmap, sizeof(BlockBitMap)) &&   // �Ƚ� BlockBitMap
		isEqual(&inode_bitmap, tempInodeBitmap, sizeof(iNodeBitMap)) &&   // �Ƚ� iNodeBitMap
		isEqual(&inode_array, tempInodeArray, sizeof(iNode_Array));       // �Ƚ� iNode_Array

	// �ͷŶ��Ϸ������ʱ�ڴ�
	delete tempBootBlock;         // �ͷ� BootBlock ����
	delete tempSuperBlock;        // �ͷ� SuperBlock ����
	delete tempGroupDescription;  // �ͷ� GroupDescription ����
	delete tempBlockBitmap;       // �ͷ� BlockBitMap ����
	delete tempInodeBitmap;       // �ͷ� iNodeBitMap ����
	delete tempInodeArray;        // �ͷ� iNode_Array ����

	return result; // ���رȽϽ��������������һ�·��� true�����򷵻� false
}

// �����·��ת��Ϊ����·��
void Change_To_AbsolutePath(string& relativePath) {
	// ����Ǿ���·����ֱ�ӷ��أ������κδ���
	if (relativePath[0] == '/') return;

	// ���·��û���� '.' ��ͷ����ζ������һ����ͨ��·������� './' ��Ϊ���·����ǰ׺
	if (relativePath[0] != '.') {
		relativePath = "./" + relativePath; // ��·���������·���ı�ʶ��
	}

	// ����һ�����ַ��� temp�������洢ȥ��ǰ׺��·������
	string temp = "";
	int length = relativePath.length();

	// �ӵڶ����ַ���ʼ����·���ַ��������� './'��
	for (int i = 1; i < length; i++) {
		temp += relativePath[i]; // ��·���е��ַ���ӵ� temp ��
	}

	// ���� relativePath Ϊȥ�� './' ǰ׺���·��
	relativePath = temp;

	// ��ȡ��ǰ����Ŀ¼�ľ���·���������������·�����ӳ������ľ���·��
	relativePath = Get_DirectoryAbsolutePath() + relativePath;

	// ���ת����ľ���·�������ڵ��ԣ�
	cout << relativePath << endl;
}

// �ڵ�ǰĿ¼�²����ļ���Ϊ filename ���ļ������ض�Ӧ�� iNode ����
unsigned int Search_File_INode_Under_Directory(unsigned int current_directory_inode_index, string filename) {
	// ���ݵ�ǰĿ¼�� iNode ������ȡ��Ŀ¼�� iNode ��Ϣ
	iNode* dirInode = GetInode(current_directory_inode_index);

	// ��ȡ��Ŀ¼���ڵĵ�һ�����ݿ�Ĵ��̿�λ��
	unsigned int directory_block_position = dirInode->blockPointer[0];

	// ��ȡ���ݿ��λ�ò���ȡ���ݿ�
	directory_block_position = GetDataBlockPosition(directory_block_position);
	Block data_block;
	data_block.loadBlockData(directory_block_position);

	// �ڸ�Ŀ¼�����ݿ��У�����Ŀ¼�����е��ļ�������Ŀ���ļ���
	for (int i = 0; i < dirInode->files_num; i++) {
		// ͨ����ǰ�ļ��� iNode ���������ļ���Ӧ�� iNode ��Ϣ
		unsigned int current_file_inode_index = data_block.data[i];
		iNode* tempInode = GetInode(current_file_inode_index);

		// ����ҵ�ƥ����ļ��������ظ��ļ��� iNode ����
		if (tempInode->file_name == filename) {
			return current_file_inode_index;
		}
	}

	// ��������������ļ���û���ҵ�Ŀ���ļ������� Nothing���ļ������ڣ�
	return Nothing;
}

// ���ݾ���·�������ļ��� iNode ����
unsigned int Search_File_INode(string absolute_path) {
	// ���и�Ŀ¼·����ֱ�ӷ��ظ�Ŀ¼�� iNode ���� 0
	if (absolute_path == "/") return 0; // ��Ŀ¼ֱ�ӷ��� iNode �� 0

	// �ֽ����·�����õ�����·����
	path.Divide_Path(absolute_path);

	// �Ӹ�Ŀ¼��ʼ������
	unsigned temp_inode = Root_Directory; // ��ʼ���Ǹ�Ŀ¼
	for (int i = 0; i < path.path_count; i++) {
		// ������²�����Ŀ¼�� iNode���������ղ���Ŀ���ļ��� iNode
		temp_inode = Search_File_INode_Under_Directory(temp_inode, path.path_segment[i]);

		// ���ĳһ�����Ŀ¼���ļ�δ�ҵ���·����Ч������ Nothing
		if (temp_inode == Nothing) {
			return Nothing; // ·�����󣬷��� Nothing
		}
	}

	// ���������ҵ��� iNode ����
	return temp_inode;
}

// �ж��ڸ�Ŀ¼���Ƿ������Ϊ sonFile_name ���ļ�
bool is_File_Exist(unsigned int fatherDirectory_inode, string SubFile_name) {
	// ���� Search_File_INode_Under_Directory ��������Ŀ���ļ��� iNode
	unsigned int find_inode = Search_File_INode_Under_Directory(fatherDirectory_inode, SubFile_name);

	// ���δ�ҵ�Ŀ���ļ������� false����ʾ�ļ�������
	if (find_inode == Nothing) {
		return false;
	}

	// ����ҵ���Ŀ���ļ������� true����ʾ�ļ�����
	return true;
}

// ���¸�Ŀ¼��Ϣ������һ���ļ��� iNode�����ڸ�Ŀ¼�����ݿ��и����ļ��б�
void Update_info_Create(unsigned int father_directory_inode, unsigned int file_inode) {
	// ���ݸ�Ŀ¼�� iNode ������ȡ��Ŀ¼�� iNode ��Ϣ
	iNode* parentInode = GetInode(father_directory_inode);

	// ���Ӹ�Ŀ¼�е��ļ�����
	parentInode->files_num++;

	// ��ȡ��Ŀ¼�ĵ�һ�����ݿ�λ��
	unsigned int block_position = parentInode->blockPointer[0];
	block_position = GetDataBlockPosition(block_position); // ��ȡ���ݿ��ʵ��λ��

	// ����һ���µ����ݿ���󲢼������ݿ�����
	Block new_datablock;
	new_datablock.loadBlockData(block_position);

	// ���´������ļ� iNode ��Ŵ洢����Ŀ¼�����ݿ���
	int index = parentInode->files_num - 1; // ���ļ�Ӧ�ô洢���ļ���Ŀ����λ��
	new_datablock.data[index] = file_inode;  // ���ļ��� iNode �����ӵ����ݿ�

	// �����º�����ݿ鱣��ش���
	new_datablock.saveBlockData(block_position);
}

// ���¸�Ŀ¼��Ϣ��ɾ��ָ���ļ��� iNode�����ڸ�Ŀ¼�����ݿ��и����ļ��б�
void Update_info_Remove(unsigned int current_file_inode_index) {
	// �����ļ��� iNode ������ȡ���ļ��� iNode ��Ϣ
	iNode* fileInode = GetInode(current_file_inode_index);

	// ��ȡ�ļ��ĸ�Ŀ¼ iNode
	unsigned int father_directory_inode = fileInode->parent_directory_index;
	iNode* parentDir = GetInode(father_directory_inode);

	// ��ȡ��Ŀ¼�ĵ�һ�����ݿ�λ��
	unsigned int block_position = parentDir->blockPointer[0];
	block_position = GetDataBlockPosition(block_position); // ��ȡ���ݿ��ʵ��λ��

	// ����һ���µ����ݿ���󲢼������ݿ�����
	Block new_datablock;
	new_datablock.loadBlockData(block_position);

	// ����Ƿ����ҵ�Ҫɾ�����ļ�
	bool isFileDeleted = false;

	// ������Ŀ¼�����ݿ飬�ҵ�Ҫɾ�����ļ����������ļ���ǰ�ƶ����ɾ���ļ��Ŀ�λ
	for (int i = 0; i < parentDir->files_num; i++) {
		if (isFileDeleted) {
			// ������ҵ�ɾ�����ļ����������ļ���ǰ�ƶ�
			new_datablock.data[i] = new_datablock.data[i + 1];
		}
		else if (new_datablock.data[i] == current_file_inode_index) {
			// �ҵ�Ҫɾ�����ļ�����ǲ��������ļ��ƶ�����ǰ�ļ���λ��
			isFileDeleted = true;
			new_datablock.data[i] = new_datablock.data[i + 1];
		}
	}

	// ������º�����ݿ�
	new_datablock.saveBlockData(block_position);

	// ���ٸ�Ŀ¼�е��ļ�����
	parentDir->files_num--;
}

// ����һ�����ļ���������ӵ�ָ���ĸ�Ŀ¼
unsigned int Create_New_File(unsigned int father_directory_inode, string filename) {
	// ����ļ�ϵͳ�Ƿ����㹻����Դ��iNode �����ݿ飩���������ļ�
	if (!is_Allocation_Available(father_directory_inode)) {
		return Nothing;  // �����Դ���㣬���� Nothing ��ʾ����ʧ��
	}

	// �� iNode λͼ���ҵ�һ�����е� iNode������������ļ�
	unsigned int file_inode_index = Get_FreeINodeIndex();

	// ���¸�Ŀ¼����Ϣ�������ļ��� iNode ��ӵ���Ŀ¼�����ݿ���
	Update_info_Create(father_directory_inode, file_inode_index);

	// ��ȡ�´������ļ��� iNode ָ��
	iNode* fileInode = GetInode(file_inode_index);

	// �������ļ������ԣ������ļ����͸�Ŀ¼�� iNode ����
	fileInode->setup_new_file(filename, father_directory_inode);

	// �����ļ�ϵͳ��Ԫ���ݣ�ȷ������һ����
	Save_Simdisk();

	// ���µ�ǰ����Ŀ¼Ϊ��Ŀ¼
	Current_Directory_iNode = father_directory_inode;

	// �������ļ��� iNode ����
	return file_inode_index;
}

// ����һ����Ŀ¼��������ӵ�ָ���ĸ�Ŀ¼
unsigned int Create_New_Directory(unsigned int father_directory_inode_index, string directory_name) {
	// ����ļ�ϵͳ�Ƿ����㹻����Դ��iNode �����ݿ飩��������Ŀ¼
	if (!is_Allocation_Available(father_directory_inode_index)) {
		return Nothing;  // �����Դ���㣬���� Nothing ��ʾ����ʧ��
	}

	// �� iNode λͼ���ҵ�һ�����е� iNode�����������Ŀ¼
	unsigned int directory_inode_index = Get_FreeINodeIndex();

	// �����ݿ�λͼ���ҵ�һ�����е����ݿ飬���ڴ洢��Ŀ¼������
	unsigned int datablock_index = Get_FreeBlockIndex();

	// ���¸�Ŀ¼����Ϣ������Ŀ¼�� iNode ��ӵ���Ŀ¼�����ݿ���
	Update_info_Create(father_directory_inode_index, directory_inode_index);

	// ��ȡ�´�����Ŀ¼�� iNode ָ��
	iNode* dirInode = GetInode(directory_inode_index);

	// ������Ŀ¼�����ԣ�����Ŀ¼������Ŀ¼�� iNode ���������ݿ�����
	dirInode->setup_new_directory(directory_name, father_directory_inode_index, datablock_index);

	// �����ļ�ϵͳ��Ԫ���ݣ�ȷ������һ����
	Save_Simdisk();

	// ���µ�ǰ����Ŀ¼Ϊ�´�����Ŀ¼
	Current_Directory_iNode = directory_inode_index;

	// ������Ŀ¼�� iNode ����
	return directory_inode_index;
}

// ����ָ���ļ��� iNode λͼ��ǣ��ͷŸ��ļ�ռ�õ� iNode
void Clear_File_INode_Bitmap(unsigned int current_file_inode_index) {

	// �� iNode λͼ���Ƴ���ǰ�ļ��� iNode ��ǣ���ʾ�� iNode �ѱ��ͷ�
	inode_bitmap.Remove_Inode_Bitmap(current_file_inode_index);
	superblock.used_inode--; // ������ʹ�õ� iNode ����

	// ��ȡ��ǰ�ļ��� iNode ��Ϣ
	iNode* fileInode = GetInode(current_file_inode_index);

	// �����ļ��Ƿ�ռ���˶�� iNode�����Ƿ��к����� iNode��
	if (fileInode->nextInode_index != Nothing) {
		// ���������һ�� iNode���ݹ�������һ�� iNode ��λͼ���
		Clear_File_INode_Bitmap(fileInode->nextInode_index);
	}
}

// ����ָ���ļ������ݿ�λͼ��ǣ��ͷŸ��ļ�ռ�õ����ݿ�
void Clear_File_Block_Bitmap(unsigned int current_file_inode_index) {

	// ��ȡ��ǰ�ļ��� iNode ��Ϣ
	iNode* fileInode = GetInode(current_file_inode_index);

	// ������ǰ�ļ���ռ�õ��������ݿ飬�ͷŶ�Ӧ�Ŀ�λͼ���
	for (int i = 0; i < fileInode->block_num; i++) {
		unsigned int dataBlock_Position = fileInode->blockPointer[i]; // ��ȡ���ݿ�λ��
		block_bitmap.Remove_Block_BitMap(dataBlock_Position); // �ӿ�λͼ���Ƴ����ݿ���
		superblock.used_datablock--; // ������ʹ�õ����ݿ����
	}

	// �����ļ��Ƿ�ռ���˶�� iNode�����Ƿ��к����� iNode��
	if (fileInode->nextInode_index != Nothing) {
		// ���������һ�� iNode���ݹ�������һ�� iNode ���������ݿ�λͼ
		Clear_File_Block_Bitmap(fileInode->nextInode_index);
	}

	return;
}

// ���ָ���ļ���ռ�õ����ݿ�
void Clear_File_DataBlock(unsigned int current_file_inode_index) {

	// ��ȡָ���ļ��� iNode ��Ϣ
	iNode* fileInode = GetInode(current_file_inode_index);

	Block new_datablock;
	// �����ļ���ռ�õ��������ݿ飬�������
	for (int i = 0; i < fileInode->block_num; i++) {
		unsigned int block_position = fileInode->blockPointer[i];
		// �������ݿ��ڴ����е�ʵ��λ��
		block_position = GetDataBlockPosition(block_position);
		// ����յ����ݿ鵽���̣�����ԭ�е�����
		new_datablock.saveBlockData(block_position);
	}

	// ����ļ�ռ�ö�� iNode���ݹ���պ��� iNode ���������ݿ�
	if (fileInode->nextInode_index != Nothing) {
		Clear_File_DataBlock(fileInode->nextInode_index);
	}

	return;
}

// ɾ��ָ������ͨ�ļ�
void Delete_File(unsigned int current_file_inode_index) {
	// ����ļ���ռ�õ����ݿ�
	Clear_File_DataBlock(current_file_inode_index);

	// ���¸�Ŀ¼����Ϣ���Ƴ����ļ��� iNode ����
	Update_info_Remove(current_file_inode_index);

	// �����ļ���ռ�õĿ�λͼ���ͷ����ݿ�
	Clear_File_Block_Bitmap(current_file_inode_index);

	// �����ļ��� iNode λͼ���ͷ� iNode
	Clear_File_INode_Bitmap(current_file_inode_index);

	// �����ļ��� iNode ��Ϣ������ļ�Ԫ����
	iNode* fileInode = GetInode(current_file_inode_index);
	fileInode->reset_info();

	// �����ļ�ϵͳ��Ԫ���ݵ����̣�ȷ������һ����
	Save_Simdisk();

	return;
}

// ɾ��ָ���Ŀ�Ŀ¼
void Remove_Empty_Directory(unsigned int current_directory_Inode_index) {
	// ��ȡ��ǰĿ¼�� iNode ��Ϣ
	iNode* dirInode = GetInode(current_directory_Inode_index);

	// ���Ŀ¼�����ݿ�����
	unsigned int dataBlock_Position = dirInode->blockPointer[0]; // ��ȡĿ¼�ĵ�һ�����ݿ�����
	dataBlock_Position = GetDataBlockPosition(dataBlock_Position); // �������ݿ��ڴ����е�ʵ��λ��
	Block emptyBlock; // ����һ���յĿ飬���ڸ���ԭ�е�����

	emptyBlock.saveBlockData(dataBlock_Position); // ���տ�д����̣�������ݿ�����

	// ���¸�Ŀ¼����Ϣ���Ƴ���Ŀ¼�� iNode ����
	Update_info_Remove(current_directory_Inode_index);

	// ���¿�λͼ���ͷŸ�Ŀ¼ռ�õ����ݿ�
	block_bitmap.Remove_Block_BitMap(dataBlock_Position);
	superblock.used_datablock--; // ������ʹ�õ����ݿ����

	// ���� iNode λͼ���ͷŸ�Ŀ¼�� iNode
	inode_bitmap.Remove_Inode_Bitmap(current_directory_Inode_index);
	superblock.used_inode--; // ������ʹ�õ� iNode ����

	// ���ø�Ŀ¼�� iNode ��Ϣ�����Ԫ����
	dirInode->reset_info();

	// �����ļ�ϵͳ��Ԫ���ݵ����̣�ȷ������һ����
	Save_Simdisk();
}

// �ݹ�ɾ��ָ����Ŀ¼������Ŀ¼���ļ�
void Remove_Directory(unsigned int dirInode_index) {
	// ��ȡָ��Ŀ¼�� iNode ��Ϣ
	iNode* dirInode = GetInode(dirInode_index);

	// �ж��ļ����ͣ��������ͨ�ļ���ֱ��ɾ�����ݹ�ĳ��ڣ�
	if (!dirInode->isDirectoryType()) {
		Delete_File(dirInode_index); // ����ɾ���ļ��ĺ���
		return;
	}
	else {
		// �����Ŀ¼����
		if (dirInode->isDirectoryEmpty()) {
			// ���Ŀ¼Ϊ�գ�ֱ��ɾ���ÿ�Ŀ¼
			Remove_Empty_Directory(dirInode_index);
			return;
		}
		else {
			// ���Ŀ¼��Ϊ�գ���Ҫ�ݹ�ɾ����Ŀ¼���ļ�
			unsigned int dataBlock_Position = dirInode->blockPointer[0]; // ��ȡĿ¼�����ݿ�����
			dataBlock_Position = GetDataBlockPosition(dataBlock_Position); // �������ݿ��ڴ����е�ʵ��λ��

			Block Directory_Block;
			Directory_Block.loadBlockData(dataBlock_Position); // ����Ŀ¼�����ݿ�

			// �ݹ�ɾ�����ļ�����Ŀ¼
			while (!dirInode->isDirectoryEmpty()) {
				Directory_Block.loadBlockData(dataBlock_Position); // ���¼������ݿ飬��Ϊ���ݿ����Ѹ���
				unsigned int child_File_Inode_index = Directory_Block.data[0]; // ��ȡ��һ�����ļ��� iNode ����
				Remove_Directory(child_File_Inode_index); // �ݹ�ɾ�����ļ�����Ŀ¼
			}

			// ��Ŀ¼����գ�ɾ���ÿ�Ŀ¼
			Remove_Empty_Directory(dirInode_index);
		}
	}
}

// ���ļ�ϵͳ�ڲ���һ���ļ����Ƶ���һ��λ��
void Transfer_File_Within_Simdisk(unsigned int src_inode_index, unsigned int dest_inode_index) {

	// �ļ�ϵͳ�ڲ��Ķ������ļ��������� src_inode ���Ƶ� dest_inode ��
	// ��ȡԴ�ļ���Ŀ���ļ��� iNode ָ��
	iNode* srcInode = GetInode(src_inode_index);
	iNode* destInode = GetInode(dest_inode_index);

	// ʹĿ���ļ���ռ�����ݿ�������Դ�ļ�һ��
	destInode->block_num = srcInode->block_num;

	// �����ļ�����Ŀ���ļ��� iNode
	strcpy_s(destInode->file_name, NameLength, srcInode->file_name);

	// ���ƾ�������ݿ�����
	Block data_block;
	for (int i = 0; i < srcInode->block_num; i++) {
		// ��ȡԴ�ļ������ݿ�
		unsigned int block_position = srcInode->blockPointer[i];
		block_position = dataBlock_begin + block_position; // ��ȡ���ݿ��ڴ����е�ʵ��λ��

		data_block.loadBlockData(block_position); // ����Դ���ݿ�����

		// �����µ����ݿ��Ŀ���ļ�
		unsigned int new_block_index = Get_FreeBlockIndex();  // ��ȡһ�����е����ݿ�����
		destInode->blockPointer[i] = new_block_index;         // ����Ŀ���ļ��� iNode ��

		// �����ݿ�����д���µ����ݿ�λ��
		data_block.saveBlockData(dataBlock_begin + new_block_index);
	}

	// ���Դ�ļ�ռ���˶�� iNode����ʽ iNode�����ݹ鸴��
	if (srcInode->nextInode_index != Nothing) {
		// �����µ� iNode ��Ŀ���ļ�����һ������
		unsigned int next_inode_index = Get_FreeINodeIndex();  // ��ȡһ�����е� iNode ����
		iNode* nextInode = GetInode(next_inode_index);
		nextInode->clear_info();  // ����·���� iNode ��Ϣ

		destInode->nextInode_index = next_inode_index; // ����Ŀ���ļ��� nextInode_index

		// �ݹ鸴����һ�� iNode
		Transfer_File_Within_Simdisk(srcInode->nextInode_index, destInode->nextInode_index);
	}

	return;
}

// �������ϵ��ļ����ص������У��Ա����ļ�ϵͳ��ʹ��
void load_HostFile_To_Cache(string hostpath) {
	// ��Ҫ�������ļ��ŵ���������

	// ��ջ����� blockCache��ȷ��������û�в���������
	while (!blockCache.empty()) {
		blockCache.pop(); // �Ƴ������е����п�
	}

	// ��ȡ�ļ���С��������Ҫ���������̿������ܿ���
	unsigned int filesize = 0, full_need_block = 0, total_need_block = 0;
	file_IO.open(hostpath, ios::in | ios::binary); // �Զ����ƶ�ģʽ�������ļ�
	file_IO.seekg(0, ios::end);      // ���ļ�ָ���ƶ����ļ�ĩβ
	filesize = file_IO.tellg();      // ��ȡ�ļ���С���ֽ�����

	full_need_block = filesize / sizeof(Block); // ���������Ŀ���
	if (filesize % sizeof(Block) == 0) {
		total_need_block = full_need_block;
	}
	else {
		total_need_block = full_need_block + 1; // �����ʣ�ಿ�֣������� 1
	}
	file_IO.seekg(0, ios::beg); // �����ļ�ָ�뵽�ļ���ʼ

	// ��ȡ�����Ĵ��̿鲢���뻺��
	Block full_block;
	for (unsigned int i = 0; i < full_need_block; i++) {
		full_block.copyBlockData(256); // ��ȡ�����Ŀ����ݣ�256 �� unsigned int��
		blockCache.push(full_block);   // ������뻺�����
	}

	// ��ȡʣ������ݣ�����һ���飩�����뻺��
	Block remain_block;
	unsigned int remain_size = filesize % sizeof(Block); // ʣ������ݴ�С���ֽ�����
	unsigned int need_int = remain_size / sizeof(unsigned int); // ��Ҫ��ȡ�� unsigned int ����
	unsigned int need_char = remain_size % sizeof(unsigned int); // ʣ����ֽ���

	remain_block.copyBlockData(need_int); // ��ȡʣ��� unsigned int ����

	if (need_char != 0) {
		char chars[4] = { 0 }; // ����һ���ַ����飬��СΪ 4 �ֽڣ���ʼ��Ϊ 0
		file_IO.read(chars, need_char); // ��ȡʣ����ַ�����

		// ���ַ���ϳ�һ�� unsigned int
		unsigned int value = 0;
		for (unsigned int i = 0; i < need_char; i++) {
			value |= (static_cast<unsigned int>(static_cast<unsigned char>(chars[i])) << ((3 - i) * 8));
		}

		remain_block.data[need_int] = value; // ����Ϻ�� unsigned int ���� remain_block
	}

	blockCache.push(remain_block); // ��ʣ������ݿ���뻺��

	// �ر��ļ�
	file_IO.close();
}

// ��������������׷��д���ļ�ϵͳ
void Write_Cache_To_Simdisk(unsigned int file_inode_index) {

	// ԭ�ļ�����ռ���˶�� iNode����Ҫ�ҵ����һ�� iNode����������������
	iNode* fileInode = GetInode(file_inode_index);
	while (fileInode->nextInode_index != Nothing) {
		// �ƶ�����һ�� iNode
		file_inode_index = fileInode->nextInode_index;
		fileInode = GetInode(file_inode_index);
	}

	// ��ʼ��������������д���ļ�ϵͳ
	Block dataBlock;
	while (!blockCache.empty()) { // ����������Ϊ��ʱ
		// �жϵ�ǰ iNode �����ݿ�ָ���Ƿ������������������Ҫ�����µ� iNode ������
		if (fileInode->block_num == MAX_BLOCK_POINTER) { // ��ǰ iNode �Ѿ�����
			unsigned int next_inode_index = Get_FreeINodeIndex(); // ��ȡ�µĿ��� iNode ����
			iNode* nextInode = GetInode(next_inode_index);

			nextInode->clear_info(); // ����� iNode ����Ϣ
			fileInode->nextInode_index = next_inode_index; // ���� iNode ���ӵ���ǰ�ļ��� iNode ������

			// �ݹ���ã���ʣ�������д���µ� iNode
			Write_Cache_To_Simdisk(next_inode_index);
			return; // ���أ�������ǰ�ݹ�
		}

		// �ӻ������л�ȡ���ݿ�
		dataBlock = blockCache.front();  // ȡ�����׵����ݿ�
		blockCache.pop();                // �������׵����ݿ�

		// �����µ����ݿ�����
		unsigned int free_block_index = Get_FreeBlockIndex(); // ��ȡһ�����е����ݿ�����
		// �����ݿ�д������е����ݿ�����
		dataBlock.saveBlockData(free_block_index + dataBlock_begin);

		// ���µ�ǰ iNode ����Ϣ
		unsigned int index = fileInode->block_num; // ��ȡ��ǰ iNode ��ʹ�õ����ݿ�����
		fileInode->block_num++;                    // �������ݿ�����
		fileInode->blockPointer[index] = free_block_index; // ��¼�·�������ݿ�����
	}

	return;
}
