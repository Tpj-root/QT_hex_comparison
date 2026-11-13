// HexDiffView.h
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

protected:
    void mouseMoveEvent(QMouseEvent *event) override;

private:
    void configure();

    HexDiffModel *m_hexModel = nullptr;
};