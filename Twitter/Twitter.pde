/* -*- c++ -*-
 *
 * Twitter.pde
 *
 * Author: Markku Rossi <mtr@iki.fi>
 *
 * Copyright (c) 2011 Markku Rossi
 *
 * This program is free software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see
 * <http://www.gnu.org/licenses/>.
 *
 */

#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <sha1.h>
#include <Time.h>
#include <EEPROM.h>
#include <Twitter.h>

/* OneWire bus pin. */
#define ONE_WIRE_BUS 4

OneWire one_wire(ONE_WIRE_BUS);
DallasTemperature sensors(&one_wire);

/* Local network configuration. */
uint8_t mac[6] =     {0xC4, 0x2C, 0x3A, 0x3B, 0xC5};
uint8_t ip[4] =      {172, 21, 38, 108};
uint8_t gateway[4] = {172, 21, 38, 1};
uint8_t subnet[4] =  {255, 255, 254, 0};

/* Connect via HTTP proxy. */
uint8_t twitter_ip[4] = {172, 16, 99, 10};
uint16_t twitter_port = 8080;

unsigned long last_tweet = 0;

#define TWEET_DELTA (15L * 60L)

/* Work buffer for twitter client.  This shold be fine for normal
   operations, the biggest items that are stored into the working
   buffer are URL encoded consumer and token secrets and HTTP response
   header lines. */
char buffer[512];

const static char consumer_key[] PROGMEM = "3azqS8rD5Ku7MRHY74qFRg";
const static char consumer_secret[] PROGMEM
= "S67Jon338GJ0sNxMyWVe5ccfrH1z3HtjrVGj3xiXApQ";

Twitter twitter(buffer, sizeof(buffer));

void
setup()
{
  Serial.begin(9600);
  Serial.println("Arduino Twitter demo");

  sensors.begin();

  Ethernet.begin(mac, ip, gateway, subnet);

  twitter.set_twitter_endpoint(PSTR("api.twitter.com"),
                               PSTR("/1/statuses/update.json"),
                               twitter_ip, twitter_port, true);
  twitter.set_client_id(consumer_key, consumer_secret);

#if 1
  /* Read OAuth account identification from EEPROM. */
  twitter.set_account_id(256, 384);
#else
  /* Set OAuth account identification from program memory. */
  twitter.set_account_id(PSTR("*** set account access token here ***"),
                         PSTR("*** set account token secret here ***"));
#endif

  delay(500);
}

void
loop()
{
  if (twitter.is_ready())
    {
      unsigned long now = twitter.get_time();

      if (last_tweet == 0)
        {
          /* First round after twitter initialization. */
          Serial.print("Time is ");
          Serial.println(now);

          /* Wait few seconds before making our first tweet.  This
             gives our sensors some time to get running (I hope). */
          last_tweet = now - TWEET_DELTA + 15L;
        }

      sensors.requestTemperatures();

      float temp = sensors.getTempCByIndex(0);

      Serial.println(temp);

      if (temp != DEVICE_DISCONNECTED && now > last_tweet + TWEET_DELTA)
        {
          char msg[32];
          long val = temp * 100L;

          sprintf(msg, "Office temperature is %ld.%02ld\302\260C",
                  val / 100L, val % 100L);

          Serial.print("Posting to Twitter: ");
          Serial.println(msg);

          last_tweet = now;

          if (twitter.post_status(msg))
            Serial.println("Status updated");
          else
            Serial.println("Update failed");
        }
    }

  delay(5000);
}
