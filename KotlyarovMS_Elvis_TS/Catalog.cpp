#include "FileSystem.hpp"

void Catalog::print(size_t depth) const
{
	std::string treeBuilderString;
	for (size_t i = 1; i < depth; i++)
	{
		treeBuilderString += "| ";
	}
	if (depth == 0)
	{
		std::cout << name << std::endl;
	}
	else std::cout << treeBuilderString << "|_" << name << std::endl;
	for (size_t i = 0; i < files.size(); i++)
	{
		std::cout << treeBuilderString << "| |_";
		files[i]->print();
		std::cout << std::endl;
	}
	for (size_t i = 0; i < links.size(); i++)
	{
		std::cout << treeBuilderString << "| |_";
		links[i]->print();
		std::cout << std::endl;
	}

	for (size_t i = 0; i < subCatalogs.size(); i++)
	{

		subCatalogs[i]->print(depth + 1);

	}

}

bool Catalog::isSubCatalog(Catalog* cat) const 
{
	Catalog* temp = cat;
	//finding this above cat
	while (temp != nullptr)
	{
		if (temp == this)
			return true;
		temp = temp->parent;
	}
	return false;
}

bool Catalog::isEmpty() const
{
	return (links.empty() && files.empty() && subCatalogs.empty()) ? true : false;
}

bool Catalog::createCatalog(const std::string& name)
{
	this->name = name;
	return true;
}

bool Catalog::isFilePushable(const std::string& fname) const
{
	if (fname.empty())
		return false;
	//If file name has no extention
	if (size_t lastCommaPos = fname.find_last_of("."); lastCommaPos == std::string::npos)
		return false;
	else
	{
		//if file name does not match the condition
		if (fname.length() - lastCommaPos - 1 > MAX_F_EXT_NAME || lastCommaPos >= MAX_NAME_LEN)
			return false;
		for (size_t i = 0; i < files.size(); i++)
		{
			//There must not be another files with equal name
			if (files[i]->name == fname)
				return false;
		}
		for (size_t i = 0; i < subCatalogs.size(); i++)
		{
			//There must not be another catalogs with equal name
			if (subCatalogs[i]->name == fname.substr(0, lastCommaPos))
				return false;
		}
	}
	return true;
}

bool Catalog::isCatalogCreatable(const std::string& catName) const
{
	if (catName.empty())
		return false;
	//Checking if the filename and catalog name match
	for (size_t i = 0; i < files.size(); i++)
	{
		if (catName == files[i]->name.substr(0, files[i]->name.find_last_of(".")))
		{
			throw std::runtime_error("Can`t create directory - File exists");
		}
	}
	return true;
}

bool Catalog::moveCatalog(Catalog* into)
{
	if (!into)
		return false;
	if (!isHardLinked())
		return false;
	//If there was no head Catalog
	if (!parent)
	{
		into->push(this);
		return true;
	}
	else {//Else deleting refs

		for (size_t i = 0; i < parent->subCatalogs.size(); i++)
		{
			if (parent->subCatalogs[i] == this)
			{
				parent->subCatalogs.erase(parent->subCatalogs.begin() + i);
				into->push(this);
				return true;
			}
		}
	}
	return false;
	
}


void Catalog::updateReferences()
{
	//updating links paths for all files
	for (size_t i = 0; i < files.size(); i++)
	{
		files[i]->updateReferences();
	}
}

bool Catalog::push(Link* link)
{
	if (!link)
		return false;
	link->headCatalog = this;
	links.push_back(link);
	return true;
}

bool Catalog::push(File* file)
{
	if (!file)
		return false;
	files.push_back(file);
	file->catalog = this;
	return true;
}

bool Catalog::push(Catalog* catalog)
{
	if (!catalog)
		return false;
	subCatalogs.push_back(catalog);
	catalog->parent = this;
	return true;
}

void Catalog::deleteFile(const std::string& fName)
{
	for (size_t i = 0; i < files.size(); i++)
	{
		if (fName == files[i]->name)
		{
			files[i]->deleteFile();
		}
	}
}


void Catalog::deleteLink(Link::LinkType & type, const std::string& linkPath)
{
	for (size_t i = 0; i < links.size(); i++)
	{
		if (links[i]->path == linkPath && links[i]->type == type)
		{
			links[i]->deleteLink();
		}
	}
	
}


void Catalog::deleteEmptyCatalog()
{
	if (isEmpty())
	{
		deleteCatalog();
	}
}
void Catalog::deleteCatalog()
{
	if (parent != nullptr)//Deletion of this ref at parent cat
	{
		for (size_t i = 0; i < parent->subCatalogs.size(); i++)
		{
			if (parent->subCatalogs[i] == this)
			{
				parent->subCatalogs.erase(parent->subCatalogs.begin() + i);
			}

		}
	}
	for (size_t i = 0; i < files.size(); i++)
	{
		files[i]->deleteFile();
	}
	for (size_t i = 0; i < links.size(); i++)
	{
		links[i]->deleteLink();
	}
	//Recursively deleting nested Catalogs
	for (size_t i = 0; i < subCatalogs.size(); i++)
	{
		subCatalogs[i]->deleteCatalog();
	}
	delete this;//It is quite ok
}


void Catalog::deleteTree()
{
	if (isTreeDeletable())
	{
		deleteCatalog();
	}
	
	
}
bool Catalog::isTreeDeletable()
{
	for (size_t i = 0; i < files.size(); i++)
	{
		if (!files[i]->isDeletable())
			throw std::runtime_error("File has hardLink");
	}

	if (!isHardLinked())//Check Catalog for hardlink
		return false;
	for (size_t i = 0; i < subCatalogs.size(); i++)
	{
		if (!subCatalogs[i]->isEmpty())
		{
			if (!subCatalogs[i]->isTreeDeletable())
				throw std::runtime_error("Catalog has hardLink");
		}
	}
	return true;
}

bool Catalog::isHardLinked() const
{
	for (size_t i = 0; i < linksOnCatalog.size(); i++)
		if (linksOnCatalog[i]->type == Link::LinkType::hard)
			return false;
	return true;
}

File* Catalog::findFile(const std::string& fName) const 
{
	for (size_t i = 0; i < files.size(); i++)
	{
		if (fName == files[i]->name)
			return files[i];
	}
	return nullptr;
}

Catalog* Catalog::findCatalog(const std::string& catName) const
{
	for (size_t i = 0; i < subCatalogs.size(); i++)
	{
		if (catName == subCatalogs[i]->name)
			return subCatalogs[i];
	}
	return nullptr;
}

std::string Catalog::getPath()
{
	Catalog* temp = this;
	std::string path;
	//As long as there is a Catalog above the tree, concatenate directory names
	while (temp)
	{
		path = temp->name + pathSeparator + path;
		temp = temp->parent;
	}
	path.pop_back();//Removing an unnecessary "\" character at the end of a string
	return path;
}