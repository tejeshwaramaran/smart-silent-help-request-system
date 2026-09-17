#include <WiFi.h>
#include <FirebaseESP32.h>

// ======================
// WiFi Credentials
// ======================
#define WIFI_SSID "NAME"
#define WIFI_PASSWORD "PASSWORD"

// ======================
// Firebase Credentials
// ======================
#define FIREBASE_HOST "N/A"
#define FIREBASE_AUTH "N/A"

// ======================
// Pins
// ======================
#define BUTTON_PIN 16
#define PIR_PIN    18

// ======================
// Firebase Objects
// ======================
FirebaseData firebaseData;
FirebaseConfig config;
FirebaseAuth auth;

void setup() {
  Serial.begin(115200);

  // Button uses internal pull-up
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  // PIR is normal digital input
  pinMode(PIR_PIN, INPUT);

  // ======================
  // Connect to WiFi
  // ======================
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  Serial.print("Connecting to WiFi");

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // ======================
  // Initialize Firebase
  // ======================
  config.host = FIREBASE_HOST;
  config.signer.tokens.legacy_token = FIREBASE_AUTH;

  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);

  Serial.println("Firebase Connected");

  // Create Device B enabled value if it does not exist
  if (!Firebase.getInt(firebaseData, "/deviceB/enabled")) {
    Firebase.setInt(firebaseData, "/deviceB/enabled", 1);
  }
}

void loop() {

  // INPUT_PULLUP:
  // Not pressed = HIGH
  // Pressed = LOW
  int buttonPressed = (digitalRead(BUTTON_PIN) == LOW);

  // PIR:
  // No motion = 0
  // Motion detected = 1
  int motionState = digitalRead(PIR_PIN);

  // ======================
  // Send Device A data
  // ======================
  if (!Firebase.setInt(firebaseData, "/deviceA/button", buttonPressed)) {
    Serial.print("Button Firebase Error: ");
    Serial.println(firebaseData.errorReason());
  }

  if (!Firebase.setInt(firebaseData, "/deviceA/motion", motionState)) {
    Serial.print("Motion Firebase Error: ");
    Serial.println(firebaseData.errorReason());
  }

  // ======================
  // Serial Monitor
  // ======================
  Serial.print("Button: ");
  Serial.print(buttonPressed);

  Serial.print(" | Motion: ");
  Serial.println(motionState);

  delay(500);

