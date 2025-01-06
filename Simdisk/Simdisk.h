#ifndef Simdisk
#define Simdisk
#include<vector>
#include<iostream>
#include<fstream>
#include<iomanip> // Ϊ std::setw �ṩ֧��
#include<sstream>
#include<queue>
#include"constant.h"

using namespace std;

class BootBlock { // �����飬ռ�ļ�ϵͳ1�飬�ܴ�СΪ1024�ֽ�
public:
	unsigned int iNode_array_begin;           // iNode�����ʼ���
	unsigned int iNode_array_end;             // iNode��Ľ������
	unsigned int dataBlock_begin;             // ���ݿ����ʼ���
	unsigned int dataBlock_end;               // ���ݿ�Ľ������

	unsigned int padding[252];//����ռ��ģ����̿ռ�

	BootBlock() {
		  // ��ʼ�� space �Ĵ�СΪ 256
		for (size_t i = 0; i < 252; ++i) {
			padding[i] = 0; //unsigned intռ4B��4 * 256 = 1024�ֽ�
		}
		// iNode�����ʼ�ͽ������ (�洢�ļ���Ԫ����)
		iNode_array_begin = 23;      // iNode��ӵ�23�鿪ʼ
		iNode_array_end = 4118;      // iNode��ռ����4096���飬ֱ����4118��

		// ���ݿ����ʼ�ͽ������ (���ڴ洢�ļ���ʵ������)
		dataBlock_begin = 4119;      // ���ݿ�ӵ�4119�鿪ʼ
		dataBlock_end = 102399;      // ���ݿ�һֱ����Ľ����飬����102399��
	}
	unsigned int BlockNum() {
		return dataBlock_end - dataBlock_begin + 1;
	}
	unsigned int iNodeArrayNum() {
		return iNode_array_end - iNode_array_begin + 1;
	}
};

class SuperBlock { // �����飬ռ�ļ�ϵͳ1�飬�ܴ�СΪ1024�ֽڣ�8192����
public:
	// �ļ�ϵͳ�Ļ�����Ϣ
	unsigned int total_block;     // �ܴ��̿���
	unsigned int total_datablock; // �����ݿ���Ŀ
	unsigned int used_datablock;  // ��ʹ�õ����ݿ���Ŀ����ʼʱ����Ŀ¼ռ��һ�����ݿ飩
	unsigned int total_inode;     // �� inode ��Ŀ
	unsigned int used_inode;      // ��ʹ�õ� inode ��Ŀ����ʼʱ����Ŀ¼ռ��һ�� inode��
	unsigned int block_size;      // ���̿��С����λ�ֽڣ�1024B����1KB��
	unsigned int inode_size;      // ÿ�� inode �Ĵ�С����λ�ֽڣ�128B��

	// �ļ�ϵͳ�й����ļ��ľ�������
	unsigned int file_total_num;  // �ļ�ϵͳ�������ļ�������Ŀ
	unsigned int file_num_directory;  // ÿ��Ŀ¼�����ɰ������ļ�����
	unsigned int name_length;        // �ļ�������󳤶ȣ���λ���ַ���

	// ������䣬ȷ���������СΪ1024�ֽ�
	unsigned int padding[246]; // ����ռ��ģ����̿ռ䣬�ܹ� 246 �� unsigned int��ռ 1024 �ֽ�

	// ���캯������ʼ��������ĸ������
	SuperBlock() {
		total_block = TOTAL_BLOCK_NUM; // �����ܿ���
		total_datablock = DataBlockNum; // �������ݿ���
		used_datablock = 1; // ��Ŀ¼ռ��һ�����ݿ�
		total_inode = TOTAL_INODE_NUM; // ������ inode ��
		used_inode = 1; // ��Ŀ¼ռ��һ�� inode
		block_size = BLOCK_SIZE; // ���ÿ��С
		inode_size = INODE_SIZE; // ���� inode ��С
		file_total_num = TOTAL_INODE_NUM; // �ļ��������� inode ����
		file_num_directory = File_Num_Directory; // ÿ��Ŀ¼���ļ�������
		name_length = NameLength; // �ļ�����������

		// ��� padding ���飬ȷ���������ܴ�СΪ1024�ֽ�
		for (size_t i = 0; i < 246; ++i) {
			padding[i] = 0; // �� padding ���������Ԫ�س�ʼ��Ϊ0
			// unsigned int ռ�� 4 �ֽڣ�246 * 4 = 984 �ֽڣ�ʣ�� 40 �ֽڽ��������ֶ�ռ��
		}
	}
};


