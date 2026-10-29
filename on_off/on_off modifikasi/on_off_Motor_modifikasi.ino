int ledPin = 2;
char myData = 0;

int motor1Pin1 = 27;
int motor1Pin2 = 26;
int enable1Pin = 12;

// Setting PWM properties
const int freq = 5000;  // DIUBAH v3: 30000 -> 5000 Hz 
const int pwmChannel = 0;
const int resolution = 8;
int dutyCycle = 200;

const int kickDuty = 255;       
const int kickTime = 60;       
bool motorRunning = false;      

const byte pin_rpm = 13;
int volatile rev = 0;

void setup()
{
  pinMode(ledPin, OUTPUT);
  Serial.begin(115200);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);
  pinMode(pin_rpm, INPUT_PULLUP);

  ledcSetup(pwmChannel, freq, resolution);
  ledcAttachPin(enable1Pin, pwmChannel);

  Serial.println("Testing DC Motor...");
  Serial.println("Ketik: 1=ON, 0=OFF, 2=pelan, 3=sedang, 4=cepat");
}

// Motor maju dengan kecepatan (duty cycle 0-255)
void MotorOn(int speed)
{
  dutyCycle = speed;
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);

  // hanya jika motor sebelumnya diam dan kecepatan target rendah
  if (!motorRunning && speed < kickDuty) {
    ledcWrite(pwmChannel, kickDuty);
    delay(kickTime);
  }

  ledcWrite(pwmChannel, dutyCycle);
  motorRunning = true;
}

void MotorOff()
{
  dutyCycle = 0;
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);
  ledcWrite(pwmChannel, dutyCycle);
  motorRunning = false;
}

void loop()
{
  if (Serial.available() > 0) {
    myData = Serial.read();

    if (myData == '1') {
      Serial.println("LED is on !!!");
      digitalWrite(ledPin, HIGH);
      MotorOn(205);
    }
    else if (myData == '0') {
      Serial.println("LED is off !!!");
      digitalWrite(ledPin, LOW);
      MotorOff();
    }
    else if (myData == '2') {
      Serial.println("Motor pelan (duty 100)");
      digitalWrite(ledPin, HIGH);
      MotorOn(100);
    }
    else if (myData == '3') {
      Serial.println("Motor sedang (duty 150)");
      digitalWrite(ledPin, HIGH);
      MotorOn(150);
    }
    else if (myData == '4') {
      Serial.println("Motor cepat (duty 255)");
      digitalWrite(ledPin, HIGH);
      MotorOn(255);
    }
  }
}
