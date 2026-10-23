if (EMSCRIPTEN)
  # the demo game as a data package for the web page: Adventure.html?data=demo
  cge_web_package(demo data@data DESTINATION adventure)
else()
  INSTALL(DIRECTORY data DESTINATION adventure)
endif()
INSTALL(FILES testadventure.adz DESTINATION examples COMPONENT TestAdventure)
INSTALL(FILES airfight.adz DESTINATION examples COMPONENT Airfight)