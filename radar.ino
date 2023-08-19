// OLED Display
#include <Wire.h>               // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306Wire.h"        // legacy: #include "SSD1306.h"
#include "customFonts.h" 

// GPS Module
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

#include <string.h>
#include "FS.h"

// Display Pins SDA > D1 | SCK > D2
SSD1306Wire display(0x3c, D1, D2); // ADDRESS, SDA, SCL

// GPS Pins RX > D3
SoftwareSerial gpsSerial(D3, D4); // RX, TX // Connect only RX pin
TinyGPSPlus gps;

bool    spiffsActive = false;
#define RADARFILE "/sorted_maparadar.txt"

const double searchMargin = 0.1; // 11.1 km // Buscar radares em uma area de 11x11km
const double innerMargin = searchMargin * 0.8; // Margem interna. Ao extrapolar, procura outro radar com longitude proxima

const int tempoAlerta = 12; // Quanto tempo antes do radar comecar a alertar
const double distanciaMinimaAlerta = 0.0008; // 88 metros // Distancia minima em que o radar vai ser alertado
const int margemAngulo = 45; // Margem de direcao para alertar um radar

const int charsInLine = 32; // Quantidade de caracteres em uma linha do arquivo de radares
const int limiteRadares = 400; // Limite de radares a verificar. Tamanho do array de radares.

unsigned long indicatorMillis = 0; // Contador de millis para o indicador de funcionamento
bool indicator = false;

const unsigned char buzzer = D5;

struct radar{
  double lon;
  double lat;
  int type;
  int speed;
  int dirtype;
  int direction;
};

radar radares[limiteRadares];

void setup() {

  pinMode (buzzer,OUTPUT) ;

  Serial.begin(115200);
  Serial.println("");
  Serial.println("Projeto Alerta Radar");

  if (SPIFFS.begin()) {
      Serial.println("SPIFFS Active");
      spiffsActive = true;
  } else {
      Serial.println("Unable to activate SPIFFS");
  }

  
  gpsSerial.begin(9600);
  
  display.init();
  display.clear();
  // display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

}

void loop() {
  readMemory();  
}

static void smartDelay(unsigned long ms) {  // This custom version of delay() ensures that the gps object is being "fed".
  unsigned long start = millis();
  do {
    while (gpsSerial.available()) {
        gps.encode(gpsSerial.read());
    }
  } while (millis() - start < ms);
}

void readMemory(){
  if (spiffsActive) {
    if (SPIFFS.exists(RADARFILE)) {
      File f = SPIFFS.open(RADARFILE, "r");
      if (!f) {
        Serial.println("Unable To Open file for Reading");
      } 
      else {
        Serial.println("Opened file for Reading");
        unsigned long posZero = f.position();
        while(true){
          isGpsWorking();

          smartDelay(0);
          double gpsLon = gps.location.lng();
          double baseLon = gps.location.lng();

          f.seek(posZero);
          getNearRadares(f);

          int camposPreenchidos = 0;
          for (radar r : radares) { // for each element in the array
            if(r.lon != 0){
              camposPreenchidos++;
            }
          }

          Serial.print("Campos preenchidos: " + String(camposPreenchidos) + "/" + String(sizeof(radares)/sizeof(radares[0])));
          
          while((gpsLon >= baseLon - innerMargin) && (gpsLon <= baseLon + innerMargin)){
            smartDelay(0);
            gpsLon = gps.location.lng();

            for (radar r : radares) { // for each element in the array

              if(r.lon != 0){
                bool radarProximo = isRadarProximo(r.lat, r.lon, r.direction, r.dirtype);

                if(radarProximo){
                  while(radarProximo){
                    smartDelay(0);
                    radarProximo = isRadarProximo(r.lat, r.lon, r.direction, r.dirtype);

                    display.clear();
                    defaultDisplay();
                    display.fillCircle(102, 26, 26);
                    display.setColor(BLACK);
                    display.fillCircle(102, 26, 23);
                    display.setColor(WHITE);
                    if(r.speed >= 100){
                      display.setFont(Open_Sans_Bold_24);
                    }else{
                      display.setFont(Open_Sans_Bold_28);
                    }
                    display.setTextAlignment(TEXT_ALIGN_CENTER_BOTH);
                    display.drawString(102, 26, String(r.speed));

                    double distanciaRadar = gps.distanceBetween(gps.location.lat(), gps.location.lng(), r.lat, r.lon);
                    double espacoDeAlerta = calcularMargemDinamica() * 111.1 * 1000;
                    display.drawProgressBar(0, 57, 127, 6, (distanciaRadar / espacoDeAlerta) * 100); 
                    
                    if(gps.speed.kmph() >= r.speed){
                      //Esta acima do limite do radar

                      for(int j=0; j<3; j++){
                        display.invertDisplay();
                        tone(buzzer, 950);
                        smartDelay(100);
                        display.normalDisplay();
                        noTone(buzzer);
                        smartDelay(distanciaRadar * 10);
                      }
                    }

                    display.display();

                  }

                }else {
                  smartDelay(0);
                  display.clear();
                  defaultDisplay();
                  display.display();
                }

              } else{
                break;
              }
            }

            // Serial.println("Loopou");

          }
        }
        Serial.println("Closing File");
        f.close();
      }
      Serial.println();
     
    } else {
      Serial.println("Unable To Find file");
    }
  }
}

int direcaoInversa(int direction){
  if(direction < 180){
    return direction + 180;
  } else if(direction > 180){
    return direction - 180;
  } else return 0;
}

