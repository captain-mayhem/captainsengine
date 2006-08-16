//  ==================================================================
// |                         Hero Quest                               |
// |------------------------------------------------------------------|
// |                 The revival of the board game                    |
// |                                                                  |
// | Marco Staginski                                                  |
// | captain@captain-online.de                                        |
// |------------------------------------------------------------------|
// | File: opcodes.hh                                                 |
//  ==================================================================

#ifndef OPCODES_HH
#define OPCODES_HH

//just client-server message opcodes
 
#define QUIT          1000
#define CHAT          1001
#define LOGIN         1002
#define CREATE        1003
#define ACK           1004
#define NAK           1005
#define CHAT_PERSON   1006
#define HELP          1007
#define LIST          1008
#define DRAW          1009  //unused
#define PLAY          1010
#define START         1011
#define END           1012
#define NO_TURN       1013
#define MOVE          1014
#define LEFT_DIR      1015
#define RIGHT_DIR     1016
#define UP_DIR        1017
#define DOWN_DIR      1018
#define CONNECT       1019
#define TURN          1020
#define POSITION      1021
#define OPEN          1022
#define ATTACK        1023
#define DEFEND        1024
#define NO_ACTION     1025
#define DAMAGE        1026
#define WHOIS         1027
#define SECRETDOOR    1028
#define SPELL         1029
#define FIGHT					1030
#define USE_ITEM      1031
#define TREASURE      1032
#define WHATIS        1033
#define TAKE_OFF      1034
#define SPAWN         1035
#define SCRIPT        1036
#define TRAP          1037
#define DISARM        1038
#define JUMP          1039
#define DUMP          1040 //unused
#define REPLAY        1041 //unused
#define DROP          1042

#endif
