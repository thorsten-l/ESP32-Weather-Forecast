#include "DisplayHandler.hpp"

// 'Ä', 'Ö', 'Ü', 'ä', 'ö', 'ü', 'ß', '§'
uint16_t umlaute_utf8[] = { 0xc384, 0xc396, 0xc39c, 0xc3a4, 0xc3b6, 0xc3bc, 0xc39f, 0xc2a7 };
uint8_t umlaute_iso8859[] = { 0xc4, 0xd6, 0xdc, 0xe4, 0xf6, 0xfc, 0xdf, 0xa7 };

char *DisplayHandler::utf8ToIso8859( char *isobuffer, String utf8String )
{
  isobuffer[0] = 0;
  int bi = 0;

  for ( int i=0; i<utf8String.length(); i++ )
  {
    uint16_t c = (uint16_t)utf8String.charAt(i);
    
    if ( c == 0xc2 || c == 0xc3 )
    {
      i++;
      c <<= 8;
      c |= (uint16_t)utf8String.charAt(i);

      for( int j=0; j<8; j++)
      {
        if ( umlaute_utf8[j] == c )
        {
          c = umlaute_iso8859[j];
          break;
        }
      }
    }
    
    isobuffer[bi++] = c;
    isobuffer[bi] = 0;
  }

  return isobuffer;
}

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

String DisplayHandler::getTimeString(time_t time)
{
  struct tm timeinfo;
  char buffer[6];
  localtime_r(&time, &timeinfo);
  sprintf(buffer, "%02d:%02d", timeinfo.tm_hour, timeinfo.tm_min);
  return String(buffer);
}

int DisplayHandler::zoneShift(int utc_hour, int zone)
{
  return (utc_hour + zone) % 24;
}
