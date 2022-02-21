#include "FileSystem.hpp"


int main(int argc,char* argv[])
{
	
	constexpr uint8_t pos_fs = 1;
	constexpr uint8_t pos_inputFileName = 2;
	constexpr uint8_t pos_outputFileName = 3;
	switch (argc)
	{
	case 4:
	{
		
		if (strcmp("-fs", argv[pos_fs]) != 0)
			std::cerr << "\nUnknown Command\n";
		FileSystem fs;
		if (fs.processFile(argv[pos_inputFileName]))
			fs.writeFile(argv[pos_outputFileName]);
		break;
	}
	default:
		std::cerr << "\nUNKNOWN COMMAND\n";
		return 0;
	}
	return 0;
}