#include "FileSystem.hpp"

//Working with inputStr
inline void FileSystem::toLowerStr(std::string& str)
{
	for (size_t i = 0; i < str.size(); i++)
	{
		if (isalpha(str[i]))
			str[i] = tolower(str[i]);
	}
}

void FileSystem::inputCommand(std::string command)
{
	try {
		toLowerStr(command);//In order to ignore the case of the letter, we translate it into lower case
		switch (getCommand(command))
		{
		case FScommand::MD:workMD(command); break;
		case FScommand::CD:workCD(command); break;
		case FScommand::RD:workRD(command); break;
		case FScommand::DELTREE:workDELTREE(command); break;
		case FScommand::MF:workMF(command); break;
		case FScommand::MHL:workMHL(command); break;
		case FScommand::MDL:workMDL(command); break;
		case FScommand::DEL:workDEL(command); break;
		case FScommand::ÑOPY:workCOPY(command); break;
		case FScommand::MOVE:workMOVE(command); break;
		default:
			std::cerr << "UNKNOWN COMMAND";
			break;
		}
	}
	catch(std::runtime_error& err)
	{
		std::cerr << "\nFS error:" << err.what();
		throw(err);
	}
	catch (std::exception& err)
	{
		std::cerr << "\nStandard error:" << err.what();
		throw(err);
	}
	catch(...)
	{
		std::cerr << "\nUNDEFINED ERROR!";
		throw std::runtime_error("Something bad happened");
	}

}

FScommand FileSystem::getCommand(std::string & command)
{
	if (command.size() < 2)
		return FScommand::undefined;

	const size_t endCommand = command.find_first_of(" ");
	if (endCommand == std::string::npos)
		return FScommand::undefined;

	const std::string command_view = command.substr(0, endCommand);
	command = command.substr(endCommand + 1, command.length() - endCommand);

	if (command_view == FScommand_str::MD)
		return FScommand::MD;
	else if (command_view == FScommand_str::CD)
		return FScommand::CD;
	else if (command_view == FScommand_str::RD)
		return FScommand::RD;
	else if (command_view == FScommand_str::DELTREE)
		return FScommand::DELTREE;
	else if (command_view == FScommand_str::MF)
		return FScommand::MF;
	else if (command_view == FScommand_str::MDL)
		return FScommand::MDL;
	else if (command_view == FScommand_str::MHL)
		return FScommand::MHL;
	else if (command_view == FScommand_str::DEL)
		return FScommand::DEL;
	else if (command_view == FScommand_str::ÑOPY)
		return FScommand::ÑOPY;
	else if (command_view == FScommand_str::MOVE)
		return FScommand::MOVE;
	else return FScommand::undefined;
}

std::pair<std::string, std::string> FileSystem::splitPaths(const std::string& paths)
{
	if (paths.empty())
		return std::make_pair(std::string(), std::string());

	const size_t separator = paths.find_first_of(" ");
	if (separator == std::string::npos)
		return std::make_pair(std::string(), std::string());
	
	return std::make_pair(paths.substr(0, separator), paths.substr(separator + 1, paths.size() - separator));
}

Catalog* FileSystem::readPath(std::string &path, const bool readLastWord)
{
	//if (path == "c:")
	Catalog* temp = current;
	if (size_t pathBegin = path.find_first_of(pathSeparator); pathBegin == std::string::npos)
	{
		if(readLastWord)
			return current->findCatalog(path);
		else return current;
		
	}
	else if (head->name == path.substr(0,pathBegin))
	{
		temp = head;
		path = path.substr(path.find_first_of(pathSeparator) + 1, path.length() - path.find_first_of(pathSeparator));
	}

	while (!path.empty())
	{
		const size_t end = path.find_first_of(pathSeparator);
		const std::string currName = path.substr(0, end);


		if (end == std::string::npos)
		{
			if (readLastWord)
			{
				path = path.substr(end + 1, path.length() - end);
				temp = temp->findCatalog(path);
			}
			return temp;
		}


		path = path.substr(end + 1, path.length() - end);

		temp = temp->findCatalog(currName);
		if (!temp)
		{
			temp = nullptr;
			throw std::runtime_error("PATH NOT FOUND");
		}
	}
}
//Processing commands

void FileSystem::workMD(std::string &command)
{
	//Making and pushing catalog
	Catalog* processedCatalog = readPath(command, false);
	if (!processedCatalog)
		return;
	
	if (!processedCatalog->isCatalogCreatable(command))
		return;
	
	Catalog* tempCat = new Catalog;
	tempCat->createCatalog(command);
	processedCatalog->push(tempCat);
}

void FileSystem::workCD(std::string& command)
{
	//Change the current folder
	if (command.empty())
		throw std::runtime_error("Not enough arguments for: CD");
	if (command == "c:")
	{
		current = head;
		return;
	}
	Catalog* processedCatalog = readPath(command, true);
	if (!processedCatalog)
		return;
	current = processedCatalog;
}

void FileSystem::workRD(std::string& command)
{
	//deletion of cat if its empty
	Catalog* temp = readPath(command, false);
	if (command.empty()||!temp)
	{
		throw std::runtime_error("Can`t delete current directory");
		return;
	}
	temp = temp->findCatalog(command);
	if (temp->isSubCatalog(current))
		throw std::runtime_error("Can`t delete current directory");
	if (temp == current)
		return;
	else temp->deleteEmptyCatalog();
}

