#include "FileSystem.hpp"

void File::print() const
{
	std::cout << name;
}

std::string File::getPath() const
{
	return catalog->getPath() + pathSeparator + name;
}

bool File::isDeletable() const
{
	for (size_t i = 0; i < links.size(); i++)
	{
		if (links[i]->type == Link::LinkType::hard)// File linked with hard link is undelitable
			return false;
	}
	return true;
}

void File::createFile(const std::string& fName)
{
	name = fName;
}

void File::deleteFile()
{
	if (!isDeletable())
		throw std::runtime_error("File cannot be deleted");
	//Deletion of all refs on this link from containers
	for (size_t i = 0; i < links.size(); i++)
	{
		if (links[i])
		{
			links[i]->deleteLink();
		}
	}
	for (size_t i = 0; i < catalog->files.size(); i++)
	{
		if (catalog->files[i] == this)
		{
			catalog->files.erase(catalog->files.begin() + i);
		}
	}
	catalog = nullptr;
	delete this;
}

bool File::moveFile(Catalog* dest)
{
	if (!dest)
		return false;
	//Deletion of refs in old headCatalog
	if (catalog)
	{
		for (size_t i = 0; i < catalog->files.size(); i++)
		{
			if (this == catalog->files[i])
			{
				catalog->files.erase(catalog->files.begin() + i);
				catalog = nullptr;
				break;
			}
		}
	}
	catalog = dest;
	return catalog->push(this);
}

bool File::cpyFile(Catalog* dest)
{
	//Copying a file does not imply link rebinding
	File* temp = new File(*this);
	return dest->push(temp);
}

bool File::updateReferences()
{
	//Refreshing links path after file transformations
	for (size_t i = 0; i < links.size(); i++)
	{
		links[i]->updateRefences();
	}
	return true;
}