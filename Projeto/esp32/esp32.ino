#include <WiFi.h>
#include "driver/uart.h"

const char* ssid     = "Kfouri's cell";
const char* password = "kfouriskr";

uart_config_t uart_config = {
  .baud_rate = 9600,
  .data_bits = UART_DATA_8_BITS,
  .parity    = UART_PARITY_DISABLE,
  .stop_bits = UART_STOP_BITS_1,
  .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
};

void despejar();

WiFiServer server(80);

void setup() {
    Serial.begin(115200);
    delay(10);

    // Configurando o Wifi

//    Serial.println();
//    Serial.println();
//    Serial.print("Connecting to ");
//    Serial.println(ssid);
//
//    WiFi.begin(ssid, password);
//
//    while (WiFi.status() != WL_CONNECTED) {
//        delay(500);
//        Serial.print(".");
//    }
//
//    Serial.println("");
//    Serial.println("WiFi connected.");
//    Serial.println("IP address: ");
//    Serial.println(WiFi.localIP());
//    
//    server.begin();

    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, 18, 19);
    // Setup UART buffered IO with event queue
    const int uart_buffer_size = (1024 * 2);
    QueueHandle_t uart_queue;
    // Install UART driver using an event queue here
    uart_driver_install(UART_NUM_2, uart_buffer_size, uart_buffer_size, 10, &uart_queue, 0);
}

void loop() {
//    WiFiClient client = server.available();   // listen for incoming clients
//
//    if (client) {                             // if you get a client,
//        Serial.println("New Client.");           // print a message out the serial port
//        String currentLine = "";                // make a String to hold incoming data from the client
//        while (client.connected()) {            // loop while the client's connected
//            if (client.available()) {             // if there's bytes to read from the client,
//                char c = client.read();             // read a byte, then
//                Serial.write(c);                    // print it out the serial monitor
//            if (c == '\n') {                    // if the byte is a newline character
//
//                // if the current line is blank, you got two newline characters in a row.
//                // that's the end of the client HTTP request, so send a response:
//                if (currentLine.length() == 0) {
//                // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
//                // and a content-type so the client knows what's coming, then a blank line:
//                client.println("HTTP/1.1 200 OK");
//                client.println("Content-type:text/html");
//                client.println();
//
//                // the content of the HTTP response follows the header:
//                client.print("Click <a href=\"/D\">here</a> to give food to the pet");
//
//                // The HTTP response ends with another blank line:
//                client.println();
//                // break out of the while loop:
//                break;
//            }
//            else {    // if you got a newline, then clear currentLine:
//                currentLine = "";
//            }
//        } 
//        else if (c != '\r') {  // if you got anything else but a carriage return character,
//            currentLine += c;      // add it to the end of the currentLine
//        }
//
//        if (currentLine.endsWith("GET /D")) {
//          despejar();
//          client.print("<a href=\"/P\">");
//        }
//      }
//    }
//    // close the connection:
//    client.stop();
//    Serial.println("Client Disconnected.");
//   }
   uart_wait_tx_done(UART_NUM_2, 100);
   char* test_str = "01010101";
   uart_write_bytes(UART_NUM_2, (const char*)test_str, 8);
}
//
//void despejar(){
//    uart_wait_tx_done(UART_NUM_2, 100);
//    char* test_str = "01010101";
//    uart_write_bytes(UART_NUM_2, (const char*)test_str, 8);
//    return;
//}
