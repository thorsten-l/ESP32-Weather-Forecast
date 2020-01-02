#include "DisplayHandler.hpp"

#define CX 20
#define CY 16
#define CWIDTH 144

char *cAbDays[] = {"Mo", "Di", "Mi", "Do", "Fr", "Sa", "So"};
char *cFlDays[] = {"Sonntag", "Montag", "Dienstag", "Mittwoch", "Donnerstag", "Freitag", "Samstag"};
char *cFlMonth[] = {"Januar", "Februar", "März", "April", "Mai", "Juni", "Juli", "August", "September", "Oktober", "November", "Dezember"};
int daysOfMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

void DisplayHandler::showCalendar(time_t now, struct tm timeinfo, int x, int y)
{
  char buffer[32];
  char monthBuffer[32];
  int day = timeinfo.tm_mday;
  char *dayName = cFlDays[timeinfo.tm_wday];

  int noDays = daysOfMonth[timeinfo.tm_mon];
  int year = timeinfo.tm_year + 1900;

  if (timeinfo.tm_mon == 1 && timeinfo.tm_year % 4 == 0)
  {
    noDays = 29;
  }

  time_t firstDayOfMonth = now - ((timeinfo.tm_mday - 1) * 86400);
  localtime_r(&firstDayOfMonth, &timeinfo);

  int firstDay = (timeinfo.tm_wday + 6) % 7;

  y += 22;
  display.setTextColor(GxEPD_BLACK);
  display.setFont(&DejaVuSans_Bold10pt8b);
  showCentered(x, y, CWIDTH, dayName);

  y += 68;
  display.setFont(&DejaVuSans_Bold36pt8b);
  sprintf(buffer, "%02d", day);
  showCentered(x, y, CWIDTH, buffer);

  y += 32;
  display.setFont(&DejaVuSans_Bold10pt8b);
  showCentered(x, y, CWIDTH, utf8ToIso8859( monthBuffer, String(cFlMonth[timeinfo.tm_mon])));

  y += 32;
  sprintf(buffer, "%02d", year);
  showCentered(x, y, CWIDTH, buffer);

  y += 32;
  display.setFont(&DejaVuSansMono_Bold7pt8b);
  for (int i = 0; i < 7; i++)
  {
    display.setCursor(x + (i * CX), y);
    display.print(cAbDays[i]);
  }

  y += 4;
  display.drawLine(0, y, CWIDTH, y, GxEPD_BLACK);

  y += CY - 1;
  for (int i = 0; i < noDays; i++)
  {
    display.setCursor(x + ((((i + firstDay) % 7) * CX)), y + ((((i + firstDay) / 7) * CY)));

    if (i + 1 == day)
    {
      display.setTextColor(GxEPD_RED);
      display.printf("%02d", (i + 1));
      display.setTextColor(GxEPD_BLACK);
    }
    else
    {
      display.printf("%02d", (i + 1));
    }
  }
}