class GroupDescription {
public:
	// �ļ�ϵͳ�еĲ�ͬ�������ʼ�ͽ������
	unsigned int group_begin;                 // �����ʼ���
	unsigned int group_end;                   // ��Ľ������
	unsigned int superBlock_begin;            // ���������ʼ���
	unsigned int superBlock_end;              // ������Ľ������
	unsigned int groupDescription_begin;      // ���������������ʼ���
	unsigned int groupDescription_end;        // ������������Ľ������
	unsigned int block_bit_map_begin;         // ��λͼ����ʼ���
	unsigned int block_bit_map_end;           // ��λͼ�Ľ������
	unsigned int iNode_bit_map_begin;         // iNodeλͼ����ʼ���
	unsigned int iNode_bit_map_end;           // iNodeλͼ�Ľ������

	unsigned int padding[246];               // ģ��һ����������ʾ���еĿռ�ռ�����

	GroupDescription() {
		// ��ʼ�������������ʼ�ͽ������
		group_begin = 1;             // ��1������Ŀ�ʼ��
		group_end = 102399;          // ��Ľ������ǵ�102399�飨��������ϵͳ��102400���飬��0���������飩

		// ���������ʼ�ͽ������ (������ͨ���洢�ļ�ϵͳ��������Ϣ)
		superBlock_begin = 1;        // ������ӵ�1�鿪ʼ
		superBlock_end = 1;          // �������ռ1�飬��˽���Ҳ�ǵ�1��

		// ���������������ʼ�ͽ������ (��¼�ļ�ϵͳ�и��������Ϣ)
		groupDescription_begin = 2;  // ���������ӵ�2�鿪ʼ
		groupDescription_end = 2;    // ��������ֻռ�õ�2��

		// ��λͼ����ʼ�ͽ������ (���ڼ�¼��Щ���ݿ��ѱ�ʹ��)
		block_bit_map_begin = 3;     // ��λͼ�ӵ�3�鿪ʼ
		block_bit_map_end = 18;      // ��λͼռ����16���飬ֱ����18��

		// iNodeλͼ����ʼ�ͽ������ (��¼��ЩiNode�ѱ�ʹ��)
		iNode_bit_map_begin = 19;    // iNodeλͼ�ӵ�19�鿪ʼ
		iNode_bit_map_end = 22;      // iNodeλͼռ����4���飬ֱ����22��

		// ��ʼ�� `padding` �����Ĵ�СΪ242����������Ԫ������Ϊ0
		for (int i = 0; i < 246; i++) {
			padding[i] = 0;            // ��������ÿ��Ԫ�ض�����Ϊ0
		}
	}

	// �������������Ĵ�С
	unsigned int groupDescriptionSize() {
		return groupDescription_end - groupDescription_begin + 1;
	}

	// �����λͼ�Ĵ�С
	unsigned int blockBitMapSize() {
		return block_bit_map_end - block_bit_map_begin + 1;
	}

	// ���� iNode λͼ�Ĵ�С
	unsigned int iNodeBitMapSize() {
		return iNode_bit_map_end - iNode_bit_map_begin + 1;
	}
};

class BlockBitMap { // ��λͼ��Block Bitmap��ռ16�����̿�, �ܴ�СΪ16KB��16384B)�������������ݿ��ʹ�����
public:
	unsigned int is_used[BLOCKBITMAP_SIZE];  // ���ڴ洢��λͼ�����飬ÿ��Ԫ�ض�Ӧ 32 �����ݿ��ʹ�����
	unsigned int padding[BLOCKBITMAP_PADDING]; // ������䣬ȷ����λͼ�Ĵ�С����Ҫ��

