#define RATE 115200
#define MAX_UNITS 12

#define MAX_PACKET_LENGTH 12

UnitState states[MAX_UNITS];
int lowestNegative = 0;
int highestPositive = 0;

Stream* Conn[2];
String buff[2];
bool hasChecksum[2];
byte checksum[2];

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
    // Conn[i]->setTimeout(4);
    hasChecksum[i] = false;
  }
}

void handlePacket(int offset, String packet) {
  if (offset < lowestNegative) {
    if (highestPositive - offset >= MAX_UNITS) {
      Serial.println("Did not process packet, negative offset can not fit in buffer.");
      return;
    }
    lowestNegative = offset;
  } else if (offset > highestPositive) {
    if (offset - lowestNegative >= MAX_UNITS) {
      Serial.println("Did not process packet, positive offset can not fit in buffer.");
      return;
    }
    highestPositive = offset;
  }

  Serial.print("RECV ");
  Serial.print(offset);
  Serial.print(": ");
  Serial.println(packet);

  int index = stateIndex(offset);
  bool hadPresence = states[index].hasPresence();

  states[index].handlePacket(packet);

  // If this packet made a unit gain presence, disable neighbouring presence.
  // To-Do: Move this code into a presence wrapper class.
  if (!hadPresence && states[index].hasPresence()) {
    if (offset > lowestNegative) {
      states[stateIndex(offset - 1)].forceTimeout();
    }
    if (offset < highestPositive) {
      states[stateIndex(offset + 1)].forceTimeout();
    }
  }
}

void sendPacket(int line, int fromOffset, String packet) {
  byte checksum = int8_t(fromOffset);
  for (int i = 0; i < packet.length(); i++) {
    checksum ^= packet.charAt(i);
  }
  Conn[line]->print('#');
  Conn[line]->write(checksum);
  Conn[line]->write(byte(int8_t(fromOffset)));
  Conn[line]->println(packet);
}

void broadcastPacket(String packet) {
  // Handle the packet ourselves first.
  handlePacket(0, packet);
  sendPacket(1, -1, packet); // To Right
  sendPacket(0, 1, packet); // To Left
}

void packetReceived(int i) {
  if (checksum[i] == 0) {
    int offset = int(int8_t(buff[i].charAt(0)));
    String packet = buff[i].substring(1);

    handlePacket(offset, packet);
    if (i == 0) {
      sendPacket(1, offset - 1, packet); // To Right
    } else {
      sendPacket(0, offset + 1, packet); // To left
    }
  } else {
    Serial.print("Invalid packet from ");
    Serial.print(i);
    Serial.print(": ");
    Serial.println(buff[i]);
  }
  buff[i] = "";
}

void loopComms() {
  String packet;
  for (int i = 0; i < 2; i++) {
    while (Conn[i]->available()) {
      byte b = Conn[i]->read();
      switch (b) {
        case '\r': // Skip carriage return
          break;
        case '\n': // End character
          packetReceived(i);
        case '#': // Reset character
          hasChecksum[i] = false;
          break;
        default:
          if (hasChecksum[i]) {
            checksum[i] ^= b;
            buff[i] += char(b);
          } else {
            hasChecksum[i] = true;
            checksum[i] = b;
          }
          // Buffer full, process packet
          if (buff[i].length() == MAX_PACKET_LENGTH) {
            packetReceived(i);
            hasChecksum[i] = false;
          }
          break;
      }
    }
  }
}
