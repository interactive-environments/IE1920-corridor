void setupWave() {
  
}

void loopWave() {
  
}

void loopWaveDebug() {
  // Make the first LED red if there is presence here.
  setLight(0, 255 * states[0].hasPresence(), 0, 0);

  bool nearbyPresence = false;
  for (int offset = lowestNegative; offset <= highestPositive; offset++) {
    bool isUnitOpen = states[(offset + MAX_UNITS) % MAX_UNITS].hasPresence();
    Serial.print(offset);
    Serial.print(" = ");
    Serial.println(isUnitOpen);

    if (offset == -1 || offset == 1) {
      nearbyPresence |= isUnitOpen;
    }
  }

  // Make the second LED blue if there is presence nearby.
  setLight(1, 0, 0, 255 * nearbyPresence);
}
