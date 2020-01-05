#include "DisplayHandler.hpp"

void DisplayHandler::showError()
{
  display.setTextColor(GxEPD_RED);
  display.setFont(&DejaVuSansCondensed_Bold8pt8b);
  display.setCursor( 470, 364 );
  display.print( "Verbindungsfehler!" );
}

void DisplayHandler::showCentered(int x, int y, int cwidth, const char *text)
{
  int16_t x1, y1;
  uint16_t w, h;

  display.getTextBounds(text,
                        (uint16_t)x, (uint16_t)y, &x1, &y1, &w, &h);
  display.setCursor((x - 1) + ((cwidth - w) / 2), y);
  display.print(text);
}