	// ���캯������ʼ����λͼ
	BlockBitMap() {
		// ��ʼ�����п�λͼΪ 0����ʾ���п鶼�ǿ��е�
		for (int i = 0; i < BLOCKBITMAP_SIZE; i++) {
			is_used[i] = 0;  // ��������λΪ 0����ʾ�������ݿ��ʼʱΪ����״̬
		}
		is_used[0] = 1;  // �� 0 ����Ϊ�����飨Boot Block������Ŀ¼ռ�øÿ飬������Ϊ��ʹ��
		for (int i = 0; i < BLOCKBITMAP_PADDING; i++) {
			padding[i] = 0; // ���λͼ�еĿ��ಿ��
		}
	}

	// ����һ���������ݿ�
	unsigned int Allocate_FreeBlock() {
		// Step 1: ������λͼ�ҵ���һ�����п�
		for (int row_index = 0; row_index < BLOCKBITMAP_SIZE; row_index++) {
			unsigned int& bitmap = this->is_used[row_index]; // ��ȡ��ǰ���λͼ��ʹ�����ü����ڴ濪��
			// ������ǰ���32��λ
			for (int column_index = 0; column_index < 32; column_index++) {
				if ((bitmap & (1 << column_index)) == 0) { // ��鵱ǰλ�Ƿ�Ϊ���У�0��ʾ���У�
					bitmap |= (1 << column_index); // ����ǰλ����Ϊ��ռ�ã�1��ʾ��ʹ�ã�
					return row_index * 32 + column_index; // ���ؿ��п���±�
				}
			}
		}
		return Nothing; // ���û���ҵ����п飬���ء��ޡ���ʾʧ�ܣ������ϲ�Ӧ�õ������
	}

	// �Ƴ�ĳ����ռ�ÿ��λͼ���
	void Remove_Block_BitMap(unsigned int block_position) {
		unsigned int row_index = 0, column_index = 0; // ��������λ����
		row_index = block_position / 32; // ���������
		column_index = block_position % 32;   // ����λ����
		unsigned int mask = ~(1 << (31 - column_index)); // �������룬�����Ӧλ�õ� 1����ʾ���У�
		this->is_used[row_index] &= mask; // �����Ӧλ�õ�λ�����Ϊδʹ�ã�0��ʾ���У�
	}
};

class iNodeBitMap { // ���ڱ�ʾ�ļ�ϵͳ�� iNode λͼ��iNode Bitmap����ռ 4 �����̿飬��СΪ 4KB
public:
	unsigned int is_used[INODEBITMAP_SIZE];  // iNode λͼ���飬ÿ��Ԫ�ر�ʾ 32 �� iNode ��ʹ�����

	// ���캯������ʼ�� iNode λͼ
	iNodeBitMap() {
		// ��ʼ������ iNode λͼΪ 0����ʾ���� iNode ��ʼʱΪ����
		for (int i = 0; i < INODEBITMAP_SIZE; i++) {
			is_used[i] = 0;  // ������λ����Ϊ 0����ʾ���� iNode ��ʼʱΪ����״̬
		}
		is_used[0] = 1;  // �� 0 �� iNode �ѱ�ռ�ã���Ŀ¼ռ�ã�����������Ϊ��ʹ��
	}

	// ����һ�����е� iNode
	unsigned int Allocate_FreeINode() {
		// Step 1: ���� iNode λͼ���ҵ���һ�����е� iNode
		for (int row_index = 0; row_index < INODEBITMAP_SIZE; row_index++) {
			unsigned int& bitmap = this->is_used[row_index];  // ��ȡ��ǰ iNode λͼ�飬���ü����ڴ濪��
			// ������ǰ��� 32 �� bit λ
			for (int column_index = 0; column_index < 32; column_index++) {
				// ��鵱ǰ bit λ�Ƿ�Ϊ���У�0��ʾ���У�
				if ((bitmap & (1 << column_index)) == 0) {
					// ����ǰ bit λ����Ϊ��ռ�ã�1��ʾ��ʹ�ã�
					bitmap |= (1 << column_index);
					return row_index * 32 + column_index;  // ���ط���� iNode ����
				}
			}
		}
		return Nothing;  // ���û�п��е� iNode������ Nothing�������ϲ�Ӧ�ﵽ�˴���
	}

