#ifndef GIRMODEL_H
#define GIRMODEL_H

#include <QStandardItemModel>
#include <QMultiHash>

namespace MedicalUtils {
namespace AGGIR {
class NewGirScore;
}
}

namespace Gir {

struct GirItem
{
    GirItem(const QString &gr, const QString &subGr, const QString &tip,
            int scoreItem = -1, int scoreSubItem = -1) :
            group(gr), subgroup(subGr), tooltip(tip),
            girScoreItem(scoreItem),
            girScoreSubItem(scoreSubItem)
    {}

    QString group, subgroup, tooltip;
    int girScoreItem, girScoreSubItem;
};

class GirModel : public QStandardItemModel
{
    Q_OBJECT
public:
    explicit GirModel(QObject *parent = 0);
    ~GirModel();

    void setStringfiedGirScore(const QString &score);
    QString stringfiedGirScore() const;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const ;

    int rowCountWithChildren() const {return m_fullRowCount;}

Q_SIGNALS:
    void girCalculated(const int score);

private:
    QMultiHash<int, GirItem> m_groups;
    QStandardItem *m_score;
    MedicalUtils::AGGIR::NewGirScore *m_GirScore;
    int m_fullRowCount;
};

} // End namespace Gir


#endif // GIRMODEL_H
