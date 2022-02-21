#define _CRT_SECURE_NO_WARNINGS
#pragma once
#include <vector>
#include <list>
#include <fstream>
#include <map>
#include <string>
#include <iostream>
//FileName parameters
constexpr uint8_t MAX_NAME_LEN      = 8;
constexpr uint8_t MAX_F_EXT_NAME    = 3;


const std::string pathSeparator     = "\\";//С++17 does not allow to use constexpr strings

//Instructions for FileSystem
namespace FScommand_str
{
	const std::string MD           = "md";
	const std::string CD           = "cd";
	const std::string RD           = "rd";
	const std::string DELTREE      = "deltree";
	const std::string MF           = "mf";
	const std::string MHL          = "mhl";
	const std::string MDL          = "mdl";
	const std::string DEL          = "del";
	const std::string СOPY         = "copy";
	const std::string MOVE         = "move";
}

//Enumerations definition

enum class FScommand : int8_t
{//               Опция                                                                        Формат
	undefined = 0x0,
	MD,     //Создать каталог                                                              MD [drive:]path 
	CD,      //Сменить текущий каталог                                                     CD [drive:][path]
	RD,      //Удалить каталог если он пустой                                              RD [drive:]path
	DELTREE, //Удаляет каталог включая все подкаталоги                                     DELTREE [drive:]path
	MF,      //Создать файл                                                                MF [drive:]path 
	MHL,     //Создает hard link на файл/каталог и помещает link в заданный каталог        MHL [drive:]source [drive:]destination
	MDL,
	DEL,     //Удалить файл или линк                                                       DEL [drive:]path
	СOPY,    //Копирует существующий каталог/файл/линк в заданный каталог                  COPY [drive:]source [drive:]destination
	MOVE,    //Перемещает существующий каталог/файл/линк в заданный каталог                MOVE [drive:]source [drive:]destination
};




struct  File;
struct  Catalog;
struct Link
{
	enum class LinkType : int8_t
	{
		undefined = 0x0,
		hard,
		dynamic
	};
	enum class LinkWith : int8_t
	{
		undefined = 0x0,
		catalog,
		file
	};

	LinkType               type;
	LinkWith               linkWith;
	File                  *linkedFile;
	Catalog               *linkedCatalog;
	Catalog               *headCatalog;
	std::string            path;//LinkName

	Link() :type(LinkType::undefined),linkWith(LinkWith::undefined), linkedFile(nullptr), headCatalog(nullptr), linkedCatalog(nullptr) {}
	Link(const Link& link) :type(link.type), linkWith(link.linkWith), linkedFile(link.linkedFile), linkedCatalog(link.linkedCatalog), headCatalog(nullptr), path(link.path) {}
	~Link() 
	{
		linkedFile       = nullptr;
		linkedCatalog    = nullptr;
		headCatalog      = nullptr;
	}

	void print() const ;
	//creating links(no mem allocation)
	bool createLink(LinkType& type, File* file, const std::string& path);
	bool createLink(LinkType& type, Catalog* catalogToLink, const std::string& path);
	void updateRefences();
	//link deletion(with mem deallocation)
	void deleteLink();
	void cpyLink(Catalog* dest);//copying link into dest catalog
	friend bool operator==(const Link& l1, const Link& l2);
};


struct File
{
	Catalog                 *catalog;
	std::string             name;
	std::vector <Link*>     links;

	File():catalog(nullptr) {}
	File(const File& file) :name(file.name),catalog(nullptr) {}
	~File()
	{
		for (size_t i = 0; i < links.size(); i++)
		{
			links[i] = nullptr;
		}
		links.clear();
		catalog = nullptr;
	}

	void print() const;
	bool isDeletable() const;
	std::string getPath() const;
	//creating file(no mem allocation)
	void createFile(const std::string& fName);
	//file deletion (with nen deallocation)
	void deleteFile();

	bool moveFile(Catalog* dest);
	//Upd link paths after file motion
	bool cpyFile(Catalog* dest);
	bool updateReferences();
	
};


struct Catalog
{
	std::string                name;
	std::vector <File*>        files;
	std::vector <Link*>        links;
	std::vector <Link*>        linksOnCatalog;
	std::vector <Catalog*>     subCatalogs;
	Catalog                   *parent;

	Catalog():parent(nullptr) {}
	~Catalog() {
		for (size_t i = 0; i < files.size(); i++)
		{
			if (files[i])
			{
				delete files[i];
				files[i] = nullptr;
			}
		}
		files.clear();
		for (size_t i = 0; i < links.size(); i++)
		{
			if (links[i])
			{
				delete links[i];
				links[i] = nullptr;
			}

		}
		links.clear();
		parent = nullptr;
	}

	void print(size_t depth) const;

	bool isSubCatalog(Catalog* cat) const;
	bool isEmpty() const;
	//Checking if a file can be added to the cat
	bool isFilePushable(const std::string& fname) const;
	bool isCatalogCreatable(const std::string& catName) const;
	bool isHardLinked() const;

	bool createCatalog(const std::string& name);
	bool moveCatalog(Catalog* into);

	bool push(Link* link);
	bool push(File* file);
	bool push(Catalog* catalog);

	void updateReferences();
	std::string getPath();
	
	void deleteFile(const std::string& fName);
	void deleteLink(Link::LinkType& type, const std::string& linkName);
	
	File* findFile(const std::string& fName) const ;
	Catalog* findCatalog(const std::string& catName) const;
	

	//Deletion
	
	void deleteEmptyCatalog();
	void deleteCatalog();

	void deleteTree();
	bool isTreeDeletable();
	
};






class FileSystem
{
private:
	Catalog                   *head;
	Catalog                   *current;
public:
	FileSystem() :current(head), head(new Catalog) { head->name = "c:"; }
	~FileSystem()
	{
		current = nullptr;
		delete head;
		head = nullptr;
	}
	void print();
	void inputCommand(std::string command);

	bool processFile(const std::string inputFileName);
	bool writeFile(const std::string outputFileName);

	
private:
	void workMD(std::string &command);
	void workCD(std::string &command);
	void workRD(std::string &command);
	void workDELTREE(std::string &command);
	void workMF(std::string &command);
	inline void workMHL(std::string &command);
	inline void workMDL(std::string& command);
	void workDEL(std::string &command);
	void workCOPY(std::string &command);
	void workMOVE(std::string &command);
	void workLink(std::string& command, Link::LinkType lType);

	
	FScommand getCommand(std::string& command);
	std::pair<std::string, std::string> splitPaths(const std::string& paths);
	Catalog* readPath(std::string &path,const bool readLastWord);
	

	inline void toLowerStr(std::string& str);
	void cpyCat(Catalog* dest, Catalog* source);
};