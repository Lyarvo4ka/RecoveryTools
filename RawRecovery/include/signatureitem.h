#pragma once

#include "treeitem.h"
#include "raw/AbstractRaw.h"
#include <QAbstractItemModel>
#include <QFileIconProvider>


enum class SignatureItemType : int
{
	kSignatureItem, kCategoryItem
};

class SignatureAdapter
{
	QString name_;
protected:
	SignatureItemType signItemType_;

public:
	void setName(const QString& name)
	{
		name_ = name;
	}
	QString getName() const
	{
		return name_;
	}
	SignatureItemType getItemType() const
	{
		return signItemType_;
	}
	virtual RAW::FileStruct::Ptr createFileStruct() = 0;
};

class CategoryFolderAdapter
	: public SignatureAdapter
{
public:
	CategoryFolderAdapter()
	{
		signItemType_ = SignatureItemType::kCategoryItem;
	}
	RAW::FileStruct::Ptr createFileStruct() override
	{
		return nullptr;
	}
};

class SignatureItemAdapter
	: public SignatureAdapter
{
	RAW::FileStruct file_struct_;
public:
	SignatureItemAdapter(const RAW::FileStruct & file_struct)
		: file_struct_(file_struct)
	{
		signItemType_ = (SignatureItemType::kSignatureItem);
		setName(QString::fromLocal8Bit(file_struct.getName().c_str()));
	}
	RAW::FileStruct::Ptr createFileStruct() override
	{
		return file_struct_.clone();
	}
};

class SignatureItem 
	: public TreeItem<SignatureAdapter>
{

public:
	explicit SignatureItem(std::unique_ptr<SignatureAdapter> adapter, SignatureItem* parentItem = nullptr)
		:TreeItem(std::move(adapter), parentItem)
	{
		
	}

	SignatureItem* findWithName(const QString & name_txt)
	{
		for (auto i = 0; i < this->childCount(); ++i)
		{
			auto treeItem = this->child(i);
			if (treeItem->getAdapter()->getName().compare(name_txt) == 0)
			{
				auto signItem = static_cast<SignatureItem*>(treeItem);
				return signItem;
			}
		}
		return nullptr;
	}
	Qt::CheckState getCheckStateFromChilds()
	{
		auto checkState = this->getCheckState();

		if (this->childCount() > 0)
		{
			checkState = child(0)->getCheckState();
			for (auto i = 1; i < this->childCount(); ++i)
			{
				if (checkState != child(i)->getCheckState())
					return Qt::PartiallyChecked;
			}
		}
		return checkState;

		//auto checkState = itemIter->getCheckState();
		//while (++itemIter != childItems_.end())
		//{
		//	auto nextState = itemIter->getCheckState();
		//	if (checkState != nextState)
		//		return Qt::PartiallyChecked;

		//}
	}

	void upadateCheckParents()
	{
		auto parent = this->parentItem();
		while (parent != nullptr)
		{
			if (parent->getAdapter()->getItemType() == SignatureItemType::kCategoryItem)
			{
				auto signatureItem = dynamic_cast<SignatureItem*>(parent);
				auto check_state = signatureItem->getCheckStateFromChilds();
				parent->setCheckState(check_state);
			}
			parent = parent->parentItem();
		}
		
	}
	void updateCheckChilds(const Qt::CheckState checkState)
	{
		if (this->getAdapter()->getItemType() == SignatureItemType::kCategoryItem )
			for (auto i = 0; i < this->childCount(); ++i)
				this->child(i)->setCheckState(checkState);
	}
};

//using SignatureItem = TreeItem< SignatureAdapter>;


class SignatureTreeModel
	: public QAbstractItemModel
{
	Q_OBJECT
private:
	SignatureItem* rootItem_;
	QFileIconProvider iconProvider_;
public:
	explicit SignatureTreeModel(SignatureItem* root_item, QObject* parent = nullptr)
		:QAbstractItemModel(parent)
		, rootItem_(root_item)
	{

	}
	~SignatureTreeModel()
	{
		delete rootItem_;
	}


	QVariant data(const QModelIndex& index, int role) const override
	{
		if (!index.isValid())
			return QVariant();

		auto item = static_cast<SignatureItem*>(index.internalPointer());

		if (role == Qt::DisplayRole)
		{
			auto tmp_name = item->getAdapter()->getName();
			return tmp_name;
		}
		if (index.column() == 0)
		{
			if (role == Qt::CheckStateRole)
				return item->getCheckState();
			else
				if (role == Qt::DecorationRole)
				{
					switch (item->getAdapter()->getItemType())
					{
					case SignatureItemType::kCategoryItem:
						return iconProvider_.icon(QFileIconProvider::Folder);
					case SignatureItemType::kSignatureItem:
						return iconProvider_.icon(QFileIconProvider::File);

					}
				}
		}
		return QVariant();
	}
	bool setData(const QModelIndex& index, const QVariant& value, int role = Qt::EditRole) override
	{
		if (role == Qt::CheckStateRole)
		{
			auto signatureItem = toSignatureItem(index);
			if (signatureItem != nullptr)
			{
				auto checkState = static_cast<Qt::CheckState> (value.toInt());
				signatureItem->updateCheckChilds(checkState);
				signatureItem->setCheckState(checkState);
				signatureItem->upadateCheckParents();

				// update the whole tree views.
				emit dataChanged(QModelIndex(), QModelIndex());
				return true;
			}
		}
		return false;
	}
	SignatureItem* toSignatureItem(const QModelIndex& qModelIdex)
	{
		SignatureItem* signatureItem = nullptr;
		if (qModelIdex.isValid())
			signatureItem = static_cast<SignatureItem*>(qModelIdex.internalPointer());
		return signatureItem;
	}
	Qt::ItemFlags flags(const QModelIndex& index) const override
	{
		if (!index.isValid())
			return Qt::NoItemFlags;

		return QAbstractItemModel::flags(index)  | Qt::ItemIsUserCheckable ;

	}

	QVariant headerData(int section, Qt::Orientation orientation,
		int role = Qt::DisplayRole) const override
	{
		return QVariant();
	}

	QModelIndex index(int row, int column,
		const QModelIndex& parent = QModelIndex()) const override
	{
		if (!hasIndex(row, column, parent))
			return QModelIndex();

		SignatureItem* parentItem = nullptr;

		if (!parent.isValid())
			parentItem = rootItem_;
		else
			parentItem = static_cast<SignatureItem*>(parent.internalPointer());

		auto childItem = parentItem->child(row);
		if (childItem)
			return createIndex(row, column, childItem);
		return QModelIndex();
	}

	QModelIndex parent(const QModelIndex& index) const override
	{
		if (!index.isValid())
			return QModelIndex();

		auto childItem = static_cast<SignatureItem*>(index.internalPointer());
		auto parentItem = childItem->parentItem();

		if (parentItem == rootItem_)
			return QModelIndex();

		return createIndex(parentItem->row(), 0, parentItem);
	}

	int rowCount(const QModelIndex& parent = QModelIndex()) const override
	{
		SignatureItem* parentItem = nullptr;
		if (parent.column() > 0)
			return 0;

		if (!parent.isValid())
			parentItem = rootItem_;
		else
			parentItem = static_cast<SignatureItem*>(parent.internalPointer());

		return parentItem->childCount();
	}
	int columnCount(const QModelIndex& parent = QModelIndex()) const override
	{
		return /*(parent.isValid() && parent.column() != 0) ? 0 :*/ 1;
	}

};