import 'package:flutter/material.dart';
import 'package:mqtt_client/mqtt_client.dart' as mqtt;
import 'model/message.dart';
import 'dart:async';

void main() => runApp(MyApp());

class MyApp extends StatelessWidget {
  // This widget is the root of your application.
  @override
  Widget build(BuildContext context) {
    return MaterialApp(
      debugShowCheckedModeBanner: false,
      title: 'snake',
      home: MyHomePage(title: 'snake'),
    );

  }
}

class MyHomePage extends StatefulWidget{
  MyHomePage({Key key,this.title}) : super(key:key);
  final String title;
  @override
  _MyHomePageState createState() => _MyHomePageState();
}
class _MyHomePageState extends State<MyHomePage>{
  int snake_len = 3;
  int score = 0;
  String _pubControl = 'control';
  String _pubModeTopic = 'mode';
  String _subSnakeLen = 'snake_len';
  String _subScore = 'score';
  String _pubMsg;
  bool _retainValue = false;
  ScrollController subMsgScrollController = new ScrollController();
  String broker = '47.102.201.222';
  mqtt.MqttClient client;
  mqtt.MqttConnectionState connectionState;
  StreamSubscription subscription;
  List<Message> messages = <Message>[];
  @override
  void initState(){
    super.initState();
    _connect();
    _subMessage();
  }
  @override
  Widget build(BuildContext context) {
    return Scaffold(
      appBar: new AppBar(title: Text(widget.title),),
      body: new Column(
        children: <Widget>[
          new Row(
            children: <Widget>[
              new Container(
                padding: EdgeInsets.fromLTRB(45, 0, 0, 0),
                child: RaisedButton(
                  child: Text('开始模式1'),
                  onPressed: (){
                    _pubMsg = 'N';
                    _pubMode();
                    score = 0;
                    _subMessage();
                  },
                  color: Colors.blueAccent,
                ),
              ),
              new Container(
                padding: EdgeInsets.fromLTRB(20, 0, 0, 0),
                child: RaisedButton(
                  child: Text('开始模式2'),
                  onPressed: (){
                    _pubMsg = 'M';
                    _pubMode();
                    score = 0;
                    _subMessage();
                  },
                ),
              ),
              new Container(
                padding: EdgeInsets.fromLTRB(20, 0, 0, 0),
                child: RaisedButton(
                  child: Text('开始模式3'),
                  onPressed: (){
                    _pubMsg = 'Y';
                    _pubMode();
                    score = 0;
                    _subMessage();
                  },
                  color: Colors.greenAccent
                ),
              ),
            ],
          ),
          new Row(
            children: <Widget>[
              new Container(
                padding: EdgeInsets.fromLTRB(150, 20, 0, 0),
                child:Text(
                  "蛇长："+
                  snake_len.toString()
                ),
              )
            ],
          ),
          new Row(
            children: <Widget>[
              new Container(
                padding: EdgeInsets.fromLTRB(150, 20, 0, 0),
                child: Text(
                  "得分："+
                    score.toString()
                ),
              )
            ],
          ),
          new Row(
            children: <Widget>[
              new Container(
//                width: 250,
                padding: EdgeInsets.fromLTRB(150, 100, 0, 25),
                child: RaisedButton(
                  child: Text('↑'),
                  onPressed: (){
                    _pubMsg = 'W';
                    _pubMessage();
                  },
                  color: Colors.yellowAccent,
                ),
              ),
            ],
          ),
          new Row(
            children: <Widget>[
              new Container(
//                width: 150,
                padding: EdgeInsets.fromLTRB(50, 0, 50, 25),
                child: RaisedButton(
                    child:  Text('←'),
                    onPressed:(){
                      _pubMsg = 'A';
                      _pubMessage();
                    },
                    color: Colors.yellowAccent,
                ),
              ),
              new Container(
//                width: 150,
                padding: EdgeInsets.fromLTRB(50, 0, 0, 25),
                child: RaisedButton(
                  onPressed: (){
                    _pubMsg = 'D';
                    _pubMessage();
                  },
                  child: Text('→'),
                  color: Colors.yellowAccent,
                ),
              ),
            ],
          ),
          new Row(
            children: <Widget>[
              new Container(
                padding: EdgeInsets.fromLTRB(150, 0, 0, 0),
                child: RaisedButton(
                    onPressed:(){
                      _pubMsg = 'S';
                      _pubMessage();
                    },
                    child: Text('↓'),
                    color: Colors.yellowAccent,
                ),
              )
            ],
          )

        ],
      ),
    );

  }
  void _connect() async{
    //client连接的初始化配置
    //默认端口1883，如果不是1883就采用
    //client = mqtt.MqttClient.withPort(broker, '',1883);
    client = mqtt.MqttClient(broker,'');
    client.logging(on: true);

    client.keepAlivePeriod = 30;

    client.onDisconnected = _onDisconnected;

    final mqtt.MqttConnectMessage connMess = mqtt.MqttConnectMessage()
        .withClientIdentifier('webberFlutter')
        .startClean()
        .keepAliveFor(30)
        .withWillTopic('willtopic')
        .withWillMessage('My Will message')
        .withWillQos(mqtt.MqttQos.atLeastOnce);
    print('MQTT client connecting....');
    client.connectionMessage = connMess;
    try {
      await client.connect();
    } catch (e) {
      print(e);
      _disconnect();
    }

    if (client.connectionState == mqtt.MqttConnectionState.connected) {
      print('MQTT client connected');
      setState(() {
        connectionState = client.connectionState;
      });

    } else {
      print('ERROR: MQTT client connection failed - '
          'disconnecting, state is ${client.connectionState}');

      _disconnect();
    }

    subscription = client.updates.listen(_onMessage);

  }
  void _disconnect() {
    client.disconnect();
    _onDisconnected();
  }

