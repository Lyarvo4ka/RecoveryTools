#ifndef BASEINDEX_H
#define BASEINDEX_H

#include "fs/DirectoryTree.h"

using FileSystem::NodeEntry;

class BaseIndex
{
public:
	BaseIndex()
		: checked_(Qt::Unchecked)
	{

	}
	//void setEntry(const NodeEntry & node_entry)
	//{
	//	node_entry_ = node_entry;
	//}
	//inline NodeEntry getEntry() const
	//{
	//	return node_entry_;
	//}
	inline Qt::CheckState checked() const
	{
		return checked_;
	}
	void setChecked(const Qt::CheckState checked = Qt::Unchecked)
	{
		checked_ = checked;
	}
private:
	Qt::CheckState checked_;
	//NodeEntry node_entry_;
};


#endif // BASEINDEX_H