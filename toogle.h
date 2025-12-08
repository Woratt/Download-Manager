#ifndef TOOGLE_H
#define TOOGLE_H

#include <QObject>
#include <QWidget>
#include <QCheckBox>
#include <QPaintEvent>
#include <QPainter>

class Toogle : public QCheckBox
{
    Q_OBJECT
private:
    QColor m_bgColor;
    QColor m_circleColor;
    QColor m_activeColor;

public:
    Toogle(int width, const QColor& bg_color, const QColor& circle_color, const QColor& active_color, QWidget* parent = nullptr);
    void paintEvent(QPaintEvent*) override;
};

#endif // TOOGLE_H
