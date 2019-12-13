#define RATE 115200

#define MAX_PACKET_LENGTH 12
#define REDUNDANCY 3

UnitIndexer units;

Stream* Conn[2];
String buff[2];
bool hasChecksum[2];
byte checksum[2];

void setupComms() {
  Conn[1] = &Serial2;
  Conn[0] = &Serial3;

  Serial2.begin(RATE);
  Serial3.begin(RATE);

  for (int i = 0; i < 2; i++) {
    hasChecksum[i] = false;
  }
}

void sendRedundantPacket(int line, int fromOffset, String packet) {
  for (int i = 0; i < REDUNDANCY; i++) {
    sendPacket(line, fromOffset, packet);
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
  units.handlePacket(0, packet);
  sendRedundantPacket(1, -1, packet); // To Right
  sendRedundantPacket(0, 1, packet); // To Left
}

void packetReceived(int i) {
  if (checksum[i] == 0) {
    int offset = int(int8_t(buff[i].charAt(0)));
    String packet = buff[i].substring(1);

    units.handlePacket(offset, packet);
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