void noGpsDisplay() {
    display.clear();
    
    if(indicatorMillis + 300 < millis()){
      indicator = !indicator;
      indicatorMillis = millis();
    }

    if(indicator){
      display.drawXbm(0, 0, 16, 16, satellite_icon);
    }

    display.display();
}

void buscandoRadaresDisplay(){
  defaultDisplay();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(24, 6, "Buscando Radares");
}

void defaultDisplay() {
    display.setFont(ArialMT_Plain_10);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawXbm(0, 0, 16, 16, satellite_icon);
    display.drawString(18, 6, String(gps.satellites.value()));

    display.setFont(Open_Sans_Bold_36);
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 12, String(int(round(gps.speed.kmph()))));   

    drawStatusIndicator();  
}

void drawStatusIndicator(){ // Indicator that helps knowing if the application is frozen
  if(indicatorMillis + 500 < millis()){
    indicator = !indicator;
    indicatorMillis = millis();
  }
  
  if(indicator){
    display.fillCircle(120, 4, 2); 
  }
}

void isGpsWorking(){
  smartDelay(0);
  double gpsLon = gps.location.lng();

  while(gpsLon == 0){
    smartDelay(500); // Aguardar sinal do GPS
    gpsLon = gps.location.lng();
    noGpsDisplay();
  }
}

double calcularMargemDinamica(){
  double margemDinamica = ((((float)tempoAlerta)/3600)*gps.speed.kmph())/111.1;
  if(margemDinamica < distanciaMinimaAlerta){
    margemDinamica = distanciaMinimaAlerta;
  }
  return margemDinamica;
}

bool isRadarProximo(double lat, double lon, int direction, int dirtype){
  double margemDinamica = calcularMargemDinamica();
  double direcaoAteRadar = gps.courseTo(gps.location.lat(), gps.location.lng(), lat, lon);

  return ((gps.location.lng() >= lon - margemDinamica) && (gps.location.lng() <= lon + margemDinamica)) &&  //Se a longitude estiver dentro da margem
    ((gps.location.lat() >= lat - margemDinamica) && (gps.location.lat() <= lat + margemDinamica)) && //Se a latitude estiver dentro da margem
    (((gps.course.deg() >= direction - margemAngulo) && (gps.course.deg() <= direction + margemAngulo)) || dirtype == 0 || (dirtype == 2 && (gps.course.deg() >= direcaoInversa(direction) - margemAngulo) && (gps.course.deg() <= direcaoInversa(direction) + margemAngulo)) ) && // Dirtype --> 0 > todas as direcoes | 1 > direcao unica | 2 > direcao + direcao oposta
    (((direcaoAteRadar >= direction - margemAngulo) && (direcaoAteRadar <= direction + margemAngulo)) || dirtype == 0 || (dirtype == 2 && (direcaoAteRadar >= direcaoInversa(direction) - margemAngulo) && (direcaoAteRadar <= direcaoInversa(direction) + margemAngulo)) ); // Se estamos nos aproximando do radar

}

//////////////



// Buscar a posicao no arquivo do radar de longitude - 10km e do radar de longitude + 10km
// Varrer todos os radares nessa faixa e salvar em um array os radares com latitude entre lat + 10km e lat - 10km
// Assim terei um array com todos os radares em uma area de 10x10km

//Margem de busca. Vai monitorar os radares em uma area quadrada de {searchMargin}.
// double searchMargin = 0.1; // 11.1 km

void getNearRadares(File f){
  unsigned long millisComeco = millis();

  memset(radares, 0, sizeof(radares));

  Serial.println("Montando array com radares proximos");

  smartDelay(0);

  display.clear();
  //Todo nao mostrar a velocidade
  buscandoRadaresDisplay();
  display.display();

  unsigned long startPos = findNearLon(f, searchMargin, f.position(), f.size());
  unsigned long endPos = findNearLon(f, -searchMargin, f.position(), f.size());

  f.seek(startPos);
  f.readStringUntil('\n');

  double gpsLatStart = gps.location.lat() - searchMargin;
  double gpsLatEnd = gps.location.lat() + searchMargin;

  int index = 0;
  f.seek(startPos);

  while(f.position() < endPos && index < limiteRadares){
            
    double lon= (f.readStringUntil(',')).toDouble();
    double lat= (f.readStringUntil(',')).toDouble();
    int type=(f.readStringUntil(',')).toInt();
    int speed=(f.readStringUntil(',')).toInt();
    int dirtype=(f.readStringUntil(',')).toInt();
    int direction=(f.readStringUntil('\n')).toInt();

    if(lat > gpsLatStart && lat < gpsLatEnd){
      radares[ index ] = {lon, lat, type, speed, dirtype, direction};
      index++;
    }
  }

  Serial.print("Tempo de busca (s): ");
  Serial.println((millis() - millisComeco));

}

//Retorna a posicao no arquivo de um Radar com longitude parecida a do gps
unsigned long findNearLon(File f, double coordinateOffset, unsigned long start, unsigned long end){

  double gpsLon = gps.location.lng() + coordinateOffset;

  // Muda o pointer para a metade do arquivo
  unsigned long metade = (start + end)/2 ;
  f.seek(metade);
  f.readStringUntil('\n');

  double lon= (f.readStringUntil(',')).toDouble();
  f.readStringUntil('\n');

  if((end - start) <= (charsInLine * 5)){ // O radar proximo encontrado esta dentro desses 5
    return f.position();
  }
  
  if (gpsLon < lon){
    return findNearLon(f, coordinateOffset, metade, end);
  } else if (gpsLon >= lon){
    return findNearLon(f, coordinateOffset, start, metade);
  } else {
    Serial.println("Erro. Radar proximo nao encontrado");
    return f.position();
  }

}