	// �ͷ���ռ�õ� iNode λͼ�ռ�
	void Remove_Inode_Bitmap(unsigned int current_file_inode_index) {
		// Step 1: ���㵱ǰ iNode ���ڵĿ�������λ����
		unsigned int row_index = current_file_inode_index / 32;  // �������ڴ��̿������
		unsigned int column_index = current_file_inode_index % 32;  // ����λ������32 �� bit λ��

		// Step 2: ����� bit λ����ʾ�� iNode ���ͷ�
		this->is_used[row_index] &= ~(1 << (31 - column_index));  // �����Ӧ��λ���� 1 ��Ϊ 0�����Ϊ���У�
	}
};

class iNode { // iNode �࣬���ڱ�ʾ�ļ���Ԫ���ݣ�ÿ�� iNode ռ 128B
public:
	char file_name[NameLength];            // �ļ������Ϊ NameLength
	unsigned int parent_directory_index;   // ��Ŀ¼�� iNode ��ţ����ڱ�ʶ�ļ����ڵ�Ŀ¼
	unsigned int nextInode_index;          // ��һ�� iNode ��ţ�����ṹ��ָ����һ�� iNode ��λ�ã�
	bool isDirectory;                      // �ļ����ͱ�ʶ��true ��ʾĿ¼��false ��ʾ��ͨ�ļ�
	int files_num;                         // �����Ŀ¼�ļ�����¼��Ŀ¼���ļ�����
	int owner_id;                          // �ļ������ߵ��û� ID
	bool ReadPermission;                   // ��ȡȨ�ޣ�true ��ʾ�ɶ���false ��ʾ���ɶ�
	bool WritePermission;                  // д��Ȩ�ޣ�true ��ʾ��д��false ��ʾ����д
	int block_num;                         // �ļ�ռ�õĿ�����Ŀ¼�ļ����ռһ���飬��ͨ�ļ���ռ�����
	unsigned int blockPointer[MAX_BLOCK_POINTER]; // �洢���ݿ��ָ�룬Ŀ¼�ļ�������ļ��� iNode����ͨ�ļ�������ݿ��
	unsigned int filesize;                 // �ļ���С

	// ���캯������ʼ��Ϊ��Ŀ¼
	iNode() {
		for (int i = 0; i < sizeof(file_name); ++i) file_name[i] = '\0'; // �ļ�����ʼ��Ϊ���ַ���
		file_name[0] = '/';                    // ��Ŀ¼�ļ�����Ϊ '/'
		parent_directory_index = Nothing;      // ��Ŀ¼û�и�Ŀ¼������Ϊ��
		nextInode_index = Nothing;             // ��ʼ��Ϊ�޺�� iNode
		isDirectory = true;                    // ��ΪĿ¼
		files_num = 0;                         // ��Ŀ¼��ʼ�ļ�����Ϊ 0
		owner_id = AdminID;                    // ��������Ϊ���û�
		ReadPermission = true;                 // Ĭ�������ȡȨ��
		WritePermission = true;                // Ĭ������д��Ȩ��
		block_num = 1;                         // ��Ŀ¼ռ�� 1 �����ݿ�
		blockPointer[0] = 0;                   // ��Ŀ¼�ĵ�һ�����Ϊ 0
		for (int i = 1; i < MAX_BLOCK_POINTER; ++i) blockPointer[i] = Nothing; // ��ʼ��ʣ���ָ��Ϊ��
	}

	// ���� iNode ��Ϣ������������Ϊ��Ŀ¼�ĳ�ʼ״̬
	void reset_info() {
		for (int i = 0; i < sizeof(file_name); ++i) file_name[i] = '\0';  // ����ļ���
		file_name[0] = '/';                    // �ļ�������Ϊ '/'
		parent_directory_index = Nothing;      // ��Ŀ¼û�и�Ŀ¼
		nextInode_index = Nothing;             // û����һ�� iNode
		isDirectory = true;                    // ����ΪĿ¼
		files_num = 0;                         // �����ļ�����Ϊ 0
		owner_id = AdminID;                    // ������Ϊ���û�
		ReadPermission = true;                 // ��ȡȨ������Ϊ�ɶ�
		WritePermission = true;                // д��Ȩ������Ϊ��д
		block_num = 1;                         // Ŀ¼�ļ�ռ 1 �����ݿ�
		blockPointer[0] = 0;                   // Ŀ¼�ĵ�һ�����Ϊ 0
		for (int i = 1; i < MAX_BLOCK_POINTER; ++i) blockPointer[i] = Nothing; // ���ʣ���ָ��
	}

