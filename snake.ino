#include<FastLED.h>
#include<WiFi.h>
#include<PubSubClient.h>
#define PIN 22
#define MAXLED 484
CRGB leds[MAXLED];
const char* ssid = "Athletics";
const char* password = "12345678";
const char* mqtt_server = "47.102.201.222";
WiFiClient espClient;
PubSubClient client(espClient);
char towards = 'D';
char opt = 'I';
int maps[22][22];//0:nothing 1:wall 2:food 3:snake 
int snake_len = 3;
typedef struct Snakes{
  int x;
  int y;
  struct Snakes *next;
}snake;
snake *head;
struct Food{
  int x;
  int y;
}food;
void create_snake(){
  head = (snake*)malloc(sizeof(snake));
  head->x = 12;
  head->y = 10;
  snake *p = (snake*)malloc(sizeof(snake));
  snake *q = (snake*)malloc(sizeof(snake));
  p->x = 11;
  p->y = 10;
  q->x = 10;
  q->y = 10;
  head->next = p;
  p->next = q;
  q->next = NULL;
  maps[head->y][head->x]=3;
  maps[p->y][p->x]=3;
  maps[q->y][q->x]=3;
  leds[(head->y)*22+head->x]=CRGB(255,0,0);
  leds[(p->y)*22+p->x]=CRGB(255,0,0);
  leds[(q->y)*22+q->y]=CRGB(255,0,0);
}
void create_food(){
  int i,j;
  do{
    i = random(22);
    j = random(22); 
  }while(maps[i][j]!=0);
  food.y = i;
  food.x = j;
  maps[i][j]=2;
  if(i%2==1){
    int pos;
    pos=(i+1)*22-1-j;
    leds[pos]=CRGB(0,255,0);
  }else{
    leds[i*22+j]=CRGB(0,255,0);
  }
}

void setup_wifi(){
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid,password);
  while(WiFi.status()!=WL_CONNECTED){
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address:");
  Serial.println(WiFi.localIP());
}
void callback(char* topic,byte* payload,unsigned int length){   
  opt = (char)payload[0];
  Serial.println(opt);
  Serial.println((char)payload[0]);
//  Serial.print("传入数据:");
//  Serial.println((char)payload[0]);
//  if(strcmp(topic,"mode")==0){
//    opt = (char)payload[0];   
//    Serial.println(opt);        
//  }else if(strcmp(topic,"control")==0){
//    towards = (char)payload[0];
//    Serial.println(towards);
//  }
}
void clear_all(){
  for(int i = 0;i<484;i++){
    leds[i] = CRGB(0,0,0);
  }
}
void create_walls(){
  int i; 
  for(i = 0;i < 22;i++){
    maps[0][i] = 1;
    maps[21][i] = 1;
    maps[i][0] = 1;
    maps[i][21] = 1;
  }
  for(i = 0;i < MAXLED;i++){
    if(maps[i/22][i%22] == 1){
      leds[i] = CRGB(0,0,255);
    }else{
      leds[i] = CRGB(0,0,0);
    }
  }
//  FastLED.show();
}
void setup() {
  Serial.begin(115200);
  FastLED.setBrightness(64); 
  for(int i = 0;i < 22;i++){
    for(int j = 0;j < 22;j++){
      maps[i][j] = 0;
    }
  }
  setup_wifi();
  client.setServer(mqtt_server,1883);
  client.setCallback(callback);
  FastLED.addLeds<WS2812,PIN,GRB>(leds,MAXLED);
}
void reconnect(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    if(client.connect("ESP32Client")){
      Serial.println("connected");
      client.subscribe("mode");
      client.subscribe("control");
      Serial.println("订阅上了");
    }else{
      Serial.print("failed,rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}
void snake_moving(){
  int snake_len = 1;
  int x = head->x, y = head->y;
  snake *p = head->next;
  //先默认关闭最后一盏灯
  while(1){
    if(p->next == NULL){
      turn_down(p->y,p->x);
      break;
    }
    p = p->next;
  }
  switch(towards){
    case 'W':
            head->y -= 1;
            judge();
            Serial.println("向上一步走");
            break;
    case 'A':
            head->x -= 1;
            judge();
            Serial.println("向左一步走");
            break;
    case 'S':
            head->y += 1;
            judge();
            Serial.println("向下一步走");
            break;
    case 'D':
            head->x += 1;
            judge();
            Serial.println("向右一步走");
            break;
  }
  ChangeBody(y,x);
}
void ChangeBody(int y,int x){
  snake *p = head->next;
  int pos_a,pos_b,pos_x,pos_y;
  pos_a = x;
  pos_b = y;
  while(p!=NULL){
    pos_x = p->x;
    pos_y = p->y;
    p->x = pos_a;
    p->y = pos_b;
    pos_a = pos_x;
    pos_b = pos_y;
    p = p->next;
  }
  if(head->x == food.x&&head->y == food.y){
    snake *_new = (snake*)malloc(sizeof(snake));
    p = head;
    while(p->next!=NULL){
      p = p->next;
    }
    _new->x = pos_a;
    _new->y = pos_b;
    p->next = _new;
    _new ->next = NULL;
    turn_on_body(head->y,head->x);
    turn_on_body(_new->y,_new->x);
    //吃到食物时，蛇头由食物的绿变红，蛇尾需要重新点亮
    Serial.println("吃到食物");
    snake_len++;
    create_food();
  }else{
    turn_on_body(head->y,head->x);
    //没吃到食物，根据正常行进，前面已经turn_down蛇尾，这会儿点亮蛇头就行
  }
  FastLED.show();
}
void judge(){
  if(maps[head->y][head->x]==3||maps[head->y][head->x]==1){
    while(1){
      end_game();
      delay(1000);
    }
  }
}
void end_game(){
  for(int i = 0;i < MAXLED;i++){
    leds[i] = CRGB(0,0,0);
  }
  FastLED.show();
}
//蛇身在哪个方向前进
//只需要点亮蛇头
void turn_on_body(int y,int x){
  int pos;
  pos = (y + 1) * 22 - 1 - x; 
  if(y % 2 ==1){
    leds[pos] = CRGB(255,0,0);
  }else{
    leds[y * 22 + x] = CRGB(255,0,0);
  }
//  FastLED.show();
  maps[y][x] = 3;
}
void turn_down(int y,int x){
  int pos;
  pos = (y + 1) * 22 - 1 - x;
  if(y%2==1){
    leds[pos] = CRGB(0,0,0);
  }else{
    leds[y*22+x] = CRGB(0,0,0);
  }
//  FastLED.show();
  maps[y][x] = 0;
}
void welcome(){
  leds[0] = CRGB(255,0,0);
  leds[1] = CRGB(255,0,0);
  for(int i=2;i<483;i++){
    if(opt!='I'){
      Serial.println("已经变了");
      break;
    }
    leds[i] = CRGB(255,0,0);
    leds[i+1] = CRGB(255,0,0);
    leds[i-2] = CRGB(0,0,0);
    FastLED.show();
    delay(80);
  }
}
void normal_mode(){
  clear_all();
  create_walls();
  create_food();
  create_snake();
  FastLED.show();
  delay(1000);
  while(opt == 'N'){
    snake_moving();
    if(snake_len<=10){
      delay(600);
    }else if(snake_len<=20){
      delay(475);
    }else{
      delay(350);
    }
  }
}
void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();
  //初始值为I（init），即在welcome界面，一旦接收到N（normal）就进入正常模式
  if(opt == 'I'){
    welcome();
  }else if(opt == 'N'){
    normal_mode();
  }
}
