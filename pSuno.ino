/*  pSuno power supply arduino code.
 *   
 *  Displays set voltage and current as well as active voltage and current.
 *  
 *  Physical design of the baord limits the minimum current limit to ~200mA and the measurable current to ~150mA.
 *  Using freetronics LCD shield, requires pin 9 to be rerouted to pin 11.
 */

// set up lcd display
#include <Wire.h>
#include <LiquidCrystal.h>
LiquidCrystal lcd( 8, 11, 4, 5, 6, 7 );

// define IO
#define BUTTONS_PIN 0
#define I_SENSE_PIN A2
#define V_SENSE_PIN A3
#define I_CTRL_PIN 9
#define V_CTRL_PIN 10

// define each button press
#define RIGHT 1
#define UP 2
#define DOWN 3
#define LEFT 4
#define SELECT 5

// function prototypes
void update_set_display(int current, int voltage);
void update_active_display(int current, int voltage);
int get_button(int pin);
long map_array(long value, long *mapping_array);

void setup() {
    lcd.begin(16, 2);

    pinMode(I_CTRL_PIN, OUTPUT);
    pinMode(V_CTRL_PIN, OUTPUT);

    // set 10 bit pwm to increase the resolution of the voltage and current
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

  // initialise local variables for averaging out the voltage and current readings
  const int NUM_READINGS = 30;
  int read_index = 0;
  
  int i_readings[NUM_READINGS];
  int i_read_total = 0;
  int i_read_average = 0;
  
  int v_readings[NUM_READINGS];
  int v_read_total = 0;
  int v_read_average = 0;
  
  int i_disp;
  int v_disp;

  // initial startup values for current and voltage
  int i_ctrl = 200;
  int v_ctrl = 700;

  // mapping values for the mV/mA->pwm and adc->mV/mA
  long i_disp_map[] = {130, 960, 130, 1000};
  long v_disp_map[] = {70, 930, 700, 9070};
  long i_ctrl_map[] = {172, 1000, 148, 880};
  long v_ctrl_map[] = {700, 9070,61, 940};

  // values for debouncing the button presses
  unsigned long last_debounce_time = 0;
  const int DEBOUNCE_DELAY = 50;
  int last_button_state = 0;
  int button_state;

  // counter for updating the display at a certain frequency
  const int DISP_DELAY = 15;
  unsigned long disp_time;
  unsigned long last_disp_time = 0;

  // clear the current and voltage reading arrays to zero
  for (int i = 0; i < NUM_READINGS; i++) {
    i_readings[i] = 0;
    v_readings[i] = 0;
    }

  for(;;){

    // subtract the last values from the total
    i_read_total -= i_readings[read_index];
    v_read_total -= v_readings[read_index];

    // take new readings and place them in the arrays
    i_readings[read_index] = analogRead(I_SENSE_PIN);
    v_readings[read_index] = analogRead(V_SENSE_PIN);

    // add the new readings to the totals
    i_read_total += i_readings[read_index];
    v_read_total += v_readings[read_index];

    // increment and loop over the length of the arrays
    read_index++;
    if (read_index >= NUM_READINGS)
      read_index = 0;

    // take the average of the current totals
    i_read_average = i_read_total / NUM_READINGS;
    v_read_average = v_read_total / NUM_READINGS;

    // get the correct values to display.
    i_disp = map_array(i_read_average, i_disp_map);
    v_disp = map_array(v_read_average, v_disp_map);


    if ((millis() - last_disp_time) > DISP_DELAY){
      lcd.clear();
      update_set_display(i_ctrl, v_ctrl);
      
      if (i_disp <= 150){
        update_active_display(0,v_disp);
      }
      else{
        update_active_display(i_disp,v_disp);
      }
      last_disp_time = millis();
    }

    int button_reading = get_button(BUTTONS_PIN);

    if (button_reading != last_button_state){
      last_debounce_time = millis();
    }

    if((millis() - last_debounce_time) > DEBOUNCE_DELAY){
      if (button_reading != button_state){
        button_state = button_reading;
        if(button_state == UP && v_ctrl < 10000)
          v_ctrl += 100;
        else if (button_state == DOWN && v_ctrl > 700)
          v_ctrl -= 100;
        else if (button_state == LEFT && i_ctrl> 200)
          i_ctrl -= 10;
        else if (button_state == RIGHT && i_ctrl < 1000)
          i_ctrl += 10;
      }
    }
    
    last_button_state = button_reading;
    
    analogWrite(I_CTRL_PIN, map_array(i_ctrl, i_ctrl_map));
    analogWrite(V_CTRL_PIN, map_array(v_ctrl, v_ctrl_map));
  
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

long map_array(long value, long *mapping_array){
  return (value - mapping_array[0]) * (mapping_array[3] - mapping_array[2]) / (mapping_array[1]- mapping_array[0]) + mapping_array[2];
}

