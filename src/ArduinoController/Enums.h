

enum MessageType { 
  COM1ACTV = 100,
  COM1STBY = 110,
  ALTITUDE = 120,
  NAV1ACTV = 130,
  NAV1STBY = 140,
};

enum ActivityType { 
  COM1_MAJOR_UP = 100,
  COM1_MAJOR_DOWN = 101,
  COM1_MINOR_UP = 110,
  COM1_MINOR_DOWN = 111,
  COM1_SWAP = 120,
  NAV1_MAJOR_UP = 200,
  NAV1_MAJOR_DOWN = 201,
  NAV1_MINOR_UP = 210,
  NAV1_MINOR_DOWN = 211,
  NAV1_SWAP = 220,
};
