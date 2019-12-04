#define RATE 9600
#define MAX_UNITS 4

UnitState states[MAX_UNITS];
int lowestNegative = 0;
int highestPositive = 0;

Stream* Conn[2];

void setupComms() {
  Conn[0] = &Serial1;
  Conn[1] = &Serial2;

  Serial1.begin(RATE);
  Serial2.begin(RATE);
}

void loopComms() {
  String packet;
  for (int i = 0; i < 2; i++) {
    while (Conn[i]->available()) {
      Serial.print("RECV ");
      Serial.print(i);

      int offset = Conn[i]->parseInt();
      Serial.print(" ");
      Serial.print(offset);

      packet = "";
      while (!packet.endsWith("\n")) {
        if (!Conn[i]->available()) {
          delay(5);
          // If after a 5 ms buffer there is still no new data, skip this packet.
          if (!Conn[i]->available()) {
            break;
          }
        }
        packet += char(Conn[i]->read());
      }

      // No newline at the end means we aborted receiving, so just skip this packet.
      if (!packet.endsWith("\n")) {
        continue;
      }
      packet = packet.substring(0, packet.length() - 2);

      // Update limits
      if (offset < lowestNegative) {
        if (highestPositive - offset >= MAX_UNITS) {
          Serial.println(" Did not process packet, negative offset can not fit in buffer.");
          continue;
        }
        lowestNegative = offset;
      } else if (offset > highestPositive) {
        if (offset - lowestNegative >= MAX_UNITS) {
          Serial.println(" Did not process packet, positive offset can not fit in buffer.");
          continue;
        }
        highestPositive = offset;
      }

      Serial.print(": ");
      Serial.println(packet);

      states[(offset + MAX_UNITS) % MAX_UNITS].handlePacket(packet);

      // Chain forward
      Conn[i ^ 1]->print(offset - 1 + 2 * i);
      Conn[i ^ 1]->println(packet);
    }
  }
}

void broadcastPacket(String packet) {
  // Handle the packet ourselves first.
  states[0].handlePacket(packet);
  for (int i = 0; i < 2; i++) {
    int offset = 1 - 2 * i;
    Conn[i]->print(offset);
    Conn[i]->println(packet);
  }
}
