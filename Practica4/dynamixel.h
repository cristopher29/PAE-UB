#ifndef DYNAMIXEL_H_
#define DYNAMIXEL_H_

void wheelMode(void);
void moveWheel(byte ID, bool rotation, unsigned int speed);
void turnLeft(unsigned int speed);
void turnLeftD(unsigned int degree);
void turnOnItselfLeft(unsigned int speed);
void turnRight(unsigned int speed);
void turnRightD(unsigned int degree);
void turnOnItselfRight(unsigned int speed);
void motorLed(byte ID, bool status);
void stop(void);
void forward(unsigned int speed);
void backward(unsigned int speed);
void motorLed(byte ID, bool status);
int readSensor(byte ID, byte sensor);
void setCompareDistance(byte ID,unsigned int dist);
int getObstacleDetected(byte ID);
int readMaxDist(byte ID, byte position);

#endif /* DYNAMIXEL_H_ */
