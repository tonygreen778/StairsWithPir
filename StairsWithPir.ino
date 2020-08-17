// Stairs 3.0 PIR sensor

// LIGHT SETTINGS

int nightBright = 5;           // Яркость ночного режима ожидания
int ledLevelHigh = 50;         // Максимальная яркость
int ledLevelLow = 0;            // Минимальная яркость
int ledLevelStep = 5;          // Шаг изменения яркости

unsigned int lightSensorValue = 0;         // Цифровое значение фоторезистора
unsigned int lightSensorThreshold = 50;    // Пороговое значение, ниже которого должен работать ночной режим
unsigned int lightSensorHysteresis = 25;   // Допуск на гистерезис

// TIME SETTINGS

int lightOnTime = 20000;         // Время, на которое включается подсветка после срабатывания датчиков в миллисекундах
int ledLevelDelay = 10;          // Задержка при изменении яркости

// STAIRS SETTINGS

const int stairsCnt = 15;        // Количество ступенек

// PIN SETTINGS

int OutPins[stairsCnt] = {13,12,11,10,9,8,7,6,5,4,3,2,46,44,45}; // Массив PWM ног

const int nightLedPin = 27;     // Нога индикатора ночного режима
const int sonicLedPin = 26;     // Нога индикатора срабатывания ультрасоников
const int lightSensorPin = A0;  // Входная нога АЦП фоторезистора

const int PirUpPin = 31;        // Выход верхнего датчика движения
const int PirDownPin = 29;      // Выход нижнего датчика движения

// COMMON SETTINGS

boolean nightMode = false;      // Флаг - ночной режим
unsigned int time_us=0;
unsigned int distance_sm=0;
unsigned int distance_sm_1=0;

boolean debugMode = true;


void setup()
{
  if (debugMode)
    Serial.begin(115200);

  pinMode(PirUpPin, INPUT);
  pinMode(PirDownPin, INPUT);

  pinMode(nightLedPin, OUTPUT);  

  for(int i = 0; i < stairsCnt; i++)	 // Режим работы PWM ног - выход;
  {
       pinMode(OutPins[i], OUTPUT);  
  }
}

void loop()
{
  if (debugMode)
  {
    Serial.println("Loop Start");  
    Serial.println("getNightMode()");
  }
  
  getNightMode();
  
  // Опрос верхнего датчика
  if (nightMode && checkPir(PirUpPin))
  {
    if (debugMode)
      Serial.println("lightUp ON");
    
    lightUp();
    ledOnTimer();

    if (debugMode)
      Serial.println("darkUp");
    
    darkUp();      
  }
  
  // Опрос нижнего датчика
  if (nightMode && checkPir(PirDownPin))
  {
    if (debugMode)
      Serial.println("lightDown ON");

    lightDown();
    ledOnTimer();

    if (debugMode)
      Serial.println("darkDown");
    
    darkDown();       
  }
}

boolean checkPir(int _pin)
{
    boolean ret = false;
  
    if (debugMode)
    {
      Serial.print("checkPir ");
      Serial.print(_pin);
      Serial.print(": ");
    }
    if (digitalRead(_pin) == HIGH)
    {
      ret = true;
      Serial.println("HIGH");
    }
    else
    {
      Serial.println("LOW");
    }

    return ret;  
}

void lightUp()
{
	for(int i = 0; i < stairsCnt; i++)  
	{
          LedOn(OutPins[i]);
	}
}

void lightDown()
{
	for(int i = stairsCnt - 1; i >= 0; i--)  	
	{
	  LedOn(OutPins[i]);
	}
}

void darkUp()
{
	for(int i = 0; i < stairsCnt; i++)  
	{
	  LedOff(OutPins[i]);
	}
}

void darkDown()
{
	for(int i = stairsCnt - 1; i >= 0; i--)  	
	{
	  LedOff(OutPins[i]);	
	}
}

