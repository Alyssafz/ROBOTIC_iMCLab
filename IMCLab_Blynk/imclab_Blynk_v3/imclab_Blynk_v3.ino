
#define BLYNK_TEMPLATE_ID "TMPL6ovVsijth"
#define BLYNK_TEMPLATE_NAME "New Template"
#define BLYNK_AUTH_TOKEN "1yeEOj7V_9e0cwJiufx7s6pisnZc7D5W"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>

// Konfigurasi WiFi
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "TesRobotik";
char pass[] = "lulabies";

int ledPin = 2;

int motor1Pin1 = 27;
int motor1Pin2 = 26;
int enable1Pin = 12;

const byte pin_rpm = 13;
const int HOLES = 2;

volatile unsigned int rev = 0;
volatile unsigned long lastPulseTime = 0;

const unsigned long DEBOUNCE_US = 500;

const int freq = 5000;          // 30000 -> 5000 Hz 
const int pwmChannel = 0;
const int resolution = 8;

int dutyCycle = 200;

const int DUTY_STEP = 10;
const int DUTY_MIN = 0;
const int DUTY_MAX = 255;
const int DUTY_ON  = 205;       

bool motorEnable = false;

const int LEVEL_DUTY[4] = {0, 100, 150, 255};
const char* LEVEL_NAMA[4] = {"MANUAL", "PELAN", "SEDANG", "CEPAT"};

const int kickDuty = 255;       // duty penuh saat awal start
const int kickTime = 60;        
bool motorRunning = false;      

float rpm = 0.0;
unsigned long old_time = 0;

String inputString = "";

BlynkTimer timer;

void IRAM_ATTR isr()
{
  unsigned long now = micros();

  if (now - lastPulseTime > DEBOUNCE_US)
  {
    rev++;
    lastPulseTime = now;
  }
}

int levelDariDuty(int duty)
{
  for (int i = 1; i <= 3; i++)
  {
    if (duty == LEVEL_DUTY[i]) return i;
  }
  return 0;
}

BLYNK_CONNECTED()
{
  Blynk.syncVirtual(V2);
  Blynk.syncVirtual(V3);
  Blynk.syncVirtual(V4);   
}

void hitungRPM()
{
  unsigned long cur_time = millis();

  if (cur_time - old_time >= 1000)
  {
    detachInterrupt(digitalPinToInterrupt(pin_rpm));

    float rot_per_sec = (float)rev / (float)HOLES;

    rpm = rot_per_sec * 60.0;

    rev = 0;

    attachInterrupt(
      digitalPinToInterrupt(pin_rpm),
      isr,
      FALLING);

    old_time = millis();
  }
}

void applyMotor()
{
  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);

  if (!motorRunning && dutyCycle > 0 && dutyCycle < kickDuty)
  {
    ledcWrite(pwmChannel, kickDuty);
    delay(kickTime);
  }

  ledcWrite(pwmChannel, dutyCycle);
  motorRunning = (dutyCycle > 0);

  digitalWrite(ledPin, dutyCycle > 0);

  // Sinkronisasi ke Blynk
  Blynk.virtualWrite(V1, dutyCycle);
  Blynk.virtualWrite(V2, dutyCycle);
  Blynk.virtualWrite(V3, dutyCycle > 0);

  int lv = levelDariDuty(dutyCycle);
  if (lv > 0) Blynk.virtualWrite(V4, lv);

  if (dutyCycle == 0) Blynk.virtualWrite(V5, "OFF");
  else Blynk.virtualWrite(V5, LEVEL_NAMA[lv]);
}

void setLevel(int level)
{
  level = constrain(level, 1, 3);
  dutyCycle = LEVEL_DUTY[level];
  motorEnable = true;
  applyMotor();
}

void tampilkanStatus(const char* aksi)
{
  Serial.print(aksi);
  Serial.print(" | PWM=");
  Serial.print(dutyCycle);
  Serial.print(" | RPM=");
  Serial.println(rpm, 1);
}

// Kirim Data ke Blynk

void kirimKeBlynk()
{
  Blynk.virtualWrite(V0, rpm);
  Blynk.virtualWrite(V1, dutyCycle);
  Blynk.virtualWrite(V2, dutyCycle);
  Blynk.virtualWrite(V3, dutyCycle > 0);
}

// Slider PWM dari Blynk (V2)

BLYNK_WRITE(V2)
{
  dutyCycle = constrain(
    param.asInt(),
    DUTY_MIN,
    DUTY_MAX);

  motorEnable = (dutyCycle > 0);

  applyMotor();

  Serial.print("PWM dari Blynk = ");
  Serial.println(dutyCycle);
}

