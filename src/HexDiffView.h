#pragma once
#include <QTableView>

class HexDiffModel;

class HexDiffView : public QTableView
{
    Q_OBJECT
public:
    explicit HexDiffView(QWidget *parent = nullptr);

    void setHexModel(HexDiffModel *model);
    void refreshMetrics();
    void goToOffset(qint64 offset);

signals:
    void offsetUnderCursor(qint64 offset);
    void selectionRangeChanged(qint64 startOffset, qint64 endOffset);  // <-- new

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private slots:
    void onSelectionChanged();                                        // <-- new

private:
    void configure();

    HexDiffModel *m_hexModel = nullptr;
};