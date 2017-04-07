#include "BankModel.h"

#include "RomInfo.h"

BankModel::BankModel(QObject *parent)
    : QAbstractListModel(parent)
    , m_bank(EmsCart::InvalidBank)
{
    m_roleNames.insert(Qt::UserRole, "title");
    m_roleNames.insert(Qt::UserRole + 1, "size");
    m_roleNames.insert(Qt::UserRole + 2, "offset");
}

BankModel::~BankModel()
{
}

EmsCart::Bank BankModel::bank() const
{
    return m_bank;
}

QHash<int, QByteArray> BankModel::roleNames() const
{
    return m_roleNames;
}

int BankModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return m_titles.count();
}

QVariant BankModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    if (row >= m_titles.count()) {
        return QVariant();
    }

    switch (role) {
        case Qt::UserRole:
            return QVariant(m_titles.value(row));

        case Qt::UserRole + 1:
            return QVariant(m_sizes.value(row));

        case Qt::UserRole + 2:
            return QVariant(m_offsets.value(row));

        default:
            return QVariant();
    }
}

void BankModel::setBank(EmsCart::Bank bank)
{
    m_bank = bank;

    // Disconnect all signals
    disconnect(EmsCart::instance(), 0, this, 0);

    switch (bank) {
        case EmsCart::BankOne:
            connect(EmsCart::instance(), &EmsCart::bankOneChanged, this, &BankModel::refreshData);
            break;

        case EmsCart::BankTwo:
            connect(EmsCart::instance(), &EmsCart::bankTwoChanged, this, &BankModel::refreshData);
            break;

        default:
            // Avoid compiler whining
            break;
    }

    refreshData();
}

void BankModel::refreshData()
{
    beginResetModel();

    m_titles.clear();
    m_sizes.clear();
    m_offsets.clear();

    endResetModel();

    QList<RomInfo *> roms;
    switch (m_bank) {
        case EmsCart::BankOne:
            roms = EmsCart::instance()->bankOne();
            break;

        case EmsCart::BankTwo:
            roms = EmsCart::instance()->bankTwo();
            break;

        default:
            return;
    }

    if (roms.isEmpty()) {
        return;
    }

    beginInsertRows(index(0), 0, roms.count() - 1);
    for (RomInfo *rom : roms) {
        m_titles.append(rom->title());
        m_sizes.append(rom->romSize());
        m_offsets.append(rom->offset());
    }
    endInsertRows();
}
