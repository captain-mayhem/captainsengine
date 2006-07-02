#ifndef MENU_H
#define MENU_H

//! This class is responsible for menu and button handling
class Menu{
public:
  //! the main menu
  static void mainMenu();
  //! connect button
  static void connect();
  //! login button
  static void login();
  //! package button
  static void package();
  //! level button
  static void level();
  //! load level button
  static void loadLevel();
  //! movement button
  static void move();
  //! end turn button
  static void endTurn();
  //! start game button
  static void start();
  //! play button
  static void play();
  //! open door button
  static void open();
  //! attack button
  static void attack();
  //! defend button
  static void defend();
  //! search button
  static void search();
  //! search button
  static void secretdoor();
  //! search button
  static void treasure();
  //! search button
  static void trap();
  //! inventory button
  static void inventory();
  //! use button
  static void use();
  //! use item on something
  static void useOn(Vector2D click);
  //! takeoff button
  static void takeoff();
  //! whatis button
  static void whatis();
  //! close inventory
  static void close();
  //! trap button
  static void trapMenu();
  //! disarm button
  static void disarm();
  //! other button
  static void other();
  //! whois button
  static void whois();
  //! whois button
  static void whois(Vector2D click);
  private:
};

#endif