	// ������� iNode ��Ϣ����ʾ����ռ���κο�
	void clear_info() {
		for (int i = 0; i < sizeof(file_name); ++i) file_name[i] = '\0'; // ����ļ���
		parent_directory_index = Nothing;      // ��ո�Ŀ¼����
		nextInode_index = Nothing;             // ��պ�� iNode ����
		isDirectory = true;                    // ��ΪĿ¼
		files_num = 0;                         // �����ļ�����Ϊ 0
		owner_id = AdminID;                    // ����Ϊ���û�
		ReadPermission = true;                 // ��ȡȨ������Ϊ�ɶ�
		WritePermission = true;                // д��Ȩ������Ϊ��д
		block_num = 0;                         // ����ռ�ÿ���Ϊ 0
		for (int i = 0; i < MAX_BLOCK_POINTER; ++i) blockPointer[i] = Nothing; // ��տ�ָ��
	}

	// �������ļ��� iNode ��Ϣ
	void setup_new_file(const string& filename, unsigned int father_inode) {
		strncpy_s(file_name, filename.c_str(), NameLength - 1);  // �����ļ���
		file_name[NameLength - 1] = '\0';                          // ȷ���ļ����� '\0' ��β
		parent_directory_index = father_inode; // ���ø�Ŀ¼�� iNode ���
		nextInode_index = Nothing;             // ���ļ��޺�� iNode
		isDirectory = false;                   // ���Ϊ��ͨ�ļ�
		files_num = 0;                         // ��ͨ�ļ��������ļ�
		owner_id = Current_User_Id;            // �����ļ������ߵ��û� ID
		ReadPermission = Current_Permission > 0 ? true : false;  // ���ݵ�ǰȨ�����ö�ȡȨ��
		WritePermission = Current_Permission > 1 ? true : false; // ���ݵ�ǰȨ������д��Ȩ��
		block_num = 0;                         // ��ʼΪ���ļ�����ռ�����ݿ�
	}

	// ������Ŀ¼�� iNode ��Ϣ
	void setup_new_directory(const string& directory_name, unsigned int father_inode, unsigned int datablock) {
		strncpy_s(file_name, directory_name.c_str(), NameLength - 1);  // ����Ŀ¼��
		file_name[NameLength - 1] = '\0';                               // ȷ��Ŀ¼���� '\0' ��β
		parent_directory_index = father_inode; // ���ø�Ŀ¼�� iNode ���
		nextInode_index = Nothing;             // ��Ŀ¼�޺�� iNode
		isDirectory = true;                    // ���ΪĿ¼�ļ�
		files_num = 0;                         // ��ʼĿ¼�����ļ�
		owner_id = Current_User_Id;            // ���ô����ߵ��û� ID
		ReadPermission = Current_Permission > 0 ? true : false;  // ���ݵ�ǰȨ�����ö�ȡȨ��
		WritePermission = Current_Permission > 1 ? true : false; // ���ݵ�ǰȨ������д��Ȩ��
		block_num = 1;                         // Ŀ¼�ļ�ռ�� 1 �����ݿ�
		blockPointer[0] = datablock;           // ��¼Ŀ¼�����ݿ���
	}

	// �жϸ� iNode �Ƿ�ΪĿ¼����
	bool isDirectoryType() const {
		return this->isDirectory;
	}

	// �жϸ� iNode �Ƿ�Ϊ��Ŀ¼
	bool isDirectoryEmpty() {
		return (this->isDirectory == true && this->files_num == 0);
	}