  void _onDisconnected() {
    setState(() {
      connectionState = client.connectionState;
      client = null;
//      subscription.cancel();
//      subscription = null;
    });
    print('MQTT client disconnected');
  }
  void _onMessage(List<mqtt.MqttReceivedMessage> event) {

    print(event.length);
    print(event[0].topic);

    final mqtt.MqttPublishMessage recMess = event[0].payload as mqtt.MqttPublishMessage;
    final String message = mqtt.MqttPublishPayload.bytesToStringAsString(recMess.payload.message);

//    final mqtt.MqttSubscribeAckMessage subMess;

    print('MQTT message: topic is <${event[0].topic}>, '
        'payload is <-- ${message} -->');
    print(client.connectionState);
    setState(() {
      if(event[0].topic=='snake_len'){
        snake_len = int.parse(message);
        print(snake_len);
      }else if(event[0].topic=='score'){
        score = int.parse(message);
        print(score);
      }
    });

  }
  void _subMessage(){
    //开始接收subtopic的submessage
    print("on sub message");
    if(connectionState == mqtt.MqttConnectionState.connected){
      setState(() {
        print('subscribe to ${_subSnakeLen}');
        client.subscribe(_subSnakeLen, mqtt.MqttQos.exactlyOnce);
        client.subscribe(_subScore, mqtt.MqttQos.exactlyOnce);
      });
    }

//      come =  messages.map((Message message){message.message;}).toString();

  }
  void _pubMessage(){
    //发布控制消息

    final mqtt.MqttClientPayloadBuilder builder =
    mqtt.MqttClientPayloadBuilder();

    builder.addString(_pubMsg);
    print("pub message ${_pubControl}:${_pubMsg}");
    client.publishMessage(
      _pubControl,
      mqtt.MqttQos.values[0],
      builder.payload,
      retain: _retainValue,
    );
  }
  void _pubMode(){
    //发布控制消息

    final mqtt.MqttClientPayloadBuilder builder =
    mqtt.MqttClientPayloadBuilder();

    builder.addString(_pubMsg);
    print("pub message ${_pubModeTopic}:${_pubMsg}");
    client.publishMessage(
      _pubModeTopic,
      mqtt.MqttQos.values[0],
      builder.payload,
      retain: _retainValue,
    );
  }
}