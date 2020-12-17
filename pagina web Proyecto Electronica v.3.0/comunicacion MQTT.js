usuario = 'projectXesp8266';
contrasena = '19951998';
estadoRiego = "OFF";
humSuelo = 0;
temperaturas = 0;
humAmbiente = 0;

//Se establece la comunicacion con cloudMQTT
function OnOff(dato){ //OnOff es la funcion para encender y apagar el led
  message = new Paho.MQTT.Message(dato);
  message.destinationName = '/' + usuario + '/riegoManual' //comprueba si el mensaje es igual al topic
  client.send(message); //envia el mensaje si es on/off
};

function enviarSalidaAnalogica(){//funcion para configurar la hum Minima
  //var dato = 25;
  var dato = document.getElementById("myRange").value; //dato va ser igual a variable myrange
  message = new Paho.MQTT.Message(dato);
  message.destinationName = '/' + usuario + '/salidaAnalogica'  //comprueba si el mensaje es igual al topic
  client.send(message);
};
 
// called when the client connects
function onConnect() {
  //Once a connection has been made, make a subscription and send a message.
  console.log("onConnect");
  client.subscribe("#"); //subscripcion de todos los componentes para hacer publicaciones
}
  
// called when the client loses its connection
function onConnectionLost(responseObject) {
  if (responseObject.errorCode !== 0) {
    console.log("onConnectionLost:", responseObject.errorMessage);
    setTimeout(function() { client.connect() }, 5000);
  }
}
  
// called when a message arrives
function onMessageArrived(message) {
  if (message.destinationName == '/' + usuario + '/' + 'humedadDelSuelo') { //acá ponemos el topic
      document.getElementById("humedadDelSuelo").textContent = message.payloadString  + " %";
      //g.refresh(message.payloadString);
      humSuelo = parseFloat(message.payloadString);
  }//imprimimos temperatura, luego el mensaje que llega y ºc
 
  if (message.destinationName == '/' + usuario + '/' + 'riegoManual') { 
      document.getElementById("riegoManual").textContent = message.payloadString;
  }
  if (message.destinationName == '/' + usuario + '/' + 'salidaAnalogica') { 
      document.getElementById("salidaAnalogica").textContent = message.payloadString + "%";
  }
  if (message.destinationName == '/' + usuario + '/' + 'temperaturaAmbiente') { //acá ponemos el topic
      document.getElementById("temperaturaAmbiente").textContent = message.payloadString  + " ºC";
      //g.refresh(message.payloadString);
      temperaturas = parseFloat(message.payloadString);
  }//imprimimos temperatura, luego el mensaje que llega y °c				
  if (message.destinationName == '/' + usuario + '/' + 'humedadAmbiente') { //acá ponemos el topic
      document.getElementById("humedadAmbiente").textContent = message.payloadString  + " %";
      //g.refresh(message.payloadString);
      humAmbiente = parseFloat(message.payloadString);
  }//imprimimos humedad, luego el mensaje que llega y %

  if(humSuelo == 0.0){ 
    alert("El sensor de humedad esta desconectado.");
   }
}

function onFailure(invocationContext, errorCode, errorMessage) {
  var errDiv = document.getElementById("error");
  errDiv.textContent = "Could not connect to WebSocket server, most likely you're behind a firewall that doesn't allow outgoing connections to port 39627";
  errDiv.style.display = "block";
}
//crea un ID de cliente aleatorio
var clientId = "ws" + Math.random(); 
// crea una instancia con el cliente
var client = new Paho.MQTT.Client("m15.cloudmqtt.com", 39262, clientId); //puerto websockets

// set callback handlers
client.onConnectionLost = onConnectionLost;
client.onMessageArrived = onMessageArrived;

// conecta con el client
client.connect({
  useSSL: true,
  userName: usuario,
  password: contrasena,
  onSuccess: onConnect,
  onFailure: onFailure
});

  