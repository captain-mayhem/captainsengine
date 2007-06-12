#ifndef MENU_H
#define MENU_H

namespace Gui{
class GuiElement;
}

//! This class is responsible for menu and button handling
class Menu{
public:
  //! the main menu
  static void mainMenu(Gui::GuiElement* elem);
  //! connect button
  static void connect(Gui::GuiElement* elem);
  //! login button
  static void login(Gui::GuiElement* elem);
  //! package button
  static void package(Gui::GuiElement* elem);
  //! level button
  static void level(Gui::GuiElement* elem);
  //! load level button
  static void loadLevel(Gui::GuiElement* elem);
  //! movement button
  static void move(Gui::GuiElement* elem);
  //! end turn button
  static void endTurn(Gui::GuiElement* elem);
  //! start game button
  static void start(Gui::GuiElement* elem);
  //! play button
  static void play(Gui::GuiElement* elem);
  //! open door button
  static void open(Gui::GuiElement* elem);
  //! attack button
  static void attack(Gui::GuiElement* elem);
  //! attack button for long range
  static void attackOn(Vector2D click);
  //! defend button
  static void defend(Gui::GuiElement* elem);
  //! search button
  static void search(Gui::GuiElement* elem);
  //! search button
  static void secretdoor(Gui::GuiElement* elem);
  //! search button
  static void treasure(Gui::GuiElement* elem);
  //! search button
  static void trap(Gui::GuiElement* elem);
  //! inventory button
  static void inventory(Gui::GuiElement* elem);
  //! use button
  static void use(Gui::GuiElement* elem);
  //! use item on something
  static void useOn(Vector2D click);
  //! takeoff button
  static void takeoff(Gui::GuiElement* elem);
  //! whatis button
  static void whatis(Gui::GuiElement* elem);
  //! close inventory
  static void close(Gui::GuiElement* elem);
  //! trap button
  static void trapMenu(Gui::GuiElement* elem);
  //! disarm button
  static void disarm(Gui::GuiElement* elem);
  //! jump over traps
  static void jump(Gui::GuiElement* elem);
  //! other button
  static void other(Gui::GuiElement* elem);
  //! whois button
  static void whois(Gui::GuiElement* elem);
  //! whois button
  static void whois(Vector2D click);
  //! shop button
  static void shop(Gui::GuiElement* elem);
  //! close shop button
  static void closeShop(Gui::GuiElement* elem);
  //! whatis (shop) button
  static void whatisShop(Gui::GuiElement* elem);
  //! buy button
  static void buy(Gui::GuiElement* elem);
  //! sell button
  static void sell(Gui::GuiElement* elem);
  //! play zargon button
  static void zargon(Gui::GuiElement* elem);
  //! drop button
  static void drop(Gui::GuiElement* elem);
  //! pickup button
  static void pickup(Gui::GuiElement* elem);
  //! create hero button
  static void createHero(Gui::GuiElement* elem);
  //! create hero execute button
  static void chexec(Gui::GuiElement* elem);
  private:
};

#endif

