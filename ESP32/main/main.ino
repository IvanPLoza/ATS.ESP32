/****************************************************************************
 *  @project:     ATS Project - ESP32 code
 *  @file_name:   main.ino
 *  @brief:
 *  @note:
 *           
 *  @author:      Ivan Pavao Lozancic ivanplozancic@gmail.com
 *  @date:        11-12-2018
 ****************************************************************************/
#include <WiFi.h>
#include <Int64String.h>

//#define WEBSOCKET
#ifdef WEBSOCKET
#include <WebSocketClient.h>  //Use WebSocket if defined
#else
#include <SocketIOClient.h>   //Use Socket.IO if WebSocket is not defined
#endif //WEBSOCKET

/****************************************************************************
 *                            Public definitions
 ***************************************************************************/

//Board and code configuration
#define BAUD_RATE 115200
#define TEST_MODE
#define WIFI_ENABLE
#define SERVER_CONNECT
//#define DEBUG

//WiFi Configuration
#ifdef WIFI_ENABLE

//#define HOME
#define DUMP
//#define BLUE_CAFFE
//#define RETRO
//#define MOBITEL
//#define DOMACIN

#ifdef HOME
#define SSID      "Jonelo2"
#define PASSWORD  "172030ZN"
#endif //HOME

#ifdef BLUE_CAFFE
#define SSID      "BlueEyes2"
#define PASSWORD  "blueeyes2"
#endif //BLUE_EYES

#ifdef DUMP
#define SSID      "dump"
#define PASSWORD  "Dump.12345"
#endif  //DUMP

#ifdef RETRO
#define SSID      "Retro_Caffe"
#define PASSWORD  "Lozinka12345"
#endif //RETRO

#ifdef MOBITEL
#define SSID      "loza"
#define PASSWORD  "la123456"
#endif

#ifdef DOMACIN
#define SSID      "Domacin"
#define PASSWORD  "domacin123"
#endif //DOMACIN

#endif //WIFI_ENABLE

//Server configuration
#ifdef SERVER_CONNECT
#define HOST  "192.168.88.198"     //Replace with server IP
#define PATH  "/" 
#define PORT  1337
#define TEST_DATA "DrazenDebil"
#endif //SERVER_CONNECT

#ifdef WEBSOCKET
WebSocketClient webSocketClient;
WiFiClient client;
#else
SocketIOClient client;
#endif //WEBSOCKET

//COMMANDS
uint8_t COMMAND_VEHICLEPASS = 0x01;
uint8_t COMMAND_CO2_UPDATE  = 0x02;
uint8_t COMMAND_ERROR       = 0x03;
uint8_t COMMAND_UNIT_INIT   = 0x04;

//TEST_DATA
uint64_t TESTDATA_COMMAND_VEHICLEPASS =  0x20001;         //command + 100000000000000001 = 1100000000000000001 = 393217
uint64_t TESTDATA_COMMAND_CO2UPDATE =    0x100401;        //command + 1000000000000000000000000001 = 10100000000010000000001 = 5243905
uint64_t TESTDATA_COMMAND_ERROR =        0x1005;          //command + 10000000000000000001 = 111000000000101 = 28677
uint64_t TESTDATA_COMMAND_UNITINIT =     0x4000000000003; //command + 100000000000000000000000000000000000000000000000001 
                                                          //   = 100100000000000000000000000000000000000000000000001 = 140737488355331 

//TEMP DATA 
#define SECTOR 0x01
#define ID     0x02

/****************************************************************************
 *                            Public functions
 ***************************************************************************/

/****************************************************************************
 *  @name:        WiFiConnect
 *  *************************************************************************
 *  @brief:       Connect to WiFi using ESP32
 *  @note:
 *  *************************************************************************
 *  @param[in]:   char ssid []
 *                char password []
 *  @param[out]:   
 *  @return:      [true]  = wifi connection is established
 *                [false] = wifi connection failed
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-12-2018
 ***************************************************************************/
#ifdef WIFI_ENABLE
bool WiFiConnect(char ssid[], char password[]){

  uint8_t connectTimeOverflow = 0;

  WiFi.begin(ssid, password); //Connect to wifi

  Serial.print("Connecting to: "); Serial.println(ssid);

  //Check if device is connected
  while (WiFi.status() != WL_CONNECTED  && connectTimeOverflow < 50){

    delay(500);

    #ifdef TEST_MODE
    Serial.print ( "." );
    #endif

    connectTimeOverflow++;
  }
  
  if(WiFi.status() != WL_CONNECTED){

    #ifdef TEST_MODE
    Serial.print("Falied to connect to: "); Serial.println(ssid);
    #endif //TEST_MODE

    return false;
  }

  #ifdef TEST_MODE
  Serial.print("Succesfully connected to: "); Serial.println(ssid);
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  #endif //TEST_MODE

  return true;

}
#endif //WIFI_ENABLE

/****************************************************************************
 *  @name:        clientConnect
 *  *************************************************************************
 *  @brief:       Check if client connection to server is established
 *  @note:
 *  *************************************************************************
 *  @param[in]:   char host []
 *                char port []
 *  @param[out]:   
 *  @return:      [true]  = client connection is established
 *                [false] = client connection failed
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-12-2018
 ***************************************************************************/
bool clientConnect(char host[], uint16_t port){

  if(client.connect(host, port)){

    #ifdef TEST_MODE
    Serial.print("Connected to "); Serial.print(host); Serial.print(" to port:"); Serial.println(port);
    #endif //TEST_MODE

    return true;
  } else {

    #ifdef TEST_MODE
    Serial.println("Connection failed.");
    #endif //TEST_MODE

    return false;
  }
}

