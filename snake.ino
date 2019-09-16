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
int maps[22][22];//0:nothing 1:wall 2:food 3:snake 
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
   towards = (char)payload[0];
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
  create_walls();
  create_food();
  create_snake();
  Serial.println("初始化了");
  FastLED.show();
  delay(1000);
}
void reconnect(){
  while(!client.connected()){
    Serial.print("Attempting MQTT connection...");
    if(client.connect("ESP32Client")){
      Serial.println("connected");
      client.subscribe("control");
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
  snake *p = head;
  snake *q = p->next;
  while(1){
    if(q->next == NULL){
      turn_down(q->y,q->x);
//      p->next = NULL; //本身长度不减，不能删除节点
      Serial.println("关掉最后一盏灯");
      break;
    }
    snake_len++;
    q = q->next;
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
//  turn_on_body(head->y,head->x,1);  
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
    snake *p = head;
    while(p->next!=NULL){
      p = p->next;
    }
    _new->x = pos_a;
    _new->y = pos_b;
    p->next = _new;
    _new ->next =NULL;
    turn_on_body(_new->y,_new->x,2);//状态参数2代表吃到食物
    Serial.println("吃到食物");
    create_food();
  }else{
    turn_on_body(head->y,head->x,1);  
  }
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
void turn_on_body(int y,int x,int sta){
  int pos;
  pos = (y + 1) * 22 - 1 - x;
  if(y%2==1){
    leds[pos] = CRGB(255,0,0);
  }else{
    leds[y*22+x] = CRGB(255,0,0);
  }
  if(sta == 2){
    pos = (head->y + 1) * 22 - 1 - x; 
    if(head->y % 2 ==1){
      leds[pos] = CRGB(255,0,0);
    }else{
      leds[head->y * 22 +head->x] = CRGB(255,0,0);
    }
  }
  FastLED.show();
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
  FastLED.show();
  maps[y][x] = 0;
}
void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();
  snake_moving();
  delay(600);
}
