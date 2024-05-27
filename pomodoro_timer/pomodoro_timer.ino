#define BLYNK_TEMPLATE_ID "replace it with templated id"
#define BLYNK_TEMPLATE_NAME "name of the project	"
#define BLYNK_AUTH_TOKEN "your blynk token	"

#include <BlynkSimpleEsp32.h>
// I add every thing you need in comments.
// read the code clearly
char auth[] = BLYNK_AUTH_TOKEN;
char ssid[] = "your wifi ssid";
char pass[] = "password of wifi";

#define RGB_RED 21
#define RGB_GREEN 5
#define RGB_BLUE 18
#define BUZZER 26

BlynkTimer timer;

int studyTime = 50; // default study time in minutes
int shortBreak = 15; // default short break in minutes
int longBreak = 40; // default long break in minutes
int sessions = 8; // default number of sessions
bool isRunning = false;
bool sessionState = false; // false for stopped, true for running
int currentSession = 0;
unsigned long remainingTime = studyTime * 60 * 1000; // remaining time in milliseconds
unsigned long previousMillis = 0;
bool isLedOn = false;

BLYNK_WRITE(V0) { // Start/Restart button
  if (param.asInt() == 1) {
    startPomodoro();
  }
}

BLYNK_WRITE(V1) { // Stop/Resume button
  if (param.asInt() == 1) {
    if (isRunning) {
      stopPomodoro();
    } else {
      resumePomodoro();
    }
  }
}

BLYNK_WRITE(V2) { // Set study time
  studyTime = param.asInt();
  remainingTime = studyTime * 60 * 1000;
}

BLYNK_WRITE(V3) { // Set number of sessions
  sessions = param.asInt();
}

void setup() {
  pinMode(RGB_RED, OUTPUT);
  pinMode(RGB_GREEN, OUTPUT);
  pinMode(RGB_BLUE, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);

  timer.setInterval(1000L, updateTimer); // Update timer every second
  timer.setInterval(500L, blinkLed); // Blink LED every 500ms
}

void blinkLed() {
  if (isRunning && sessionState) {
    isLedOn = !isLedOn;
    digitalWrite(RGB_BLUE, isLedOn ? HIGH : LOW); // Blink Blue LED during study session
  } else {
    digitalWrite(RGB_BLUE, LOW); // Ensure LED is off when not running
  }
}

void updateTimer() {
  if (isRunning && sessionState) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= 1000) {
      previousMillis = currentMillis;
      remainingTime -= 1000;
      int minutes = (remainingTime / 1000) / 60;
      int seconds = (remainingTime / 1000) % 60;
      String timeString = String(minutes) + ":" + (seconds < 10 ? "0" : "") + String(seconds);
      Blynk.virtualWrite(V2, timeString); // Update session time
      if (!sessionState && (currentSession % 4 == 0 || currentSession == sessions)) {
        Blynk.virtualWrite(V4, timeString); // Update break time
      }
      if (remainingTime <= 0) {
        endSession();
      }
    }
  }
}

void startPomodoro() {
  isRunning = true;
  sessionState = true;
  currentSession = 0;
  remainingTime = studyTime * 60 * 1000;
  Blynk.virtualWrite(V0, 0); // Reset Blynk Start button
  Blynk.virtualWrite(V1, 0); // Reset Blynk Stop/Resume button
  startSession();
}

void stopPomodoro() {
  isRunning = false;
  sessionState = false;
  Blynk.virtualWrite(V1, 0); // Reset Blynk Stop/Resume button
  resetLedsAndBuzzer();
  digitalWrite(RGB_RED, HIGH); // Indicate stopped state with Red LED
}

void resumePomodoro() {
  isRunning = true;
  sessionState = true;
  previousMillis = millis();
}

void startSession() {
  if (currentSession < sessions) {
    Blynk.virtualWrite(V2, String(studyTime) + ":00"); // Display study time
    currentSession++;
    Blynk.virtualWrite(V3, currentSession); // Display current session
    digitalWrite(RGB_BLUE, HIGH); // Study session with Blue LED
    digitalWrite(RGB_RED, LOW);
    digitalWrite(RGB_GREEN, LOW);
    playSound(1); // Sound for starting session
    previousMillis = millis();
  } else {
    startLongBreak();
  }
}

void endSession() {
  if (sessionState) {
    digitalWrite(RGB_BLUE, LOW);
    if (currentSession % 4 == 0) {
      startLongBreak();
    } else {
      startShortBreak();
    }
  }
}

void startShortBreak() {
  Blynk.virtualWrite(V2, String(shortBreak) + ":00"); // Display short break time
  Blynk.virtualWrite(V4, String(shortBreak) + ":00"); // Display remaining break time
  // RGB LED cycling colors for short break
  for (int i = 0; i < shortBreak * 6; i++) {
    digitalWrite(RGB_RED, HIGH);
    digitalWrite(RGB_GREEN, LOW);
    digitalWrite(RGB_BLUE, LOW);
    delay(500);
    digitalWrite(RGB_RED, LOW);
    digitalWrite(RGB_GREEN, HIGH);
    digitalWrite(RGB_BLUE, LOW);
    delay(500);
    digitalWrite(RGB_RED, LOW);
    digitalWrite(RGB_GREEN, LOW);
    digitalWrite(RGB_BLUE, HIGH);
    delay(500);
  }
  playSound(2); // Sound for short break
  previousMillis = millis();
  remainingTime = shortBreak * 60 * 1000;
}

void startLongBreak() {
  Blynk.virtualWrite(V2, String(longBreak) + ":00"); // Display long break time
  Blynk.virtualWrite(V4, String(longBreak) + ":00"); // Display remaining break time
  digitalWrite(RGB_GREEN, HIGH); // Long break with Green LED
  digitalWrite(RGB_RED, LOW);
  digitalWrite(RGB_BLUE, LOW);
  playSound(3); // Sound for long break
  previousMillis = millis();
  remainingTime = longBreak * 60 * 1000;
}

void resetPomodoro() {
  stopPomodoro();
  Blynk

