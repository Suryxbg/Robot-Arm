#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>
#include <ESPAsyncWebServer.h>

// Create an instance of the Adafruit_PWMServoDriver
Adafruit_PWMServoDriver pwm;

// WiFi setup
const char *ssid = "TEJAS";
const char *password = "12345678t";

// Create an instance of the AsyncWebServer
AsyncWebServer server(80);

// Servo positions
int basePos = 90;
int armPos = 0;
int shoulderPos = 0;
int gripperPos = 90;

int base, arm, shoulder, gripper;

void setup() {
  Serial.begin(115200);

  // Initialize PCA9685 with the default address (0x40)
  pwm.begin();
  pwm.setPWMFreq(60); // Analog servos run at ~60 Hz updates

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Serve HTML page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    String html = "<html><body>";
    html += "<h1>Servo Control</h1>";
    html += "<label>Base: </label><input type='range' min='0' max='150' value='" + String(basePos) + "' id='baseSlider' onchange='updateServo(\"base\", this.value)'><br>";
    html += "<label>Arm: </label><input type='range' min='0' max='150' value='" + String(armPos) + "' id='armSlider' onchange='updateServo(\"arm\", this.value)'><br>";
    html += "<label>Shoulder: </label><input type='range' min='0' max='150' value='" + String(shoulderPos) + "' id='shoulderSlider' onchange='updateServo(\"shoulder\", this.value)'><br>";
    html += "<label>Gripper: </label><input type='range' min='0' max='90' value='" + String(gripperPos) + "' id='gripperSlider' onchange='updateServo(\"gripper\", this.value)'><br>";
    html += "<script>function updateServo(servo, value) {var xhttp = new XMLHttpRequest(); xhttp.open('GET', '/update?servo=' + servo + '&value=' + value, true); xhttp.send();}</script>";
    html += "</body></html>";
    request->send(200, "text/html", html);
  });

  // Handle servo updates
  server.on("/update", HTTP_GET, [](AsyncWebServerRequest *request){
    String servo = request->getParam("servo")->value();
    int value = request->getParam("value")->value().toInt();

    if (servo == "base") base = value;
    else if (servo == "arm") arm = value;
    else if (servo == "shoulder") shoulder = value;
    else if (servo == "gripper") gripper = value;

    updateServos();
    request->send(200, "text/plain", "OK");
  });

  // Start server
  server.begin();
}

void loop() {
  // Your code here
}

void updateServos() {
  smoothMoveServo(0, basePos, base);
  smoothMoveServo(1, armPos, arm);
  smoothMoveServo(2, shoulderPos, shoulder);
  smoothMoveServo(3, gripperPos, gripper);
}

void smoothMoveServo(int servoNumber, int &currentPos, int targetPos) {
  int step = (targetPos > currentPos) ? 1 : -1;

  while (currentPos != targetPos) {
    currentPos += step;
    pwm.setPWM(servoNumber, 0, map(currentPos, 0, 180, 150, 617));
    delay(10);  // Adjust delay for smoother motion
  }
}
