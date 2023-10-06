#include <iostream>

int main()
{
	std::printf("test");

	// Clear console:
	std::cout << "\033[2J\033[1;1H";
	
	return 0;
}