	// ��鵱ǰ�û��Ƿ��ж�ȡȨ��
	bool isRead_Available() {
		// �ǹ���Ա���ļ��������뵱ǰ�û���ƥ�䣬��û�ж�ȡȨ��ʱ���� false
		if (Current_User_Id != AdminID && this->owner_id != Current_User_Id && ReadPermission == false) {
			return false;
		}
		return true;
	}

	// ��鵱ǰ�û��Ƿ���д��Ȩ��
	bool isWrite_Available() {
		// �ǹ���Ա���ļ��������뵱ǰ�û���ƥ�䣬��û��д��Ȩ��ʱ���� false
		if (Current_User_Id != AdminID && this->owner_id != Current_User_Id && WritePermission == false) {
			return false;
		}
		return true;
	}

	// ��ȡ�ļ���Ȩ�ޱ�ʾ�ַ������� "rwx"��
	string getPermission() {
		string permission = "";
		if (this->isRead_Available()) {
			permission += "r";  // �����ȡ
		}
		else {
			permission += "-";  // �������ȡ
		}
		if (this->isWrite_Available()) {
			permission += "w";  // ����д��
		}
		else {
			permission += "-";  // ������д��
		}
		// ֻ���ļ������߲�ӵ��ִ��Ȩ�ޣ��˴�Ĭ��ִ��Ȩ����ӵ����Ȩ�޹ҹ���
		if (this->owner_id == Current_User_Id || Current_User_Id == AdminID) {
			permission += "x";  // ����ִ��
		}
		else {
			permission += "-";  // ������ִ��
		}
		return permission;
	}

	// ������ļ� iNode ��ռ���ܿռ��С
	unsigned int CalculateTempSize() {
		return INODE_SIZE + BLOCK_SIZE * this->block_num;  // iNode ��С + ���ݿ��С * ����
	}
};

class iNode_Array { // ���ڱ�ʾһ������ 4096 ��ģ����̿�����飬ÿ���������� iNode��32768 ����
public:
	iNode inode[32768]; // iNode ���飬ÿ�� iNode ռ 128 �ֽڣ��ܹ��� 32768 �� iNode���� 4096 �����̿飩
};

class Block { // һ�����̿�Ĵ�СΪ1024B��1KB��
public:
	unsigned int data[256];  // �������飬ÿ��Ԫ��ռ�� 4 �ֽڣ�unsigned int������ 256 ��Ԫ�أ�ռ�� 1024 �ֽڣ��� 1KB��

	Block() {
		this->clear_BlockData();  // ���캯������ʼ��ʱ��տ��е�����
	}

	// ��մ��̿����ݣ��� data �����е�ÿ��Ԫ����Ϊ 0
	void clear_BlockData() {
		for (auto& d : data) {
			d = 0;  // ������ݣ���ÿ��Ԫ����Ϊ 0
		}
	}

	// ���ļ��м���ָ��λ�õ����ݿ�
	void loadBlockData(unsigned int position) {
		std::fstream file_IO(My_File_System_Name, std::ios::in | std::ios::binary); // ���ļ��Զ�ȡ����
		unsigned int offset = position * sizeof(Block); // �������ݿ����ļ��е�ƫ����
		file_IO.seekg(offset, std::ios::beg); // ��λ���ļ�����ȷλ��
		file_IO.read(reinterpret_cast<char*>(this), sizeof(Block)); // ��ȡ���ݵ���ǰ Block ����
		file_IO.close();  // �ر��ļ�
	}

	// ����ǰ���ݿ鱣�浽�ļ���ָ��λ��
	void saveBlockData(unsigned int position) {
		std::fstream file_IO(My_File_System_Name, std::ios::in | std::ios::out | std::ios::binary); // ���ļ��Զ�д����
		unsigned int offset = position * sizeof(Block); // �������ݿ����ļ��е�ƫ����
		file_IO.seekp(offset, std::ios::beg); // ��λ���ļ�����ȷλ��
		file_IO.write(reinterpret_cast<char*>(this), sizeof(Block)); // ����ǰ Block ���������д���ļ�
		file_IO.close(); // �ر��ļ�
	}

