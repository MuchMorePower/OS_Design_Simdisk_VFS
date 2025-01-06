#include "vfs.h"

int main() {
	srand((unsigned int)time(NULL));

	//与Shell建立连接
	ConnectToShell();

	//获得CurrentUserId
	Calculate_UserId();

	//运行程序
	RunSimdisk();

	return 0;

}