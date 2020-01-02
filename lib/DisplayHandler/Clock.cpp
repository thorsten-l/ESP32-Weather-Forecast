#include "DisplayHandler.hpp"

void DisplayHandler::showClock(struct tm timeinfo, int x, int y)
{
  display.setFont(&DejaVuSans_Bold8pt8b);
  display.setCursor(x, y);
  display.print("Aktualisierung");
  display.setFont(&DejaVuSansMono_Bold24pt8b);
  display.setCursor(x, y + 45);
  display.printf("%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
}