void FileSystem::workMF(std::string& command)
{
	//Making file
	Catalog* temp = readPath(command, false);
	if (!temp || !temp->isFilePushable(command))
		throw std::runtime_error("Can`t create file - Directory exists");
	File* file = new File;
	file->createFile(command);
	temp->push(file);
	return;
}
inline void FileSystem::workMDL(std::string& command)
{
	workLink(command, Link::LinkType::dynamic);
}

inline void FileSystem::workMHL(std::string& command)
{
	workLink(command, Link::LinkType::hard);
}
void FileSystem::workLink(std::string& command, Link::LinkType lType)
{
	//Making link
	auto paths = splitPaths(command);
	if (paths.first.empty() || paths.second.empty())
		return;


	//something was wrong,i cannot call find inside paths.first.substr
	const size_t lastSeparatorIndex = paths.first.find_last_of(pathSeparator);
	if (lastSeparatorIndex == std::string::npos)
		return;


	 std::string pathName = paths.first.substr(0, lastSeparatorIndex);
	 std::string objName = paths.first.substr(lastSeparatorIndex + 1, paths.first.length() - lastSeparatorIndex);
	if (objName.find(".") == std::string::npos)//If the word has no dots in its name, it is a cat
	{
		Catalog* linkedCat = readPath(paths.first, true);
		Catalog* linkDestination = readPath(paths.second, true);
		Link* link = new Link;
		link->createLink(lType, linkedCat, pathName);
		linkDestination->push(link);
	}
	else {

		File* linkedFile = readPath(paths.first, false)->findFile(objName);
		auto linkDestination = readPath(paths.second, true);
		Link* link = new Link;
		link->createLink(lType, linkedFile, pathName);
		linkDestination->push(link);

	}
}
void FileSystem::workDEL(std::string& command)
{
	//deletion of file
	Catalog* temp = readPath(command, false);
	if (!temp)
		return;
	File* file = temp->findFile(command);
	if (!file)
		throw std::runtime_error("File was not found");
	file->deleteFile();
	
}

void FileSystem::workDELTREE(std::string& command)
{
	//deleting a cat with all internals
	Catalog* thisCat = readPath(command, true);
	if (!thisCat)
		return;
	if (thisCat->isSubCatalog(current))
		return;
	thisCat->deleteTree();
}

void FileSystem::workMOVE(std::string& command)
{
	//Moving file or cat
	auto paths = splitPaths(command);
	Catalog* source = readPath(paths.first, false);
	Catalog* dest = readPath(paths.second, true);
	if (!dest || !source)
	{
		dest = source = nullptr;
		throw std::runtime_error("Path not found");
	}
	if (paths.first.find(".") == std::string::npos)//If the word has no dots in its name, it is a cat
	{
		source = source->findCatalog(paths.first);
		source->moveCatalog(dest);
		source->updateReferences();
	}
	else {
		File* file = source->findFile(paths.first);
		file->moveFile(dest);
		file->updateReferences();
	}
}
void FileSystem::workCOPY(std::string& command)
{
	//Copying file or cat with all internals.Copied file will have no links
	auto paths = splitPaths(command);
	Catalog* source = readPath(paths.first, false);
	Catalog* dest = readPath(paths.second, true);
	if (!dest || !source)
	{
		dest = source = nullptr;
		throw std::runtime_error("Path not found");
	}
	if (paths.first.find(".") == std::string::npos)
	{
		//copying all internals into temp,then pushing temp into destination
		source = source->findCatalog(paths.first);
		Catalog* temp = new Catalog;
		temp->createCatalog(source->name);
		cpyCat(temp,source);
		dest->push(temp);
		//source->updateReferences();
	}
	else {
		File* file = source->findFile(paths.first);
		file->cpyFile(dest);
	
	}
}

void FileSystem::print()
{
	head->print(0);
}


void FileSystem::cpyCat(Catalog* dest, Catalog* source)
{
	if (dest == nullptr || source == nullptr)
		return;
	//Copying all files,links
	for (size_t i = 0; i < source->files.size(); i++)
	{
		source->files[i]->cpyFile(dest);
	}
	for (size_t i = 0; i < source->links.size(); i++)
	{
		source->links[i]->cpyLink(dest);
	}
	//Recursively copy Catalogs
	for (size_t i = 0; i < source->subCatalogs.size(); i++)
	{
		if (source[i].isEmpty())
		{
			Catalog* temp = new Catalog;
			temp->createCatalog(source->subCatalogs[i]->name);
			dest->push(temp);
		}
		else {
			Catalog* temp = new Catalog;
			temp->createCatalog(source->subCatalogs[i]->name);
			dest->push(temp);
			cpyCat(temp, source->subCatalogs[i]);
		}
	}
}



bool FileSystem::processFile(const std::string inputFileName)
{
	if (inputFileName.empty())
		return false;
	std::ifstream file(inputFileName);
	if (!file)
		return false;
	std::string fileLine;
	try {
		while (!file.eof())
		{
			std::getline(file, fileLine);
			inputCommand(fileLine);
		}
	}
	catch (...)
	{
		file.close();
		return false;
	}
	file.close();
	return true;

}

bool FileSystem::writeFile(const std::string outputFileName)
{
	if (outputFileName.empty())
		return false;
	freopen(outputFileName.c_str(), "w", stdout);
	print();
	fclose(stdout);
	freopen("CON", "w", stdout);
	return true;
}