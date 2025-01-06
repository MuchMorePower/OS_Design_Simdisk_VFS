#include "constant.h"

// ��ǰ״̬�ĳ�ʼ��
unsigned int Current_Directory_iNode = Root_Directory;  // ��ǰĿ¼�� inode ID��Ĭ��Ϊ��Ŀ¼
char Current_User[50];  // ��ǰ�û���
unsigned int Current_User_Id; // ��ǰ�û���ID
unsigned int Current_Permission; // ��ǰ�û���Ȩ��ģʽ

fstream file_IO;    // �ļ����������
ofstream fout;      // �ļ������

// ����Ȩ���ַ��������ַ���ת��Ϊ����
unsigned int Parse_Permission(string s) {
    return std::stoi(s); // ���ַ���ת��Ϊ������ͨ�����ڽ����ļ����û���Ȩ��
}

// ���㵱ǰ�û���ID
void Calculate_UserId() {
    Current_User_Id = 0;  // Ĭ���û�IDΪ0������Ա��

    // �ж��Ƿ�Ϊ����Ա�û�
    if (std::strcmp(Current_User, "admin") != 0) {
        // ����ǰ�û���תΪ std::string ����
        std::string user_str(Current_User);

        // ʹ�ñ�׼��� hash ���������û����Ĺ�ϣֵ��Ϊ�û�ID
        std::hash<std::string> hash_fn;
        Current_User_Id = hash_fn(user_str);
    }

    // ����ǹ���Ա�û���Current_User_Id �ᱣ��Ϊ 0
    return;
}

// ��һ�� unsigned int ���͵�������ȡ�ֽڲ��洢���ַ�����
bool ExtractBytes(unsigned int data, std::string& bytes) {
    // �� 32 λ�����ֽ�Ϊ 4 ���ֽڣ������� bytes �ַ�����
    bytes[0] = static_cast<char>((data >> 24) & 0xFF);  // ��ȡ��λ�ֽ�
    bytes[1] = static_cast<char>((data >> 16) & 0xFF);  // ��ȡ�θ�λ�ֽ�
    bytes[2] = static_cast<char>((data >> 8) & 0xFF);   // ��ȡ�ε�λ�ֽ�
    bytes[3] = static_cast<char>(data & 0xFF);          // ��ȡ��λ�ֽ�

    // ��������ֽڶ�Ϊ '\0'��˵����ȡʧ�ܣ����� false
    return !(bytes[0] == '\0' && bytes[1] == '\0' && bytes[2] == '\0' && bytes[3] == '\0');
}

// ���ļ���ȡ 4 ���ֽڣ�������ת��Ϊ unsigned int ���͵�����
unsigned int Read_Data_To_Block() {
    unsigned int data_to_copy = 0;  // ���ڴ洢��ȡ������
    char bytes[4];  // �洢��ȡ�� 4 ���ֽ�

    // ���ļ����ж�ȡ 4 ���ֽڵ� bytes ������
    file_IO.read(bytes, sizeof(bytes));

    // ���ֽ�����ת��Ϊ unsigned int ���͵�����
    memcpy(&data_to_copy, bytes, sizeof(data_to_copy));

    // ���ض�ȡ������
    return data_to_copy;
}

// ��ȡ���ݿ��λ�ã�ͨ������������λ��
unsigned int GetDataBlockPosition(unsigned int block_index) {
    return dataBlock_begin + block_index;  // ���㲢�������ݿ��λ��
}
