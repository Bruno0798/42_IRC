#include <iostream>
#include <cstdlib>

void check_args(int argc)
{
	if(argc != 3)
	{
		std::cout << "Correct usage is ./ircserv [port] [password] :)" << std::endl;
		EXIT_FAILURE;
	}
}


int main(int argc, char **argv)
{
	(void)argv;
	check_args(argc);


	return 0;
}