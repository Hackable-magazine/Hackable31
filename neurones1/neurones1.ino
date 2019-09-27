#include <NeuralNetwork.h>

#define ITER 4000

unsigned int couches[] = {3, 12, 12, 1};
float *sorties;

const float entrees2[6][3] = {
  {0.64, 0.18, 0.18}, // ROUGE
  {0.26, 0.50, 0.24}, // VERT
  {0.13, 0.35, 0.51}, // BLEU
  {0.43, 0.41, 0.17}, // JAUNE
  {0.30, 0.29, 0.41}, // VIOLET
  {0.58, 0.26, 0.15}, // ORANGE
};

const float entrees[6][3] = {
  {1.00, 0.00, 0.00}, // ROUGE
  {0.00, 1.00, 0.00}, // VERT
  {0.00, 0.00, 1.00}, // BLEU
  {1.00, 1.00, 0.00}, // JAUNE
  {0.00, 1.00, 1.00}, // CYAN
  {1.00, 0.00, 1.00}, // MAGENTA (couleur limite)
};

const float sortiesAttendues[6][1] = {
  {1}, // ROUGE
  {0}, // VERT
  {0}, // BLEU
  {1}, // JAUNE
  {0}, // CYAN
  {1}, // MAGENTA
};

void setup() {
  Serial.begin(115200);

  NeuralNetwork NN(couches, 4);

  for (int i = 0; i < ITER; i++) {
    if(i%(ITER/100)==0) {
      Serial.print("Entrainement ");
      Serial.print(i/(ITER/100));
      Serial.println("%");
    }
    for (int j = 0; j < 6; j++) {
      NN.FeedForward(entrees[j]);
      NN.BackProp(sortiesAttendues[j]);
    }
  }

  for (int i = 0; i < 6; i++) {
    sorties = NN.FeedForward(entrees[i]);
    Serial.print(i);
    Serial.print(":  ");
    Serial.println(sorties[0], 7);
  }
  
  Serial.println();
  for (int i = 0; i < 6; i++) {
    sorties = NN.FeedForward(entrees2[i]);
    Serial.print(i);
    Serial.print(":  ");
    Serial.println(sorties[0], 7);
  }

  NN.print();
}

void loop() {
}
