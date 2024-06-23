#define PULSEWIDTH 20 // 20
#define EDGETIME 10 // 10
#define HIGH 1
#define LOW 0


#define RS(x) digitalWrite(5, x)
#define RW(x) digitalWrite(6, x)
#define MSN(x) (PORTB & 0xF0) | ((x >> 4) & 0x0F)
#define LSN(x) (PORTB & 0xF0) | (x & 0x0F)
#define ENLCD(x) digitalWrite(7, x)
#define ENPULSE(x) ENLCD(HIGH); delayMicroseconds(x); ENLCD(LOW)
#define SET2WRITE DDRB = DDRB | 0x0F; RW(LOW); 
#define SET2READ DDRB = DDRB & 0xF0; RW(HIGH);


void write_lcd();
void instWrite();
void dataWrite();
byte instRead();
char dataRead();
void initLCDinterface();
void print_lcd();

int GLOBAL_wrap_en = 0;

void setup() {
  Serial.begin(9600); // Baud rate of 9600
  Serial.println("LCD Driver test");
  delay(50); // allow the LCD device to 'settle'
  for (int i = 7; i >= 5; i--) {
    pinMode(i, OUTPUT);
  }
  initLCDinterface(); // initialize the LCD as a 4-bit interface
  instWrite(0x28);  // Set the LCD to work as a 2-line display
  instWrite(0x08);  // Set the display & cursor to OFF
  instWrite(0x01);  // Wipe the display.
  delay(5);
  instWrite(0x06);  // Set the LCD to write from left-to-right
  instWrite(0x0c);  // Turn the display & cursor back on.

  Serial.write("Wrap mode? Y/N\n");
  while(Serial.available() == 0) {
      // Do nothing! Wait for there to be something to parse.
  }
  char wrapMode = Serial.read();
  if ((wrapMode == 'y') || (wrapMode == 'Y')) {
    GLOBAL_wrap_en = 1;
  }

}

void loop() {
  char input[32]; // char buffers -- this is our LCD input!
  int i, index;   // buffer indices
  int inputLen;   // buffer length variables
  int cursorPos;  // Address counter (i.e., cursor position) tracker.

  for (i = 0; i < 32; i++) {  // Pre-populate the char. arrays with 0, as C++ uses null-terminated strings.
    input[i] = 0;
  }
  i = 0;

  // Poll whatever we're looking at for input. 
  // We're going to be moving strings from the Serial Monitor to the LCD.




  while(Serial.available() == 0) {
      // Do nothing! Wait for there to be something to parse.
  }




  
  while (Serial.available()) {          // remember: a lone boolean in a condition is either TRUE or FALSE
    input[index]=Serial.read();         // Load the first char. value from the Serial monitor into the command[] array.
    delay(2); // necessary delay        // A delay of two-miliseconds. Apparently, this is necessary.
    index++;                            // Increment index. Thus, this entire while() loop will fill out the command[] array with all characters from the input. All other values will be 0.
  }

  inputLen = index;                     // Keep track of how big 'index' got. Its value should be determined by the length of our input.
  index = 0;                            // Reset the old index value back to zero.




switch (GLOBAL_wrap_en) {
  case 0:
      instWrite(0x01);      // If we get a new input, WIPE the LCD!
      delay(5);
      for (i = 0; i < inputLen; i++) {      // Our 'i' counter will be 0, and it increase after every loop until it hits the inputLen.
        cursorPos = instRead();
        dataWrite(input[i]);                // To the LCD, write whatever character is at the current location of 'i'.
        delay(2);                           // I wonder if this is a necessary delay, or if I'm only paranoid

      if (cursorPos == 15) {
        instWrite(0xC0);                      // This should move the cursor to the new line. Hopefully.
        delayMicroseconds(40);
      }
    }
  break;

  case 1:
      for (i = 0; i < inputLen; i++) {      // Our 'i' counter will be 0, and it increase after every loop until it hits the inputLen.
      cursorPos = instRead();
      dataWrite(input[i]);                // To the LCD, write whatever character is at the current location of 'i'.
      delay(2);                           // I wonder if this is a necessary delay, or if I'm only paranoid

      switch (cursorPos) {
        case 15:
            instWrite(0xC0);                      // This should move the cursor to the new line. Hopefully.
            delayMicroseconds(40);
            break;
        case 79:
            instWrite(0x02);                      // Home the cursor; wrap around.
            delay(2);
            break;
        default:
            break;
      }
    }
  break;
}


}

void print_lcd() {

}

void initLCDinterface() {
  SET2WRITE;  // Establish writemode.
  RS(LOW);    // Select instruction register.

  PORTB = LSN(0x03);  // 8-bit function set #1
  ENPULSE(50);        // Pulse the ENABLE line.
  delay(5);           // Wait for 5 ms.

  PORTB = LSN(0x03);  // 8-bit function set #2.
  ENPULSE(50);        // Pulse the ENABLE line.
  delay(1);           // Wait for 1 ms.

  PORTB = LSN(0x03);  // 8-bit function set #3.
  ENPULSE(50);        // Pulse the ENABLE line.

  PORTB = LSN(0x02);  // 4-bit function set.
  ENPULSE(50);        // Pulse the ENABLE line.
  delay(1);           // Superstition.
}


// ESSENTIAL WRITE FUNCTIONS
void write_lcd(char x) {
  PORTB = MSN(x);  // high nibble first
  ENPULSE(50);        // Pulse the enable line.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
  PORTB = LSN(x);   // Low nibble second.
  ENPULSE(50);        // Pulse the enable line.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
}

void instWrite(char cmd) {
  SET2WRITE; // establish control lines to write
  RS(LOW);   // instruction register
  write_lcd(cmd);
}

void dataWrite(char data) {
  SET2WRITE; // establish control lines to write
  RS(HIGH);   // instruction register
  
  write_lcd(data);
}

// ESSENTIAL READ FUNCTIONS
byte instRead() {
  byte value;
  SET2READ;       // establish control lines to read
  RS(LOW);        // Select instruction register.
  ENLCD(HIGH);              // Enable -> HIGH.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
  value = PINB & 0x0F;      // Read the high nibble.
  value = value << 4;       // Scoot the high nibble over.
  ENLCD(LOW);               // Enable -> LOW.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
  ENLCD(HIGH);              // Enable -> HIGH.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
  value = value | (PINB & 0x0F);  // Use an OR bitmask to combine the high nibble and the low nibble.
  ENLCD(LOW);               // Enable -> LOW.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
  return(value);  // return the value
}

char dataRead() {
  char value;
  SET2READ;
  RS(HIGH);      // Select instruction register.
  ENLCD(HIGH);              // Enable -> HIGH.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
  value = PINB & 0x0F;      // Read the high nibble.
  value = value << 4;       // Scoot the high nibble over.
  ENLCD(LOW);               // Enable -> LOW.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
  ENLCD(HIGH);              // Enable -> HIGH.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
  value = value | (PINB & 0x0F);  // Use an OR bitmask to combine the high nibble and the low nibble.
  ENLCD(LOW);               // Enable -> LOW.
  delayMicroseconds(EDGETIME);        // Wait for the edge.
  return(value); // return DR
}
