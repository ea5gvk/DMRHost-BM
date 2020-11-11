/*
 *   Copyright (C) 2016,2017,2018,2020 by Jonathan Naylor G4KLX
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include "OLED.h"
#include "Log.h"

static bool networkInfoInitialized = false;
static unsigned char passCounter = 0;

//Logo MMDVM for Idle Screen
static unsigned char logo_glcd_bmp[] =
{
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x01, 0xF8, 0x03, 0xFC, 0x7F, 0x80, 0x3F, 0xC7, 0xFF, 0xFC, 0xF8, 0x00, 0xF9, 0xFC, 0x01, 0xFE,
0x01, 0xFC, 0x07, 0xFC, 0x7F, 0xC0, 0x7F, 0xC4, 0x00, 0x02, 0x48, 0x00, 0x91, 0xFE, 0x03, 0xFE,
0x03, 0xFC, 0x07, 0xFC, 0x7F, 0xC0, 0x7F, 0xC5, 0xFF, 0xF1, 0x24, 0x01, 0x23, 0xFE, 0x03, 0xFE,
0x03, 0xFE, 0x0F, 0xBC, 0x7B, 0xE0, 0xFB, 0xC5, 0x00, 0x09, 0x24, 0x01, 0x23, 0xDF, 0x07, 0xDE,
0x07, 0xDE, 0x0F, 0x3C, 0x79, 0xE0, 0xF3, 0xC5, 0x00, 0x05, 0x12, 0x02, 0x47, 0xCF, 0x07, 0x9E,
0x07, 0x9F, 0x1F, 0x3C, 0x79, 0xF1, 0xF3, 0xC5, 0x00, 0x05, 0x12, 0x02, 0x47, 0x8F, 0x8F, 0x9E,
0x0F, 0x8F, 0x1E, 0x3C, 0x78, 0xF1, 0xE3, 0xC5, 0x00, 0x05, 0x09, 0x04, 0x8F, 0x87, 0x8F, 0x1E,
0x0F, 0x0F, 0xBE, 0x3C, 0x78, 0xFB, 0xE3, 0xC5, 0x00, 0x05, 0x09, 0x04, 0x8F, 0x07, 0xDF, 0x1E,
0x1F, 0x07, 0xFC, 0x3C, 0x78, 0x7F, 0xC3, 0xC5, 0x00, 0x05, 0x04, 0x89, 0x1F, 0x03, 0xFE, 0x1E,
0x1E, 0x03, 0xFC, 0x3C, 0x78, 0x7F, 0xC3, 0xC5, 0x00, 0x09, 0x04, 0x89, 0x1E, 0x01, 0xFE, 0x1E,
0x3E, 0x03, 0xF8, 0x3C, 0x78, 0x3F, 0x83, 0xC5, 0xFF, 0xF1, 0x02, 0x72, 0x3E, 0x01, 0xFC, 0x1E,
0x3C, 0x01, 0xF0, 0x3C, 0x78, 0x1F, 0x03, 0xC4, 0x00, 0x02, 0x02, 0x02, 0x3C, 0x00, 0xF8, 0x1E,
0x7C, 0x01, 0xF0, 0x3C, 0x78, 0x1F, 0x03, 0xC7, 0xFF, 0xFC, 0x01, 0xFC, 0x7C, 0x00, 0xF8, 0x1E,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//Logo DMR 128x16 px
static unsigned char logo_dmr_bmp[] =
{
0x00, 0x01, 0xFF, 0xFF, 0xF8, 0x01, 0xF8, 0x00, 0x00, 0x1F, 0x1F, 0xFF, 0xFF, 0xFC, 0x00, 0x00,
0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x81, 0xFC, 0x00, 0x00, 0x3F, 0x1F, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xE1, 0xFE, 0x00, 0x00, 0xFF, 0x1F, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x0F, 0xF1, 0xFF, 0x80, 0x01, 0xFF, 0x1F, 0x80, 0x00, 0x1F, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x03, 0xF9, 0xFF, 0xC0, 0x03, 0xFF, 0x1F, 0x80, 0x00, 0x0F, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x01, 0xF9, 0xFF, 0xE0, 0x07, 0xFF, 0x1F, 0x80, 0x00, 0x0F, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x01, 0xFD, 0xF3, 0xF0, 0x1F, 0x9F, 0x1F, 0x80, 0x00, 0x1F, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x00, 0xFD, 0xF1, 0xFC, 0x3F, 0x1F, 0x1F, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x00, 0xFD, 0xF0, 0xFE, 0x7E, 0x1F, 0x1F, 0xFF, 0xFF, 0xFF, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x01, 0xFD, 0xF0, 0x7F, 0xFC, 0x1F, 0x1F, 0xFF, 0xFF, 0xFC, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x01, 0xF9, 0xF0, 0x1F, 0xF0, 0x1F, 0x1F, 0x81, 0xFC, 0x00, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x07, 0xF9, 0xF0, 0x0F, 0xE0, 0x1F, 0x1F, 0x80, 0x7F, 0x00, 0x00, 0x00,
0x00, 0x01, 0xF8, 0x00, 0x3F, 0xF1, 0xF0, 0x07, 0xC0, 0x1F, 0x1F, 0x80, 0x3F, 0xC0, 0x00, 0x00,
0x00, 0x01, 0xFF, 0xFF, 0xFF, 0xC1, 0xF0, 0x03, 0x80, 0x1F, 0x1F, 0x80, 0x0F, 0xF0, 0x00, 0x00,
0x00, 0x01, 0xFF, 0xFF, 0xFF, 0x01, 0xF0, 0x00, 0x00, 0x1F, 0x1F, 0x80, 0x03, 0xFC, 0x00, 0x00,
0x00, 0x01, 0xFF, 0xFF, 0xF0, 0x01, 0xF0, 0x00, 0x00, 0x1F, 0x1F, 0x80, 0x01, 0xFF, 0x00, 0x00
};

// Logo POCASG/DAPNET, 128x16px
const unsigned char logo_POCSAG_bmp [] =
{
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
0xff, 0xff, 0xff, 0xf8, 0x7f, 0xfe, 0x03, 0xfe, 0xfe, 0x03, 0xdf, 0xf6, 0x00, 0x00, 0x1f, 0xff,
0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0xfc, 0xfc, 0xfe, 0xfc, 0xcf, 0xf6, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0x7f, 0xfe, 0xfe, 0x7d, 0x7e, 0xfe, 0xc7, 0xf6, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xfb, 0x7a, 0x7e, 0xff, 0x79, 0x7e, 0xfe, 0xd3, 0xf6, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xf7, 0xbe, 0xff, 0x7b, 0xbe, 0xfe, 0xdb, 0xf6, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xbe, 0xff, 0xbb, 0xbe, 0xfe, 0xdd, 0xf6, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xf9, 0xff, 0xbe, 0xff, 0xb7, 0xde, 0xfe, 0xde, 0xf6, 0x01, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xee, 0x77, 0xbe, 0xff, 0xb7, 0xde, 0x81, 0xde, 0x76, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xdf, 0xb7, 0x7e, 0xff, 0xa0, 0x1e, 0xff, 0xdf, 0x36, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xdf, 0xbc, 0xfe, 0xff, 0x6f, 0xee, 0xff, 0xdf, 0xb6, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xdf, 0xbf, 0xfe, 0xff, 0x6f, 0xee, 0xff, 0xdf, 0xd6, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xdf, 0xbf, 0xfe, 0xfe, 0xdf, 0xf6, 0xff, 0xdf, 0xe6, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xdf, 0x7f, 0xfe, 0xf9, 0xdf, 0xf6, 0xff, 0xdf, 0xe6, 0xff, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xe6, 0x7f, 0xfe, 0x07, 0xff, 0xf6, 0xff, 0xdf, 0xf6, 0x00, 0xfb, 0xff, 0xff,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff
};

COLED::COLED(unsigned char displayType, unsigned char displayBrightness, bool displayInvert, bool displayScroll, bool displayRotate, bool displayLogoScreensaver, bool slot1Enabled, bool slot2Enabled) :
m_displayType(displayType),
m_displayBrightness(displayBrightness),
m_displayInvert(displayInvert),
m_displayScroll(displayScroll),
m_displayRotate(displayRotate),
m_displayLogoScreensaver(displayLogoScreensaver),
m_slot1Enabled(slot1Enabled),
m_slot2Enabled(slot2Enabled),
m_ipaddress(),
m_display()
{
}

COLED::~COLED()
{
}

bool COLED::open()
{

    // SPI
    if (m_display.oled_is_spi_proto(m_displayType))
    {
        // SPI change parameters to fit to your LCD
        if ( !m_display.init(OLED_SPI_DC,OLED_SPI_RESET,OLED_SPI_CS, m_displayType) )
            return false;
    }
    else
    {
        // I2C change parameters to fit to your LCD
        if ( !m_display.init(OLED_I2C_RESET, m_displayType) )
            return false;
    }


    m_display.begin();

    m_display.invertDisplay(m_displayInvert ? 1 : 0);
    if (m_displayBrightness > 0U)
        m_display.setBrightness(m_displayBrightness);

    if (m_displayRotate > 0U) {
      m_display.sendCommand(0xC0);
      m_display.sendCommand(0xA0);
    }

    // init done
    m_display.setTextWrap(false); // disable text wrap as default
    m_display.clearDisplay();   // clears the screen  buffer
    m_display.display();        // display it (clear display)

    OLED_statusbar();
    m_display.setCursor(0,OLED_LINE3);
    m_display.print("Startup");
    m_display.display();

    return true;
}

void COLED::setIdleInt()
{
    m_mode = MODE_IDLE;

    m_display.clearDisplay();
    OLED_statusbar();

//    m_display.setCursor(0,30);
//    m_display.setTextSize(3);
//    m_display.print("Idle");

//    m_display.setTextSize(1);
    if (m_displayScroll && m_displayLogoScreensaver)
        m_display.startscrolldiagleft(0x00,0x0f);  //the MMDVM logo scrolls the whole screen
    m_display.display();

    unsigned char info[100U];
    CNetworkInfo* m_network;

    passCounter ++;
    if (passCounter > 253U)
        networkInfoInitialized = false;

    if (! networkInfoInitialized) {
        //LogMessage("Initialize CNetworkInfo");
        info[0]=0;
        m_network = new CNetworkInfo;
        m_network->getNetworkInterface(info);
        m_ipaddress = (char*)info;
        delete m_network;

        networkInfoInitialized = true;
        passCounter = 0;
    }
}

void COLED::setErrorInt(const char* text)
{
    m_mode = MODE_ERROR;

    m_display.clearDisplay();
    OLED_statusbar();

    m_display.setTextWrap(true);    // text wrap temorally enable
    m_display.setCursor(0,OLED_LINE1);
    m_display.printf("%s\n",text);
    m_display.setTextWrap(false);

    m_display.display();
}

void COLED::setQuitInt()
{
    m_mode = MODE_QUIT;

    m_display.clearDisplay();
    OLED_statusbar();

    m_display.setCursor(0,30);
    m_display.setTextSize(3);
    m_display.print("Stopped");

    m_display.setTextSize(1);
    m_display.display();
}

void COLED::writeDMRInt(unsigned int slotNo,const std::string& src,bool group,const std::string& dst,const char* type)
{
    if (m_mode != MODE_DMR) {
        m_display.clearDisplay();
        m_mode = MODE_DMR;
        clearDMRInt(slotNo);
    }
    // if both slots, use lines 2-3 for slot 1, lines 4-5 for slot 2
    // if single slot, use lines 2-3
    if ( m_slot1Enabled && m_slot2Enabled ) {

        if (slotNo == 1U) {
            m_display.fillRect(0,OLED_LINE2,m_display.width(),40,BLACK);
            m_display.setCursor(0,OLED_LINE2);
            m_display.printf("%s", src.c_str());
            m_display.setCursor(0,OLED_LINE3);
            m_display.printf("Slot: %i %s %s%s",slotNo,type,group ? "TG: " : "",dst.c_str());
        }
        else
        {
            m_display.fillRect(0,OLED_LINE4,m_display.width(),40,BLACK);
            m_display.setCursor(0,OLED_LINE4);
            m_display.printf("%s", src.c_str());
            m_display.setCursor(0,OLED_LINE5);
            m_display.printf("Slot: %i %s %s%s",slotNo,type,group ? "TG: " : "",dst.c_str());
        }

        m_display.fillRect(0,OLED_LINE6,m_display.width(),20,BLACK);
        m_display.setCursor(0,OLED_LINE6);
        m_display.printf("%s",m_ipaddress.c_str());
    }
    else
    {
        m_display.fillRect(0,OLED_LINE2,m_display.width(),m_display.height(),BLACK);
        m_display.setCursor(0,OLED_LINE2);
        m_display.printf("%s", src.c_str());
        m_display.setCursor(0,OLED_LINE3);
        m_display.printf("Slot: %i %s %s%s",slotNo,type,group ? "TG: " : "",dst.c_str());
    }

    OLED_statusbar();
    m_display.display();
}

void COLED::clearDMRInt(unsigned int slotNo)
{
    // if both slots, use lines 2-3 for slot 1, lines 4-5 for slot 2
    // if single slot, use lines 2-3
    if ( m_slot1Enabled && m_slot2Enabled ){
        if (slotNo == 1U) {
            m_display.fillRect(0, OLED_LINE3, m_display.width(), 40, BLACK);
            m_display.setCursor(0,OLED_LINE3);
            m_display.print("Slot: 1 Listening");
        }
        else {
            m_display.fillRect(0, OLED_LINE5, m_display.width(), 40, BLACK);
            m_display.setCursor(0, OLED_LINE5);
            m_display.print("Slot: 2 Listening");
        }
    }
    else {
        m_display.fillRect(0, OLED_LINE2, m_display.width(), m_display.height(), BLACK);
        m_display.setCursor(0,OLED_LINE3);
        m_display.printf("Slot: %i Listening",slotNo);
    }

    m_display.fillRect(0, OLED_LINE6, m_display.width(), 20, BLACK);
    m_display.setCursor(0,OLED_LINE6);
    m_display.printf("%s",m_ipaddress.c_str());
    m_display.display();
}

void COLED::writePOCSAGInt(uint32_t ric, const std::string& message)
{
    m_mode = MODE_POCSAG;

    m_display.clearDisplay();
    m_display.fillRect(0, OLED_LINE1, m_display.width(), m_display.height(), BLACK);

    m_display.setCursor(0,OLED_LINE3);
    m_display.printf("RIC: %u", ric);

    m_display.setTextWrap(true);    // text wrap temorally enable
    m_display.setCursor(0,OLED_LINE5);
    m_display.printf("MSG: %s", message.c_str());
    m_display.setTextWrap(false);

    OLED_statusbar();
    m_display.display();

}

void COLED::clearPOCSAGInt()
{
    m_display.fillRect(0, OLED_LINE1, m_display.width(), m_display.height(), BLACK);

    m_display.setCursor(40,OLED_LINE4);
    m_display.print("Listening");

    m_display.setCursor(0,OLED_LINE6);
    m_display.printf("%s",m_ipaddress.c_str());

    m_display.display();
}

void COLED::writeCWInt()
{
    m_display.clearDisplay();

    m_display.setCursor(0,30);
    m_display.setTextSize(3);
    m_display.print("CW TX");

    m_display.setTextSize(1);
    m_display.display();
    if (m_displayScroll)
        m_display.startscrollleft(0x02,0x0f);
}

void COLED::clearCWInt()
{
    m_display.clearDisplay();

    m_display.setCursor(0,30);
    m_display.setTextSize(3);
    m_display.print("Idle");

    m_display.setTextSize(1);
    m_display.display();
    if (m_displayScroll)
        m_display.startscrollleft(0x02,0x0f);
}

void COLED::close()
{
    m_display.clearDisplay();
    m_display.fillRect(0, 0, m_display.width(), 16, BLACK);
    if (m_displayScroll)
        m_display.startscrollleft(0x00,0x01);
    m_display.setCursor(0,00);
    m_display.setTextSize(2);
    m_display.print("-CLOSE-");
    m_display.display();

    m_display.close();
}

void COLED::OLED_statusbar()
{
    m_display.stopscroll();
    m_display.fillRect(0, 0, m_display.width(), 16, BLACK);
    m_display.setTextColor(WHITE);

    m_display.setCursor(0,0);
    if (m_mode == MODE_DMR)
        m_display.drawBitmap(0, 0, logo_dmr_bmp, 128, 16, WHITE);
    else if (m_mode == MODE_POCSAG)
        m_display.drawBitmap(0, 0, logo_POCSAG_bmp, 128, 16, WHITE);
    else if (m_displayLogoScreensaver)
        m_display.drawBitmap(0, 0, logo_glcd_bmp, 128, 16, WHITE);

    if (m_displayScroll)
        m_display.startscrollleft(0x00,0x01);
}