	// ���Ӵ��̶�ȡ�Ķ�����ݿ鸴�Ƶ���ǰ�������������
	void copyBlockData(int times) {
		for (int i = 0; i < times; i++) {
			this->data[i] = Read_Data_To_Block(); // �Ӵ��̶�ȡ���ݵ� data ����
		}
	}
};

class Path { // ����·���ֽ⣬��������ļ�ϵͳ�е��ļ�·��
public:
	unsigned int path_count; // ·���зֽ�����Ĳ��ֵļ�����
	string path_segment[3000]; // ���ڴ洢·���е�ÿһ���֣������Դ洢 3000 ��·����

	Path() {
		clear();  // ���캯������ʼ��·���ֽ���
	}

	// ���·������
	void clear() {
		path_count = 0; // ·�����ּ���������
		for (int i = 0; i < 3000; i++) {
			path_segment[i] = ""; // ��ÿ��·���γ�ʼ��Ϊ���ַ���
		}
	}

	// �ֶ������ַ������·������·���ֽ�ɸ�������
	void Divide_Path(string absolute_path) {
		this->clear(); // ���֮ǰ��·���ֽ���
		unsigned int length = absolute_path.length(); // ��ȡ·���ַ����ĳ���
		string segment = ""; // ��ʱ�ַ���������ƴ��·����

		for (unsigned int i = 1; i < length; i++) { // �� 1 ��ʼ������Ŀ¼�� '/'
			if (absolute_path[i] == '/') { // ·���еķָ���
				if (!segment.empty()) { // �����ǰ·���β�Ϊ��
					this->path_segment[this->path_count] = segment; // ����ǰ·���α��浽 path_segment ����
					this->path_count++; // ����·�����ּ�����
					segment.clear(); // �����ʱ·���α�����Ϊ��һ��·������׼��
				}
			}
			else { // ������� '/'�������ƴ�ӵ�ǰ��·����
				segment += absolute_path[i];
			}
		}

		// ���һ��·������ӵ� path ��
		if (!segment.empty()) { // ������һ��·���ηǿ�
			this->path_segment[this->path_count] = segment; // ��������·����
			this->path_count++; // ·�����ּ������� 1
		}
	}
};

extern BootBlock bootblock;  // �����飨Boot Block�����ļ�ϵͳ��������Ϣ��ͨ���洢�ļ�ϵͳ��Ԫ���ݣ�����̵Ļ�����Ϣ
extern SuperBlock superblock;  // �����飨Super Block�����洢�ļ�ϵͳ��������Ϣ�������ܿ�������ʹ�ÿ������� iNode ����
extern GroupDescription group_desciption;  // ������������Group Descriptor�����������������ϸ�����������Ϣ��ͨ������֧�ַ������ļ�ϵͳ
extern BlockBitMap block_bitmap;  // ��λͼ��Block Bitmap�������ڸ��ٴ�����ÿ�����ݿ��Ƿ�ռ�ã��� 0 ��ʾδʹ�ã�1 ��ʾ��ʹ��
extern iNodeBitMap inode_bitmap;  // iNode λͼ��iNode Bitmap�������ڸ����ļ�ϵͳ�е�ÿ�� iNode �Ƿ�ռ�ã�1 ��ʾ��ʹ�ã�0 ��ʾδʹ��
extern iNode_Array inode_array;  // iNode ���飨iNode Array�����洢���е� iNode��ÿ�� iNode ������һ���ļ���Ŀ¼��Ԫ���ݣ����ļ�����Ȩ�޵ȣ�

extern Path path;  // ·����������Path�������ڷֽ�ʹ洢·���е�ÿһ���֣������ļ�ϵͳ�����е�·������

extern queue<Block> blockCache;  // �黺�棨Block Cache�������ڻ��������ϵͳ������ģ���ļ�ϵͳ������


// ��ȡָ�������� iNode
iNode* GetInode(unsigned int inode_index);

// �ݹ�����ļ���Ŀ¼���ܴ�С������ iNode ������
unsigned int CalculateFileSize(unsigned int file_inode_index);

// �ݹ�����ļ���Ŀ¼���ܴ�С�����ݴ���� iNode ����
unsigned int CalculateFileSize(iNode* fileInode);

