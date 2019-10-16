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
//int logo[52] = {177,217,219,221,263,261,265,180,215,224,259,268,214,226,183,212,227,256,271,186,187,210,229,254,273,230,231,207,232,251,276,190,205,234,249,278,247,281,203,235,193,283,284,285,195,196,197,200,244,239,240,241};
int wall3[44] = {51,80,95,124,139,168,169,170,171,172,173,58,73,102,117,146,161,160,159,158,157,156,310,311,312,313,314,315,344,359,388,403,432,327,326,325,324,323,322,337,366,381,410,425};
char towards = 'D';
char opt = '0';
int no = 2;
int single_score = 100;//分数的计数单位
int now_score = 100;//当前关卡的分数单位
char length_msg[3];
int maps[22][22];//0:nothing 1:wall 2:food 3:snake 
int snake_len = 3;
int score = 0;
char score_msg[6];
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
void create_snake2(){
  head = (snake*)malloc(sizeof(snake));
  head->x = 6;
  head->y = 6;
  snake *p = (snake*)malloc(sizeof(snake));
  snake *q = (snake*)malloc(sizeof(snake));
  p->x = 5;
  p->y = 6;
  q->x = 4;
  q->y = 6;
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
  if(strcmp(topic,"mode")==0){
    opt = (char)payload[0];   
    Serial.println(opt);        
  }else if(strcmp(topic,"control")==0){
    towards = (char)payload[0];
    Serial.println(towards);
  }
}
void clear_all(){
  for(int i = 0;i<484;i++){
    leds[i] = CRGB(0,0,0);
  }
  for(int i = 0;i<22;i++){
    for(int j = 0;j<22;j++){
      maps[i][j] = 0;
    }
  }
  snake_len = 3;
  score = 0;
  itoa(score,score_msg,10);
  client.publish("score",score_msg);
}
void create_wall2(){
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
}
void create_wall3(){
  for(int i = 0;i<44;i++){
    leds[wall3[i]] = CRGB(0,0,255);
    if((wall3[i]/22)%2==0){
      maps[wall3[i]/22][wall3[i]%22] = 1;
    }else{
      maps[wall3[i]/22][21-wall3[i]%22] = 1;
    }
  }
}
void create_wall4(){
  for(int i = 0;i<22;i++){
    for(int j = 0;j<22;j++){
      if((i!=10&&i!=11)&&((i==j)||(i+j)==21)){
        maps[i][j] = 1;
      }
    }
  }
  for(int i = 0;i < MAXLED;i++){
    if(maps[i/22][i%22] == 1){
      leds[i] = CRGB(0,0,255);
    }else{
      leds[i] = CRGB(0,0,0);
    }
  }
}
void create_wall5(){
  int i,j;
  for(j=0;j<10;j++){
    maps[9][j] = 1;
  }
  for(i=0;i<9;i++){
    maps[i][9] = 1;
  }
  for(j=16;j<22;j++){
    maps[5][j] = 1;
    maps[13][j] =1;
  }
  for(i=6;i<13;i++){
    maps[i][16] = 1;
  }
  for(i=16;i<22;i++){
    maps[i][6] = 1;
    maps[i][17] = 1;
  }
  for(j=6;j<18;j++){
    maps[16][j] = 1;
  }
  for(i=0;i<MAXLED;i++){
    if((i/22)%2==0){
       if(maps[i/22][i%22] == 1){
          leds[i] = CRGB(0,0,255);
       }
    }else{
      if(maps[i/22][21-i%22] == 1){
        leds[i] = CRGB(0,0,255);
      }
    }
  }
}
void create_wall6(){
  int i;
  for(i=0;i<16;i++){
    maps[i][8] = 1;
    maps[i][18] = 1;
  }
  for(i=6;i<22;i++){
    maps[i][3] = 1;
    maps[i][13] = 1;
  }
  for(i=0;i<MAXLED;i++){
    if((i/22)%2==0){
       if(maps[i/22][i%22] == 1){
          leds[i] = CRGB(0,0,255);
       }
    }else{
      if(maps[i/22][21-i%22] == 1){
        leds[i] = CRGB(0,0,255);
      }
    }
  }
}
void create_wall7(){
  int i;
  for(i=0;i<22;i++){
    maps[i][10] = 1;
    maps[i][11] = 1;
    maps[10][i] = 1;
    maps[11][i] = 1; 
  }
  for(i=0;i<MAXLED;i++){
    if((i/22)%2==0){
       if(maps[i/22][i%22] == 1){
          leds[i] = CRGB(0,0,255);
       }
    }else{
      if(maps[i/22][21-i%22] == 1){
        leds[i] = CRGB(0,0,255);
      }
    }
  }
}
void create_wall8(){
  int i;
  for(i=3;i<8;i++){
    maps[3][i] = 1;
    maps[18][i] = 1;
    maps[i][3] = 1;
    maps[i][18] = 1;
  }
  for(i=14;i<19;i++){
    maps[i][3] = 1;
    maps[i][18] = 1;
    maps[3][i] = 1;
    maps[18][i] = 1; 
  }
  for(i=7;i<10;i++){
    maps[9][i] = 1;
    maps[12][i] = 1;
    maps[i][9] = 1;
    maps[i][12] = 1;
  }
  for(i=12;i<15;i++){
    maps[9][i] = 1;
    maps[12][i] = 1;
    maps[i][9] = 1;
    maps[i][12] = 1;
  }
  for(i=0;i<MAXLED;i++){
    if((i/22)%2==0){
       if(maps[i/22][i%22] == 1){
          leds[i] = CRGB(0,0,255);
       }
    }else{
      if(maps[i/22][21-i%22] == 1){
        leds[i] = CRGB(0,0,255);
      }
    }
  }
}
void create_wall9(){
  int i;
  for(i=4;i<=11;i++){
    maps[11-i][i] = 1;
    maps[i][i+10] = 1;
    maps[i+7][i-4] = 1;
    maps[i+10][22-i] = 1;
  }
  for(i=0;i<MAXLED;i++){
    if((i/22)%2==0){
       if(maps[i/22][i%22] == 1){
          leds[i] = CRGB(0,0,255);
       }
    }else{
      if(maps[i/22][21-i%22] == 1){
        leds[i] = CRGB(0,0,255);
      }
    }
  }
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
//  int snake_len = 1;
//没看懂这句话为什么写了，应该没用
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
            if(head->y==0){
              head->y = 21;
            }else{
              head->y -= 1;
            }
            judge();
            Serial.println("向上一步走");
            break;
    case 'A':
            if(head->x == 0){
              head->x = 21;
            }else{
              head->x -= 1;
            }
            judge();
            Serial.println("向左一步走");
            break;
    case 'S':
            if(head->y == 21){
              head->y = 0;
            }else{
              head->y += 1;
            }
            judge();
            Serial.println("向下一步走");
            break;
    case 'D':
            if(head->x == 21){
              head->x = 0;
            }else{                                     
              head->x += 1;
            }
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
    int n = snake_len;
    if(n>=10){
      length_msg[0] =  '0'+n/10;
      length_msg[1] = '0'+ n%10;
    }else{
      length_msg[0] = '0'+n;
      length_msg[1] = '\0';
    }
    client.publish("snake_len",length_msg);
    if(snake_len<=10){
      score += now_score;
    }else if(snake_len<=20){
      score += now_score*1.5;
    }else{
      score += now_score*2;
    }
    itoa(score,score_msg,10);
    client.publish("score",score_msg);
    create_food();
  }else{
    turn_on_body(head->y,head->x);
    //没吃到食物，根据正常行进，前面已经turn_down蛇尾，这会儿点亮蛇头就行
  }
  FastLED.show();
}
void judge(){
  if(maps[head->y][head->x]==3||maps[head->y][head->x]==1){
    opt = 'E';
    while(opt=='E'){
      end_game();
      delay(1000);
      client.loop();
      check_mode('E');
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
//  while(opt=='I'){
//    client.loop();
//    if(opt!='I'){
//      break;
//    }
//    for(int i = 0;i<52;i++){
//      leds[logo[i]] = CRGB(255,68,0);
//    }
//    FastLED.show();
//    delay(1000); 
//  }
  
  leds[0] = CRGB(255,0,0);
  leds[1] = CRGB(255,0,0);
  for(int i=2;i<483;i++){
    client.loop();
    if(opt!='0'){
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
void mode3(){
  clear_all();
  create_wall3();
  create_food();
  create_snake();
  FastLED.show();
  delay(1000);
  client.publish("snake_len","3");
  while(opt == '3'){
    client.loop();
    check_mode('3');
    snake_moving();
    delay_time();
  }
}
void infinity_mode(){
  clear_all();
  create_food();
  create_snake();
  FastLED.show();
  delay(1000);
  client.publish("snake_len","3");
  while(opt == '1'){
    client.loop();
    check_mode('1');
    snake_moving();
    delay_time();
  }
}
void mode2(){
  clear_all();
  create_wall2();
  create_food();
  create_snake();
  FastLED.show();
  delay(1000);
  client.publish("snake_len","3");
  while(opt == '2'){
    client.loop();
    check_mode('2');
    snake_moving();
    delay_time();
  }
}
void mode4(){
  clear_all();
  create_wall4();
  create_food();
  create_snake();
  FastLED.show();
  delay(1000);
  client.publish("snake_len","3");
  towards = 'D';
  while(opt == '4'){
    client.loop();
    check_mode('4');
    snake_moving();
    delay_time();
  }
}
void mode5(){
  clear_all();
  create_wall5();
  create_food();
  create_snake();
  FastLED.show();
  delay(1000);
  client.publish("snake_len","3");
  while(opt == '5'){
    client.loop();
    check_mode('5');
    snake_moving();
    delay_time();
  }
}
void mode6(){
  clear_all();
  create_wall6();
  create_food();
  create_snake();
  towards = 'S';
  FastLED.show();
  delay(1000);
  client.publish("snake_len","3");
  while(opt == '6'){
    client.loop();
    check_mode('6');
    snake_moving();
    delay_time();
  }
}
void mode7(){
  clear_all();
  create_wall7();
  create_food();
  create_snake2();
  towards = 'D';
  FastLED.show();
  delay(1000);
  client.publish("snake_len","3");
  while(opt == '7'){
    client.loop();
    check_mode('7');
    snake_moving();
    delay_time();
  }
}
void mode8(){
  clear_all();
  create_wall8();
  create_food();
  create_snake();
  towards = 'D';
  FastLED.show();
  delay(1000);
  client.publish("snake_len","3");
  while(opt == '8'){
    client.loop();
    check_mode('8');
    snake_moving();
    delay_time();
  }
}
void mode9(){
  clear_all();
  create_wall9();
  create_food();
  create_snake();
  towards = 'D';
  FastLED.show();
  delay(1000);
  client.publish("snake_len","3");
  while(opt == '9'){
    client.loop();
    check_mode('9');
    snake_moving();
    delay_time();
  }
}
//用于检查当前是不是切换了mode
void check_mode(char now_mode){
  if(opt!=now_mode){
    if(opt == '0'){
      welcome();
    }else if(opt == '2'){
      mode2();
      //四面都是墙
    }else if(opt == '1'){
      infinity_mode();
      //无墙
    }else if(opt == '3'){
      mode3();
      //四个L字墙
    }else if(opt == '4'){
      mode4();
    }else if(opt == '5'){
      mode5();
    }else if(opt == '6'){
      mode6();
    }else if(opt == '7'){
      mode7();
    }else if(opt == '8'){
      mode8();
    }else if(opt == '9'){
      mode9();
    }else if(opt == 'B'){
      battle_mode();
      //闯关模式，吃十五个食物进入下一关
    }
  }
}
void delay_time(){
  if(snake_len<=10){
      delay(475);
    }else if(snake_len<=15){
      delay(375);
    }else{
      delay(275);
   }
}
void battle_mode(){
  while(opt=='B'){
    clear_all();
    switch(no){
      case 2:
        create_wall2();
        now_score = single_score;
        break;
      case 3:
        create_wall3();
        now_score = single_score*1.5;
        break;
      case 4:
        create_wall4();
        now_score = single_score*2;
        break;
      case 5:
        create_wall5();
        now_score = single_score*2.5;
        break;
      case 6:
        create_wall6();
        now_score = single_score*3;
        break;
      case 7:
        create_wall7();
        now_score = single_score*3.5;
        break;
      case 8:
        create_wall8();
        now_score = single_score*4.0;
        break;
      case 9:
        create_wall9();
        now_score = single_score*4.5;
        break;
     }
     create_food();
     if(no==7){
      create_snake2();
     }else{
      create_snake();
     }
     FastLED.show();
     delay(1000);
     client.publish("snake_len","3");
     while(opt == 'B'){
      client.loop();
      snake_moving();
      delay_time();
      if(snake_len==15){
        no++;
        if(no==6){
          towards='S';
        }else{
          towards='D';
        }
        break;
      }
    }
  }
}
void loop() {
  if(!client.connected()){
    reconnect();
  }
  client.loop();
  //初始值为I（init），即在welcome界面，一旦接收到N（normal）就进入正常模式
  if(opt == '0'){
    welcome();
  }else if(opt == '2'){
    mode2();
    //四面都是墙
  }else if(opt == '1'){
    infinity_mode();
    //无墙
  }else if(opt == '3'){
    mode3();
    //四个L字墙
  }else if(opt == '4'){
    mode4();
  }else if(opt == '5'){
    mode5();
  }else if(opt == '6'){
    mode6();
  }else if(opt == '7'){
    mode7();
  }else if(opt == '8'){
    mode8();
  }else if(opt == '9'){
    mode9();
  }else if(opt == 'B'){
    battle_mode();
    //闯关模式，吃十五个食物进入下一关
  }
}
