#pragma once

#include "io/constants.h"
#include "utility.h"
#include <memory>

#include <filesystem>
namespace fs = std::filesystem;


#pragma warning(disable:4251)

namespace IO
{
	inline path_string getOnlyNameFromPath(const path_string &filePath)
	{
		fs::path fsPath(filePath); 
		return fsPath.stem().generic_wstring();
	}

	inline std::vector<path_string> split(const path_string& s, wchar_t delimiter)
	{
		std::vector<path_string> tokens;
		std::wstring token;
		std::wistringstream tokenStream(s);
		while (std::getline(tokenStream, token, delimiter))
		{
			tokens.emplace_back(token);
		}
		return tokens;
	}

	enum DATE_VAL_ORDER {nYYYY , nMM  , nDD  , nCOUNT };
	struct DateStruct
	{
		path_string year;
		path_string month;
		path_string day;
	};

	inline void moveToDateFolder(const path_string & filePath, const path_string & target_folder)
	{
		path_string dateName = getOnlyNameFromPath(filePath);
		constexpr auto tmp = L"-";
		const wchar_t * delimiter = L"-";
		auto dateTokents = split(dateName , *delimiter);
		DateStruct dateStruct;
		if (dateTokents.size() >=  nCOUNT)
		{
			dateStruct.year = dateTokents[nYYYY];
			dateStruct.month = dateTokents[nMM];
			dateStruct.day = dateTokents[nDD];

			path_string newPath = target_folder + addBackSlash(dateStruct.year) + addBackSlash(dateStruct.month) /*+ addBackSlash(dateStruct.day)*/;
			fs::create_directories(newPath);
			fs::path sourcePath(filePath);
			newPath += sourcePath.filename().generic_wstring();
			fs::rename(filePath, newPath);
		}


		
	}

	class DirectoryNode;

	class Node
	{
	private:
		DirectoryNode* parent_;
		path_string name_;
	public:
		Node(const path_string& name, DirectoryNode* parent = nullptr)
			:parent_(parent)
			, name_(name)
		{
		}
		~Node()
		{
			parent_ = nullptr;
		}

		void setName(const path_string& name)
		{
			this->name_ = name;
		}
		path_string getName() const
		{
			return name_;
		}

		void setParent(DirectoryNode* parent)
		{
			this->parent_ = parent;
		}

		DirectoryNode* getParent() const;
		path_string getFullPath();
	};

	class FileNode
		: public Node
	{
	public:
		typedef std::shared_ptr<FileNode> Ptr;
		static FileNode::Ptr CreateFileNode(const path_string& file_name)
		{
			return std::make_shared<FileNode>(FileNode(file_name));
		}
	private:

	public:
		FileNode(const path_string& file_name)
			: Node(file_name)
		{

		}

		path_string getExtension() const
		{
			fs::path file_path(this->getName());
			return file_path.extension().generic_wstring();
		}
	};

	class DirectoryNode
		: public Node
	{
	public:
		typedef std::shared_ptr<DirectoryNode> Ptr;
		static DirectoryNode::Ptr CreateDirectoryNode(const path_string& folder_name)
		{
			return std::make_shared<DirectoryNode>(DirectoryNode(folder_name));
		}
	private:
		std::list<DirectoryNode::Ptr> directories_;
		std::list<FileNode::Ptr> files_;
		mutable std::list<FileNode::Ptr>::const_iterator fileIter_;
		mutable std::list<DirectoryNode::Ptr>::const_iterator dirIter_;
	public:
		DirectoryNode(const path_string& directory_name)
			: Node(directory_name)
		{

		}

		void AddDirectory(const path_string& directory_name)
		{
			auto new_folder = CreateDirectoryNode(directory_name);
			new_folder->setParent(this);
			directories_.push_back(new_folder);
		}

		// tmp function

		void AddDirectory(DirectoryNode::Ptr directory_node)
		{
			directory_node->setParent(this);
			directories_.push_back(directory_node);
		}

		void AddFile(const path_string& file_name)
		{
			auto new_file = FileNode::CreateFileNode(file_name);
			new_file->setParent(this);
			files_.push_back(new_file);
		}

		FileNode::Ptr getFirstFile() const
		{
			fileIter_ = files_.begin();
			if (fileIter_ != files_.end())
				return *fileIter_;

			return nullptr;
		}
		FileNode::Ptr getNextFile() const
		{
			if (fileIter_ != files_.end())
				++fileIter_;
			if (fileIter_ != files_.end())
				return *fileIter_;
			return nullptr;
		}

		DirectoryNode::Ptr getFirstFolder() const
		{
			dirIter_ = directories_.begin();
			if (dirIter_ != directories_.end())
				return *dirIter_;
			return nullptr;
		}

		DirectoryNode::Ptr getNextFolder() const
		{
			if (dirIter_ != directories_.end())
			{
				++dirIter_;
				if (dirIter_ != directories_.end())
					return *dirIter_;
			}
			return nullptr;
		}

		//bool hasNext() const
		//{

		//}
	};

	//DirectoryNode* Node::getParent() const
	//{
	//	return parent_;
	//}

	//path_string Node::getFullPath()
	//{
	//	path_string full_path = name_;
	//	auto  pParent = parent_;
	//	while (pParent)
	//	{
	//		full_path.insert(0, addBackSlash(pParent->getName()));
	//		pParent = pParent->getParent();
	//	}
	//	return full_path;
	//}





}

