#include "CrossWidget.h"

CrossWidget::CrossWidget(QWidget *parent)
    : QWidget(parent)
{
  ui.setupUi(this);

  // init values
  mouseLeftButton = false;
  valueX = 0;
  valueY = 0;
  doubleX = 0.0;
  doubleY = 0.0;
  zeroZoneX = 0;
  zeroZoneY = 0;
  zeroZone0 = 0;

  crossWidth  = ui.widgetCross->width();
  crossHeight = ui.widgetCross->height();

  crossX = (int)(crossWidth/2)+1;
  crossY = (int)(crossHeight/2)+1;
}

CrossWidget::~CrossWidget() {
}


// PUBLIC SLOTS

void CrossWidget::setZeroZoneX(int pixel) {
  zeroZoneX = abs(pixel);
  emit zeroZoneXChanged(zeroZoneX);
}

void CrossWidget::setZeroZoneY(int pixel) {
  zeroZoneY = abs(pixel);
  emit zeroZoneYChanged(zeroZoneY);
}

void CrossWidget::setZeroZone0(int pixel) {
  zeroZone0 = abs(pixel);
  emit zeroZone0Changed(zeroZone0);
}


// PRIVATE

void CrossWidget::calcMousePos(QMouseEvent* e) {
  double posX, posY;

  // calc X-value
  if (e->x() < 0) {
    posX = 0;
  }
  else if (e->x() > crossWidth) {
    posX = crossWidth;
  }
  else {
    posX = e->x();
  }
  valueX = (int)(posX-crossX);

  // calc Y-value
  if (e->y() < 0) {
    posY = 0;
  } else if (e->y() > crossHeight) {
    posY = crossHeight;
  } else {
    posY = e->y();
  }
  valueY = -(int)(posY-crossY);

  doZeroZone();
  setDoubleValues();
}


void CrossWidget::doZeroZone(void) {
  if (abs(valueX) <= zeroZone0
      && abs(valueY) <= zeroZone0) {
    valueX = 0;
    valueY = 0;
  } else {
    if (abs(valueX) <= zeroZoneX) {
      valueX = 0;
    }
    if (abs(valueY) <= zeroZoneY) {
      valueY = 0;
    }
  }
}


void CrossWidget::setDoubleValues() {
  doubleX = (double)valueX / crossX;
  doubleY = (double)valueY / crossY;
}


// PROTECTED

void CrossWidget::mouseMoveEvent(QMouseEvent* e) {
  calcMousePos(e);
  emit newValue(doubleX, doubleY, mouseLeftButton);
}


void CrossWidget::mousePressEvent(QMouseEvent* e) {
  if (e->buttons() == Qt::LeftButton){
    mouseLeftButton = true;
    calcMousePos(e);
    emit newValue(doubleX, doubleY, mouseLeftButton);
  }
}


void CrossWidget::mouseReleaseEvent(QMouseEvent* e) {
  mouseLeftButton = false;
}

