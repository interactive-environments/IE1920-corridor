#define RATE 9600
#define MAX_UNITS 12

#define MAX_PACKET_LENGTH 12

UnitState states[MAX_UNITS];
int lowestNegative = 0;
int highestPositive = 0;

Stream* Conn[2];

int stateIndex(int offset) {
  if (offset < lowestNegative || offset > highestPositive) {
    Serial.print("Invalid state index requested: ");
    Serial.println(offset);
    return -1;
  }
  return (offset + MAX_UNITS) % MAX_UNITS;
}

void setupComms() {
  Conn[0] = &Serial1;
  Conn[1] = &Serial2;

  Serial1.begin(RATE);
  Serial2.begin(RATE);

  for (int i = 0; i < 2; i++) {
    Conn[i]->setTimeout(5);
  }
}

void handlePacketReceived(int offset, String packet) {
  int index = stateIndex(offset);
  bool hadPresence = states[index].hasPresence();

  states[index].handlePacket(packet);

  // If this packet made a unit gain presence, disable neighbouring presence.
  if (!hadPresence && states[index].hasPresence()) {
    if (offset > lowestNegative) {
      states[stateIndex(offset - 1)].forceTimeout();
    }
    if (offset < highestPositive) {
      states[stateIndex(offset + 1)].forceTimeout();
    }
  }
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

      packet = Conn[i]->readStringUntil('\n');
      packet = packet.substring(0, packet.length() - 1);

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

      handlePacketReceived(offset, packet);

      // Chain forward
      Conn[i ^ 1]->print(offset - 1 + 2 * i);
      Conn[i ^ 1]->println(packet);
    }
  }
}

void broadcastPacket(String packet) {
  // Handle the packet ourselves first.
  handlePacketReceived(0, packet);
  for (int i = 0; i < 2; i++) {
    int offset = 1 - 2 * i;
    Conn[i]->print(offset);
    Conn[i]->println(packet);
  }
}
