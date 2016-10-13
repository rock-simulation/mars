#include "JoystickWidget.h"

JoystickWidget::JoystickWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
  
  setAttribute(Qt::WA_DeleteOnClose);
  
  // connection for the CrossWidget
  connect(ui.crossW, SIGNAL(newValue(double, double, bool)), this, SLOT(updateCrossValue(double, double, bool)));
  connect(ui.vSliderLeft, SIGNAL(sliderMoved(int)), this, SLOT(setLeftSpeed(int)));
  connect(ui.vSliderRight, SIGNAL(sliderMoved(int)), this, SLOT(setRightSpeed(int)));
  connect(ui.doubleSpinBoxMaxSpeed, SIGNAL(valueChanged(double)), this, SLOT(setMaxSpeed(double)));
  
  this->setGeometry(100, 100, 360, 450);
  this->setFixedSize(360, 450);

  // init values
  crossValueX = 0;
  crossValueY = 0;
  newSpeedLeft  = 0;
  newSpeedRight = 0;
  actualSpeedLeft  = 0;
  actualSpeedRight = 0;
  zeroZoneSlider = 4;
  setMaxSpeed(12.0);

  // setup for the CrossWidget
  ui.crossW->setZeroZoneX(2);
  ui.crossW->setZeroZoneY(2);
  ui.crossW->setZeroZone0(2);
}

JoystickWidget::~JoystickWidget() {

}


// PUBLIC SLOTS

void JoystickWidget::setRightSpeed(int speed) {
  if (speed < zeroZoneSlider
      && speed > -zeroZoneSlider) {
    speed = 0;
    ui.vSliderRight->setValue(0);
  }
  actualSpeedRight = ((double)speed/100.0) * maxSpeed;
  emit newSpeed(actualSpeedLeft, actualSpeedRight);
  updateWidget();
}

void JoystickWidget::setLeftSpeed(int speed) {
  if (speed < zeroZoneSlider
      && speed > -zeroZoneSlider) {
    speed = 0;
    ui.vSliderLeft->setValue(0);
  }
  actualSpeedLeft = ((double)speed/100.0) * maxSpeed;
  emit newSpeed(actualSpeedLeft, actualSpeedRight);
  updateWidget();
}

void JoystickWidget::setMaxSpeed(double speed) {
  maxSpeed = speed;
  ui.doubleSpinBoxMaxSpeed->setValue(speed);
}

void JoystickWidget::updateCrossValue(double valueX, double valueY, bool leftMouseButton) {
  crossValueX = valueX;
  crossValueY = valueY;
  calcSpeed();
  if (leftMouseButton) {
    setSpeed();
  }
  updateWidget();
}


// PRIVATE

void JoystickWidget::updateWidget(void) {
  QString tempQString;

  // write speed of mouse position
  tempQString = "("
                 + QString().setNum(newSpeedLeft, 'f', 2)
                 + " / "
                 + QString().setNum(newSpeedRight, 'f', 2)
                 + ")";
  ui.labelNewValue->setText(tempQString);

  // write actual set speed
  tempQString = "("
                 + QString().setNum(actualSpeedLeft, 'f', 2)
                 + " / "
                 + QString().setNum(actualSpeedRight, 'f', 2)
                 + ")";
  ui.labelSetValue->setText(tempQString);
}


void JoystickWidget::calcSpeed(void) {
  double x, y;
  y = crossValueY * maxSpeed;
  x = crossValueX * maxSpeed;

  if (y-x > maxSpeed) {
    newSpeedRight = maxSpeed;
  } else if (y-x < -maxSpeed) {
    newSpeedRight = -maxSpeed;
  } else {
    newSpeedRight = y-x;
  }

  if (y+x > maxSpeed) {
    newSpeedLeft = maxSpeed;
  } else if (y+x < -maxSpeed) {
    newSpeedLeft = -maxSpeed;
  } else  {
    newSpeedLeft = y+x;
  }
}

void JoystickWidget::setSpeed(void) {
  actualSpeedLeft  = newSpeedLeft;
  actualSpeedRight = newSpeedRight;
  emit newSpeed(actualSpeedLeft, actualSpeedRight);
  ui.vSliderRight->setValue((int)((actualSpeedRight/maxSpeed) * 100));
  ui.vSliderLeft->setValue((int)((actualSpeedLeft/maxSpeed) * 100));
}


void JoystickWidget::keyPressEvent(QKeyEvent* e){
	printf("pressed %i\n",e->key());
	if (e->key() == 87){
		printf("w\n");
		newSpeedLeft = 0.1 * maxSpeed;newSpeedRight = 0.1 * maxSpeed;
	}
	if (e->key() == 83){
		printf("s\n");
		newSpeedLeft = -0.1 * maxSpeed;newSpeedRight = -0.1 * maxSpeed;
	}
	if (e->key() == 65){
		printf("a\n");
		newSpeedLeft = -0.1 * maxSpeed;newSpeedRight = 0.1 * maxSpeed;
	}
	if (e->key() == 68){
		printf("d\n");
		newSpeedLeft = 0.1 * maxSpeed;newSpeedRight = -0.1 * maxSpeed;
	}
	if (e->key() == 69){
		printf("e\n");
		newSpeedLeft = 0; newSpeedRight = 0;
	}
	setSpeed();
};

void JoystickWidget::keyReleaseEvent(QKeyEvent* e){
		//newSpeedLeft = 0;
		//newSpeedRight =	0;
};

void JoystickWidget::hideEvent(QHideEvent* event) {
  (void)event;
  emit hideSignal();
}

void JoystickWidget::closeEvent(QCloseEvent* event) {
  (void)event;
  emit closeSignal();
}