/****************************************************************************
 *  @name:        clientHandshake
 *  *************************************************************************
 *  @brief:       WebSocket client handshake (connect) function 
 *  @note:
 *  *************************************************************************
 *  @param[in]:   char path []
 *                char host []
 *  @param[out]:   
 *  @return:      [true]  = client connection is established
 *                [false] = client connection failed
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-12-2018
 ***************************************************************************/
#ifdef WEBSOCKET
bool clientHandshake(char path[], char host[]){

  webSocketClient.path = path;
  webSocketClient.host = host;

  if(webSocketClient.handshake(client)) {

    #ifdef TEST_MODE
    Serial.println("Handshake with WebSocket server was successful!");
    #endif //TEST_MODE

    return true;
  }

  #ifdef TEST_MODE
  Serial.println("Handshake with WebSocket server failed.");
  #endif //TEST_MODE
    
  return false;
}
#endif //WEBSOCKET

/****************************************************************************
 *  @name:        testCommunication
 *  *************************************************************************
 *  @brief:       Sends and receives data if WebSocket communication is 
 *                working.
 *  @note:        IF THERE IS NO DATA FROM SERVER, COMMUNICATION IS
 *                CONSIDERED AS FAILED!
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      [true]  = communication is working
 *                [false] = communication is not working
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-12-2018
 ***************************************************************************/
#ifdef WEBSOCKET
bool testCommunication_WEBSOCKET(){

  String dataReceived; //Received data

  webSocketclient.sendData(TEST_DATA);
  webSocketClient.getData(dataReceived);

  if(dataReceived.length() > 0){

    #ifdef TEST_MODE
    Serial.println("Communication is working.");
    Serial.print("Received data: ");
    Serial.println(dataReceived);
    #endif //TEST_MODE

    return true;
  }

  #ifdef TEST_MODE
  Serial.println("Communication falied!");
  #endif //TEST_MODE

  return false;
}
#endif //WEBSOCKET

/****************************************************************************
 *  @name:        sendTestData_SOCKETIO
 *  *************************************************************************
 *  @brief:       Sends test data to server
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:      [true]  - command is correct
 *                [false] - command is incorrect
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-16-2018
 ***************************************************************************/
bool sendTestData_SOCKETIO(uint8_t command){

  String data;
  uint64_t dataBuffer;

  if(command == COMMAND_VEHICLEPASS){

    dataBuffer = ((uint64_t)COMMAND_VEHICLEPASS) << 18 | TESTDATA_COMMAND_VEHICLEPASS;

    data = String(int64String(dataBuffer));

    client.sendJSON("update", data);

  } else if(command == COMMAND_CO2_UPDATE){
    
    dataBuffer = (uint64_t)COMMAND_CO2_UPDATE << 28 | TESTDATA_COMMAND_CO2UPDATE;

    data = String(int64String(dataBuffer));


    client.sendJSON("update", data);

  } else if(command == COMMAND_ERROR){

    dataBuffer = (uint64_t)COMMAND_ERROR << 20 | TESTDATA_COMMAND_ERROR;

    data = String(int64String(dataBuffer));


    client.sendJSON("update", data);
  
  } else if(command == COMMAND_UNIT_INIT){

    dataBuffer = (uint64_t)COMMAND_UNIT_INIT << 48 | TESTDATA_COMMAND_UNITINIT;

    data = String(int64String(dataBuffer));

    client.sendJSON("update", data);

  } else {

    #ifdef TEST_MODE
    Serial.print("Command "); Serial.print(command); Serial.println(" is incorrect! Couldn't send data to server!");
    #endif //TEST_MODE

    return false;
  }

  #ifdef TEST_MODE
  Serial.print("Command "); Serial.print(command); Serial.println(" was sent to server.");
  Serial.print("Buffer: "); Serial.println(int64String(dataBuffer)); Serial.println();
  #endif //TEST_MODE

  return true;
}

/****************************************************************************
 *  @name:        updateLights
 *  *************************************************************************
 *  @brief:       Updates current traffic light state for defined secotr.
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   
 *  @param[out]:   
 *  @return:     
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-16-2018
 ***************************************************************************/
void updateLights(){
}

/****************************************************************************
 *  @name:        vehicleStateUpdate
 *  *************************************************************************
 *  @brief:       Sends vehiclePass comand to server
 *  @note:        
 *  *************************************************************************
 *  @param[in]:   string data - data buffer
 *  @param[out]:   
 *  @return:      
 *  *************************************************************************
 *  @author:      Ivan Pavao Lozancic
 *  @date:        11-16-2018
 ***************************************************************************/
void vehicleStateUpdate(){

  String data = String(COMMAND_VEHICLEPASS) + SECTOR + ID;

  client.sendJSON("update", data);

  delay(100);
}

/****************************************************************************
 *                            Setup function
 ***************************************************************************/
void setup() {

  Serial.begin(BAUD_RATE);
 
  WiFiConnect(SSID, PASSWORD);

  clientConnect(HOST, PORT);

  delay(1000);

  sendTestData_SOCKETIO(COMMAND_VEHICLEPASS);
  sendTestData_SOCKETIO(COMMAND_CO2_UPDATE);
  sendTestData_SOCKETIO(COMMAND_ERROR);
  sendTestData_SOCKETIO(COMMAND_UNIT_INIT);
 
}

/****************************************************************************
 *                            Main function
 ***************************************************************************/
void loop() {
 
}