
#include "utils.h"

#include "baserfsmodel.h"


BaseRFSModel::BaseRFSModel(QObject * parent)
    :QAbstractItemModel(parent)
{
    this->init();
}

BaseRFSModel::~BaseRFSModel()
{
}
bool BaseRFSModel::init()
{
    root = new RFSDirNode();
    root->name = "";
    root->path = "/";
    root->pstatus = POP_NO_NEED_NO_DATA;
    root->status = POP_NO_NEED_WITH_DATA;
    root->size = 0;
    root->md5 = "";

    return true;
}
QModelIndex BaseRFSModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_ASSERT(row >=0 && column >= 0);
    RFSDirNode *parentItem = NULL;
    if(parent.isValid()) {
        parentItem = static_cast<RFSDirNode*>(parent.internalPointer());
    }else{
        parentItem = this->root;
    }
    RFSDirNode *childItem = 0;
    if(row < parentItem->children.count()) {
        childItem = parentItem->children.at(row);
        return this->createIndex(row, column, childItem);
    }else{
        return QModelIndex();
    }
    return QModelIndex();
}
QModelIndex BaseRFSModel::parent(const QModelIndex &child) const
{
    RFSDirNode *parentItem = 0;
    RFSDirNode *childItem = 0;
    if(child.isValid()) {
        childItem = static_cast<RFSDirNode*>(child.internalPointer());
        parentItem = childItem->parent;
        if(parentItem == this->root) {
            return QModelIndex();
        }else{
            return this->createIndex(parentItem->rowNum, 0, parentItem);
        }
    }else{
        return QModelIndex();
    }
    return QModelIndex();
}
bool BaseRFSModel::hasChildren(const QModelIndex &parent) const
{
    RFSDirNode *parentItem = 0;
    if(parent.isValid()) {
        parentItem = static_cast<RFSDirNode*>(parent.internalPointer());
        if(parentItem->children.count() > 0) return true;
        else return false;
    }else{
        return false;
    }
    return true;
}
int BaseRFSModel::columnCount(const QModelIndex &parent) const
{
    return this->column;
}
int BaseRFSModel::rowCount(const QModelIndex &parent) const
{
    RFSDirNode *parentItem = 0;
    if(parent.isValid()) {
        parentItem = static_cast<RFSDirNode*>(parent.internalPointer());
        Q_ASSERT(parentItem != NULL);
        return parentItem->children.count();
    }else{
        return 0;
    }
    return 0;
}

QVariant BaseRFSModel::data(const QModelIndex &index, int role) const 
{
    if(role != Qt::DisplayRole) return QVariant();

    RFSDirNode *item = 0;
    if(index.isValid()) {
        item = static_cast<RFSDirNode*>(index.internalPointer());
        Q_ASSERT(item != NULL);
        return item->name;
    }else{
        return QVariant();
    }
    return QVariant();
}
bool BaseRFSModel::setData(const QModelIndex &index, const QVariant &value, int role) const
{
    if(role != Qt::DisplayRole) return false;
    if(!index.isValid()) return false;

    RFSDirNode *item = 0;
    item = static_cast<RFSDirNode*>(index.internalPointer());
    item->name = value.toString();
    return true;
}
Qt::ItemFlags BaseRFSModel::flags(const QModelIndex &index) const
{

    return Qt::ItemIsSelectable | Qt::ItemIsEditable ;
}

QVariant BaseRFSModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QVariant();
}

bool BaseRFSModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    return true;
}
QMimeData * BaseRFSModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData * data = NULL;

    return data;
}
QStringList BaseRFSModel::mimeTypes() const
{
    QStringList types;

    return types;
}
Qt::DropActions BaseRFSModel::supportedDropActions() const
{
    return Qt::MoveAction | Qt::CopyAction; // | Qt::LinkAction 
}

bool BaseRFSModel::insertRows(int row, int count, const QModelIndex &parent)
{
    return true;
}
bool BaseRFSModel::removeRows(int row, int count, const QModelIndex &parent)
{
    return true;
}

bool BaseRFSModel::submit()
{
    return true;
}
void BaseRFSModel::revert()
{

}