// Tombol ON/OFF dari Blynk (V3)

BLYNK_WRITE(V3)
{
  motorEnable = param.asInt();

  if (motorEnable)
  {
    if (dutyCycle == 0)
      dutyCycle = DUTY_ON;     // 200 -> 205 
  }
  else
  {
    dutyCycle = 0;
  }

  applyMotor();

  Serial.print("Motor ");
  Serial.println(motorEnable ? "ON" : "OFF");
}

// Segmented Switch level kecepatan dari Blynk (V4)

BLYNK_WRITE(V4)
{
  int level = param.asInt();

  // Nilai di luar 1-3 
  if (level < 1 || level > 3) return;

  setLevel(level);

  Serial.print("Level dari Blynk = ");
  Serial.print(LEVEL_NAMA[level]);
  Serial.print(" (duty ");
  Serial.print(dutyCycle);
  Serial.println(")");
}

// Setup

void setup()
{
  Serial.begin(115200);

  pinMode(ledPin, OUTPUT);

  pinMode(motor1Pin1, OUTPUT);
  pinMode(motor1Pin2, OUTPUT);
  pinMode(enable1Pin, OUTPUT);

  pinMode(pin_rpm, INPUT_PULLUP);

  attachInterrupt(
    digitalPinToInterrupt(pin_rpm),
    isr,
    FALLING);

  ledcSetup(
    pwmChannel,
    freq,
    resolution);

  ledcAttachPin(
    enable1Pin,
    pwmChannel);

  digitalWrite(motor1Pin1, HIGH);
  digitalWrite(motor1Pin2, LOW);

  ledcWrite(pwmChannel, 0);

  old_time = millis();

  // Koneksi ke Blynk
  Blynk.begin(auth, ssid, pass);

  timer.setInterval(
    1000L,
    kirimKeBlynk);

  Serial.println("================================");
  Serial.println("iMCLab + Blynk Ready (v3)");
  Serial.println("================================");
  Serial.println("1 = ON (duty 205)");
  Serial.println("0 = OFF");
  Serial.println("2 = Pelan  (duty 100)");
  Serial.println("3 = Sedang (duty 150)");
  Serial.println("4 = Cepat  (duty 255)");
  Serial.println("+ = PWM naik");
  Serial.println("- = PWM turun");
  Serial.println("PWM:xxx = set PWM manual");
  Serial.println("R = lihat RPM");
}

// Loop

void loop()
{
  Blynk.run();
  timer.run();

  hitungRPM();

  while (Serial.available() > 0)
  {
    char c = Serial.read();

    if (c == '\n' || c == '\r')
    {
      inputString.trim();

      if (inputString.startsWith("PWM:"))
      {
        int val = inputString.substring(4).toInt();

        val = constrain(
          val,
          DUTY_MIN,
          DUTY_MAX);

        dutyCycle = val;
        motorEnable = (dutyCycle > 0);

        applyMotor();

        tampilkanStatus("Set PWM");
      }

      else if (inputString.length() > 0)
      {
        char cmd = inputString.charAt(0);

        if (cmd == '1')
        {
          dutyCycle = DUTY_ON;
          motorEnable = true;

          applyMotor();

          tampilkanStatus("Motor ON");
        }

        else if (cmd == '0')
        {
          dutyCycle = 0;
          motorEnable = false;

          applyMotor();

          Serial.println("Motor OFF");
        }

        else if (cmd == '2')
        {
          setLevel(1);
          tampilkanStatus("Motor PELAN");
        }

        else if (cmd == '3')
        {
          setLevel(2);
          tampilkanStatus("Motor SEDANG");
        }

        else if (cmd == '4')
        {
          setLevel(3);
          tampilkanStatus("Motor CEPAT");
        }

        else if (cmd == '+')
        {
          dutyCycle += DUTY_STEP;

          if (dutyCycle > DUTY_MAX)
            dutyCycle = DUTY_MAX;

          motorEnable = true;

          applyMotor();

          tampilkanStatus("PWM NAIK");
        }

        else if (cmd == '-')
        {
          dutyCycle -= DUTY_STEP;

          if (dutyCycle < DUTY_MIN)
            dutyCycle = DUTY_MIN;

          motorEnable = (dutyCycle > 0);

          applyMotor();

          tampilkanStatus("PWM TURUN");
        }

        else if (cmd == 'R' || cmd == 'r')
        {
          Serial.print("RPM = ");
          Serial.print(rpm, 1);
          Serial.print(" | PWM = ");
          Serial.println(dutyCycle);
        }
      }

      inputString = "";
    }
    else
    {
      inputString += c;
    }
  }
}
