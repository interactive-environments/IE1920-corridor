#define RATE 115200

#define MAX_PACKET_LENGTH 12
#define MAX_RECV_LENGTH 64

UnitIndexer units;

/**
 * Index 0 is the previous unit, while index 1 is the next unit.
 * Checksums are used to make sure the message is not corrupted,
 * although this does not matter much yet as the message is only
 * a predefined one. We do want to add tunable variables with buttons,
 * so when that system is implemented the checksums will be useful.
 */
Stream* Conn[2];
String buff[2];
bool hasChecksum[2];
byte checksum[2];

void setupComms() {
  Conn[0] = &Serial2;
  Conn[1] = &Serial3;

  Serial2.begin(RATE);
  Serial3.begin(RATE);

  for (int i = 0; i < 2; i++) {
    hasChecksum[i] = false;
  }
}

/**
 * Send every packet multiple times to increase the chance of the packet
 * arriving properly.
 */
void sendRedundantPacket(int line, int fromOffset, String packet) {
  int redundancy = getConfigi(COMMS_REDUNDANCY);
  for (int i = 0; i < redundancy; i++) {
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

/**
 * Sends a packet to all units by chaining.
 * When higher level classes decide on an action that should be seen
 * by all units, including this one, the packet should be broadcast
 * using this method.
 */
void broadcastPacket(String packet) {
  // Handle the packet ourselves first.
  units.handlePacket(0, packet);
  sendRedundantPacket(1, -1, packet); // To Right
  sendRedundantPacket(0, 1, packet); // To Left
}

void packetReceived(int i) {
  // Check if the checksum is not corrupted.
  if (checksum[i] == 0) {
    int offset = int(int8_t(buff[i].charAt(0)));
    String packet = buff[i].substring(1);

    units.handlePacket(offset, packet);
    // Forward packet to next in chain.
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

/**
 * Read asynchronously (without blocking) by checking for new data
 * in a loop. The format of a packet is as follows:
 * #{checksum}{offset}{data}\r\n
 * The checksum is calculated by XOR-ing the offset with
 * all data bytes.
 */
void loopComms() {
  String packet;
  for (int i = 0; i < 2; i++) {
    int j = 0;
    while (Conn[i]->available() && j++ < MAX_RECV_LENGTH) {
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
