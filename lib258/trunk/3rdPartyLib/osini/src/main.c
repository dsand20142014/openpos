#include <unistd.h>
#include <sand_ini.h>

int main(void)
{
	char value[32];
	GetValueFromEtcFile("./test.ini", "TEST", "test", value, sizeof(value));
	printf("%s\n",value);
	return 0;
}
