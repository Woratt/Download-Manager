#include "toogle.h"

Toogle::Toogle(int width, const QColor& bgColor, const QColor& circleColor, const QColor& activeColor, QWidget* parent) :
    m_bgColor(bgColor), m_circleColor(circleColor), m_activeColor(activeColor), QCheckBox(parent) {
    setFixedSize(width, 10);
    setCursor(Qt::PointingHandCursor);
}


void Toogle::paintEvent(QPaintEvent * event){
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(Qt::NoPen);

    QRect rect(0, 0, width(), height());

    if(isChecked()){
        painter.setBrush(QBrush(m_activeColor));
        painter.drawRoundedRect(0, 0, rect.width(), height(), height() / 2, height() / 2);

        painter.setBrush(QColor(m_circleColor));
        painter.drawEllipse(width() - 10, 1, 8, 8);
    }else{
        painter.setBrush(QBrush(m_bgColor));
        painter.drawRoundedRect(0, 0, rect.width(), height(), height() / 2, height() / 2);

        painter.setBrush(QColor(m_circleColor));
        painter.drawEllipse(1, 1, 8, 8);
    }

    painter.end();
}
