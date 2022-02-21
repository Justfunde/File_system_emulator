#include "FileSystem.hpp"

void Link::print() const 
{
	switch (type)
	{
	case LinkType::hard:
		std::cout << "hlink";
		break;
	case LinkType::dynamic:
		std::cout << "dlink";
		break;
	default:
		return;
	}
	std::cout << "[" << path << "]";
}

bool Link::createLink(LinkType& type, File* file, const std::string& path)
{
	if (!file)
		return false;
	this->type = type;
	linkWith = LinkWith::file;
	this->path = file->getPath();
	//linking file and this obj
	linkedFile = file;
	file->links.push_back(this);
	return true;
}

bool Link::createLink(LinkType& type, Catalog* catalogToLink, const std::string& path)
{
	if (!catalogToLink)
		return false;
	this->type = type;
	this->path = path;
	linkWith = LinkWith::catalog;
	//linking Catalog and this obj
	linkedCatalog = catalogToLink;
	linkedCatalog->linksOnCatalog.push_back(this);
	return true;
}

void Link::deleteLink()
{
	//Deleting all refs on this link from containers
	switch (linkWith)
	{
	case Link::LinkWith::catalog:
		for (size_t i = 0; i < linkedCatalog->linksOnCatalog.size(); i++)
		{
			if (this == linkedCatalog->linksOnCatalog[i])
			{
				linkedCatalog->linksOnCatalog.erase(linkedCatalog->linksOnCatalog.begin() + i);
				break;
			}
		}
		break;
	case Link::LinkWith::file:
		for (size_t i = 0; i < linkedFile->links.size(); i++)
		{
			if (linkedFile->links[i] == this)
			{
				linkedFile->links.erase(linkedFile->links.begin() + i);
			}
		}
		break;
	case Link::LinkWith::undefined:
		return;
	}
	if(headCatalog)
		for (size_t i = 0; i < headCatalog->links.size(); i++)
		{
			if (headCatalog->links[i] == this)
			{
				headCatalog->links.erase(headCatalog->links.begin() + i);
			}
		}
	delete this; // StackOverFlow tells that it is ok, but i dont think so...
}

void Link::cpyLink(Catalog* dest)
{
	//Creating new link and pushing it into dest catalog
	Link* temp = new Link;
	switch (linkWith)
	{
	case Link::LinkWith::undefined:
		delete temp;
		return;
	case Link::LinkWith::catalog:
		temp->createLink(type, linkedCatalog, path);
		break;
	case Link::LinkWith::file:
		temp->createLink(type, linkedFile, path);
		break;
	}
	dest->push(temp);
}

bool operator==(const Link& l1, const Link& l2)
{
	return (l1.headCatalog == l2.headCatalog && l1.linkedFile == l2.linkedFile && l1.path == l2.path && l1.type == l2.type &&
		l1.linkedCatalog == l2.linkedCatalog) ? true : false;
}


void Link::updateRefences()
{
	switch (linkWith)
	{
	case Link::LinkWith::undefined:
		break;
	case Link::LinkWith::catalog:
		path = linkedCatalog->getPath();
		break;
	case Link::LinkWith::file:
		path = linkedFile->getPath();
		break;
	default:
		break;
	}
}