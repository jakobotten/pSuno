/*  pSuno power supply arduino code.
 *   
 *  Displays set voltage and current as well as active voltage and current.
 *  
 *  Physical design of the baord limits the minimum current limit to ~200mA and the measurable current to ~150mA.
 *  
 * 
 * 
 * 
 */

#include <Wire.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd( 8, 11, 4, 5, 6, 7 );

#define BUTTONS_PIN 0
#define CURRENT_SENSE_PIN A2
#define VOLTAGE_SENSE_PIN A3
#define CURRENT_CONTROL_PIN 9
#define VOLTAGE_CONTROL_PIN 10

#define RIGHT 1
#define UP 2
#define DOWN 3
#define LEFT 4
#define SELECT 5

void update_set_display(int current, int voltage);

void update_active_display(int current, int voltage);

int get_button(int pin);

void setup() {
    lcd.begin(16, 2);

    pinMode(CURRENT_CONTROL_PIN, OUTPUT);
    pinMode(VOLTAGE_CONTROL_PIN, OUTPUT);

    // set pwm clock divider
    TCCR1B &= ~(1 << CS12);
    TCCR1B  |=   (1 << CS11);
    TCCR1B &= ~(1 << CS10);  
                           
    // set pwm resolution  to mode 7 (10 bit)
    TCCR1B &= ~(1 << WGM13);    // Timer B clear bit 4
    TCCR1B |=  (1 << WGM12);    // set bit 3
    
    TCCR1A |= (1 << WGM11);    //  Timer A set bit 1
    TCCR1A |= (1 << WGM10);    //  set bit 0
}

void loop() {
  
  const int NUM_READINGS = 30;
  int read_index = 0;
  
  int current_readings[NUM_READINGS];
  int current_read_total = 0;
  int current_read_average = 0;
  
  int voltage_readings[NUM_READINGS];
  int voltage_read_total = 0;
  int voltage_read_average = 0;
  
  int current;
  int voltage;

  int current_control = 180;
  int voltage_control = 9000;

  for (int i = 0; i < NUM_READINGS; i++) {
    current_readings[i] = 0;
    voltage_readings[i] = 0;
    }

  for(;;){
    
    current_read_total -= current_readings[read_index];
    voltage_read_total -= voltage_readings[read_index];
    
    current_readings[read_index] = analogRead(CURRENT_SENSE_PIN);
    voltage_readings[read_index] = analogRead(VOLTAGE_SENSE_PIN);
  
    current_read_total += current_readings[read_index];
    voltage_read_total += voltage_readings[read_index];
  
    read_index++;
  
    if (read_index >= NUM_READINGS)
      read_index = 0;
  
    current_read_average = current_read_total / NUM_READINGS;
    voltage_read_average = voltage_read_total / NUM_READINGS;
  
    current = map(current_read_average, 130, 960, 130, 1000);
    voltage = map(voltage_read_average, 70, 930,700, 9070);

    lcd.clear();
    update_set_display(current_control, voltage_control);
    
    if (current <= 150){
      update_active_display(0,voltage);
    }
    else{
      update_active_display(current,voltage);
    }

    if(get_button(BUTTONS_PIN) == UP && voltage_control < 10000){
      voltage_control += 100;
      delay(200);
    }
    else if (get_button(BUTTONS_PIN) == DOWN && voltage_control > 700){
      voltage_control -= 100;
      delay(200);
    }
    else if (get_button(BUTTONS_PIN) == LEFT && current_control > 180){
      current_control -= 10;
      delay(200);
    }
    else if (get_button(BUTTONS_PIN) == RIGHT && current_control < 1000){
      current_control += 10;
      delay(200);
    }
    
    analogWrite(CURRENT_CONTROL_PIN, map(current_control,172, 1000, 148, 880));
    analogWrite(VOLTAGE_CONTROL_PIN, map(voltage_control, 700, 9070,61, 940));
  
    delay(15);
  }
}

void update_set_display(int current, int voltage){
  lcd.setCursor(0, 0);
  voltage = (voltage/10)*10;
  if (voltage >= 1000){
      lcd.print("Vs:");
      lcd.print((float)voltage/1000);
      lcd.print("V ");
  }
  else{
      lcd.print("Vs:");
      lcd.print(voltage);
      lcd.print("mV ");
  }
  lcd.setCursor(0, 1);
  if (current >= 1000){
    lcd.print("Is:");
    lcd.print((float)current/1000);
    lcd.print("A ");
  }
  else {
    lcd.print("Is:");
    lcd.print(current);
    lcd.print("mA ");
  }
}
void update_active_display(int current, int voltage){
  lcd.setCursor(9, 0);
  voltage = (voltage/10)*10;
  if (voltage >= 1000){
      lcd.print("V:");
      lcd.print((float)voltage/1000);
      lcd.print("V ");
  }
  else{
      lcd.print("V:");
      lcd.print(voltage);
      lcd.print("mV ");
  }
  lcd.setCursor(9, 1);
  if (current >= 1000){
    lcd.print("I:");
    lcd.print((float)current/1000);
    lcd.print("A ");
  }
  else if (current != 0){
    lcd.print("I:");
    lcd.print(current);
    lcd.print("mA ");
  }
  else {
    lcd.print("I<150mA");
  }
}

int get_button(int pin){
  int button_read = analogRead(pin);
  int button_num;
  if (button_read < 70)
    button_num = 1; 
  else if (button_read < 230)
    button_num = 2;
  else if (button_read < 410)
    button_num = 3;
  else if (button_read < 570)
    button_num = 4;
  else if (button_read < 810)
    button_num = 5;
  else
    button_num = 0;
  return button_num;
}