// ���ڻ�ȡ�ļ���Ŀ¼����ϸ��Ϣ������ʽ�����Ϊ�ַ���
string SubFileInformation(iNode* fileInode);

// �������ñ�ͷ��Ϣ����ʽ�����Ϊ�ַ�����ͨ��������ʾ�ļ�ϵͳ���ݵ�ͷ��
void SetMessageHeader(std::string& message_to_shell);

// ��ȡ��ǰĿ¼�ľ���·��
string Get_DirectoryAbsolutePath();

// ���Դ�ģ������ļ�������ļ��Ƿ�ɷ���
bool Open_Simdisk();

// �ж��Ƿ�����������������Ƿ���Լ��������µ� inode �����ݿ飩
bool is_Allocation_Available(unsigned int father_directory_inode);

// ��ȡһ�����е����ݿ�����
unsigned int Get_FreeBlockIndex();

// ��ȡһ�����е� iNode ����
unsigned int Get_FreeINodeIndex();

// д��/��ȡ �ļ�ϵͳ����
void YDFS_IO(std::fstream& file, bool isWrite);

// �����ļ�ϵͳ��д��
void Create_Simdisk();

// �����ļ�ϵͳ����
void Load_Simdisk();

// �����ļ�ϵͳ����
void Save_Simdisk();

// �˶��ļ�ϵͳ����һ����
bool Compare_With_File();

// �����·��ת��Ϊ����·��
void Change_To_AbsolutePath(string& relative_path);

// �ڵ�ǰĿ¼�²����ļ���Ϊ filename ���ļ������ض�Ӧ�� iNode ����
unsigned int Search_File_INode_Under_Directory(unsigned int now_directory, string filename);

// ���ݾ���·�������ļ��� iNode ����
unsigned int Search_File_INode(string absolute_path);

// �ж��ڸ�Ŀ¼���Ƿ������Ϊ sonFile_name ���ļ�
bool is_File_Exist(unsigned int fatherDirectory_inode, string sonFile_name);

// ���¸�Ŀ¼��Ϣ������һ���ļ��� iNode�����ڸ�Ŀ¼�����ݿ��и����ļ��б�
void Update_info_Create(unsigned int father_directory_inode, unsigned int file_inode);

// ���¸�Ŀ¼��Ϣ��ɾ��ָ���ļ��� iNode�����ڸ�Ŀ¼�����ݿ��и����ļ��б�
void Update_info_Remove(unsigned int current_file_inode);

// ����һ�����ļ���������ӵ�ָ���ĸ�Ŀ¼
unsigned int Create_New_File(unsigned int father_directory_inode, string filename);

// ����һ����Ŀ¼��������ӵ�ָ���ĸ�Ŀ¼
unsigned int Create_New_Directory(unsigned int father_directory_inode, string directory_name);

// ����ָ���ļ��� iNode λͼ��ǣ��ͷŸ��ļ�ռ�õ� iNode
void Clear_File_INode_Bitmap(unsigned int current_file_inode);

// ����ָ���ļ������ݿ�λͼ��ǣ��ͷŸ��ļ�ռ�õ����ݿ�
void Clear_File_Block_Bitmap(unsigned int current_file_inode);

// ���ָ���ļ���ռ�õ����ݿ�
void Clear_File_DataBlock(unsigned int current_file_inode);

// ɾ��ָ������ͨ�ļ�
void Delete_File(unsigned int current_file_inode);

// ɾ��ָ���Ŀ�Ŀ¼
void Remove_Empty_Directory(unsigned int current_directory_Inode);

// �ݹ�ɾ��ָ����Ŀ¼������Ŀ¼���ļ�
void Remove_Directory(unsigned int dirInode);

// ���ļ�ϵͳ�ڲ���һ���ļ����Ƶ���һ��λ��
void Transfer_File_Within_Simdisk(unsigned int src_inode, unsigned int dest_inode);

// �������ϵ��ļ����ص������У��Ա����ļ�ϵͳ��ʹ��
void load_HostFile_To_Cache(string hostpath);

// ��������������׷��д���ļ�ϵͳ
void Write_Cache_To_Simdisk(unsigned int file_inode);

#endif

