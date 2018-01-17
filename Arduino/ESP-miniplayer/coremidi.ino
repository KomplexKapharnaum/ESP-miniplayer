#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "AppleMidi.h"

bool coremidi_isConnected = false;

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, AppleMIDI); // see definition in AppleMidi_Defs.h

void coremidi_setup()
{
  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("IMMO-miniplayer");
  //AppleMIDI.invite({10, 2, 7, 20});

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);
}

void coremidi_loop() 
{
  // Listen to incoming notes
  AppleMIDI.run();

  //if (!coremidi_isConnected) AppleMIDI.invite({10, 2, 7, 20});
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name) {
  coremidi_isConnected  = true;
  LOGINLINE(F("Connected to session "));
  LOG(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  coremidi_isConnected  = false;
  LOG(F("Disconnected"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  LOGINLINE(F("Incoming NoteOn from channel:"));
  LOGINLINE(channel);
  LOGINLINE(F(" note:"));
  LOGINLINE(note);
  LOGINLINE(F(" velocity:"));
  LOGINLINE(velocity);
  LOG();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  LOGINLINE(F("Incoming NoteOff from channel:"));
  LOGINLINE(channel);
  LOGINLINE(F(" note:"));
  LOGINLINE(note);
  LOGINLINE(F(" velocity:"));
  LOGINLINE(velocity);
  LOG();
}




