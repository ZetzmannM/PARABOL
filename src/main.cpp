#include <iostream>

/// @brief Test Struct for Doxygen tests
struct Test {
	/// @brief help
	int a=0;

	/// @brief constructor
	Test() {

	}
	
	/// @brief Leaks memory because, ehh, I mean why not
	void leakMemory() {
		new int[200];
	}
};

/// @brief Simple main Function
/// @return 0, hopefully
int main() {
	std::cout << "Hello" << std::endl;
	system("pause");
	return 0;
}