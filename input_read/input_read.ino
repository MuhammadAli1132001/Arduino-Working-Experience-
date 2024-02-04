int temp = 25;
int Rh = 30;
int pressure = 20;

void setup() {
  Serial.begin(9600);

  Serial.println("1. Temperature");
  Serial.println("2. Humidity");
  Serial.println("3. Barometric Pressure");
}

void loop() {
  Serial.println("Which sensor would you like to read? ");

  while (Serial.available() == 0) {
  }

  String input = Serial.readStringUntil('\n');   
  int menuChoice = input.toInt();

  switch (menuChoice) {

    case 1:
      // temp sensor code goes here
      Serial.print("The temperature is: ");
      Serial.println(temp);
      break;

    case 2:
      // humidity sensor code goes here
      Serial.print("The humidity is: ");
      Serial.println(Rh);
      break;

    case 3:
      // pressure sensor code goes here
      Serial.print("The barometric pressure is: ");
      Serial.println(pressure);
      break;
  }

  delay(100);
}