void setNightModeLight()
{
	if (nightMode)
	{
		analogWrite(OutPins[0], nightBright);
		analogWrite(OutPins[stairsCnt - 1], nightBright);
		digitalWrite(nightLedPin,HIGH);
            
                if (debugMode)
                  Serial.println("Night mode ON");
	}
	else
	{
		analogWrite(OutPins[0], 0);
		analogWrite(OutPins[stairsCnt - 1], 0);
		digitalWrite(nightLedPin,LOW);	

                if (debugMode)                
                  Serial.println("Night mode OFF");
	}
}

// Определение режима ДЕНЬ / НОЧЬ
boolean getNightMode()
{
  lightSensorValue = 0;
  unsigned int a = 0, b = 0;
  
  for(int i = 0; i < 50; i++)
  {
    lightSensorValue = analogRead(lightSensorPin);      // считываем значение с фоторезистора
    if (lightSensorValue < lightSensorThreshold) a++;         // Увеличиваем а, если темнее референсного значения;
    if (lightSensorValue < (lightSensorThreshold + lightSensorHysteresis)) b++;  // Увеличиваем b, если темнее чем референсное значение минус допуск на гистерезис;
  }
 
  if (debugMode)  
  {
    Serial.print("lightSensorValue: ");
    Serial.println(lightSensorValue, DEC);
  }
  
  if ((nightMode == 0) && (a > 49)) // Если сейчас режим день и 50 раз значение датчика оказалось выше референсного,
  {                             // то переключаемся в ночной режим, включаем дежурную подсветку ступенек.
    nightMode = true;
    setNightModeLight();
  }

  if ((nightMode == 1) && (b == 0)) // Если сейчас режим ночь, и 50 раз значение датчика оказалось ниже референсного за
  {                             // вычетом запаса на гистерезис - включаем режим "день", гасим подсветку ступенек.
    nightMode = 0;
    setNightModeLight();	
  }
  
  return nightMode;
}

void LedOff(int _ledPin)
{
  for (int i = ledLevelHigh; i >= ledLevelLow; i = i - ledLevelStep)
  {
    if (_ledPin != OutPins[0] && _ledPin != OutPins[stairsCnt - 1])
    {
      analogWrite(_ledPin, i);
      delay(ledLevelDelay);
    }
    else
    {
      if (!nightMode || (nightMode && i > nightBright))
      {
        analogWrite(_ledPin, i);
        delay(ledLevelDelay);
      }
    }
  }   
}

void LedOn(int _ledPin)
{
  for (int i = ledLevelLow; i <= ledLevelHigh; i = i + ledLevelStep)
  {
    if (_ledPin != OutPins[0] && _ledPin != OutPins[stairsCnt - 1])
    {  
      analogWrite(_ledPin, i);
      delay(ledLevelDelay);
    }
    else
    {
      if (!nightMode || (nightMode && i > nightBright))
      {
        analogWrite(_ledPin, i);
        delay(ledLevelDelay);
      }      
    }
  } 
}

void ledOnTimer()
{
  if (debugMode)
    Serial.println("ledOnTimer"); 
  
  unsigned long previousMillis = millis();  // считываем время, прошедшее с момента запуска программы   
  unsigned long currentTime = millis();     
  
  unsigned long elapsed = (currentTime >= previousMillis) ? currentTime - previousMillis : 0xFFFFFFFF - previousMillis + currentTime + 1;

  while(elapsed < lightOnTime)
  {
    if (checkPir(PirUpPin) || checkPir(PirDownPin)) // При срабатывании верхнего или нижнего датчика
    {
      previousMillis = millis();
      elapsed = 0;                          // обнуляем счётчик времени, прошедшего после включения освещения
      
      if (debugMode)
        Serial.println("ledOnTimer RESET");
    }   

    currentTime = millis();
    elapsed = (currentTime >= previousMillis) ? currentTime - previousMillis : 0xFFFFFFFF - previousMillis + currentTime + 1;
  }
}
