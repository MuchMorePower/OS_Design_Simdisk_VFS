#include "vfs.h"

int main() {
	srand((unsigned int)time(NULL));

	//��Shell��������
	ConnectToShell();

	//���CurrentUserId
	Calculate_UserId();

	//���г���
	RunSimdisk();

	return 0;

}