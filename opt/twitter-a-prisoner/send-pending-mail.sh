#!/bin/bash

# Check if there's anything to send

date >>/tmp/prison-email-ready-checker.log
echo "Checking..." >>/tmp/prison-email-ready-checker.log

if [ -e /temp/twitpris-email-v2.txt ]
then
   echo "-----------------------------" >>/tmp/prison-email-ready-checker.log
   echo "There's a new email to send to Mark" >>/tmp/prison-email-ready-checker.log
   date >>/tmp/prison-email-ready-checker.log

   # Perform a login to the website

   curl -k -d "txtEmail=matthew1001%40hotmail.com&txtPassword=suka621oMzr50mdo&btnSubmit=Login" -c /tmp/curlcookies.txt https://emaplogin.prison-technology-services.com/login_act.cfm 1>>/tmp/prison-email-ready-checker.log 2>>/tmp/prison-email-ready-checker.log

   # Read the list of previously sent emails (to check we're logged in ok)

   curl -k -b /tmp/curlcookies.txt https://emaplogin.prison-technology-services.com/mailbox/sent/ | grep -A 6 "Mark Alexander" 1>>/tmp/prison-email-ready-checker.log 2>>/tmp/prison-email-ready-checker.log

   # Read a URL encoded HTTP form from file, which can be read by "curl --data". The file includes:
   #   - The required form fields to send the email correctly
   #   - The actual text, urlencoded and added as a form field called txtMessage

   emailtext=`cat /temp/twitpris-email-v2.txt`
   echo $emailtext >>/tmp/prison-email-ready-checker.log

   # Submit the email to the website using the current cookies
   curl -k -b /tmp/curlcookies.txt -d "$emailtext" https://emaplogin.prison-technology-services.com/mailbox/compose/handlemessage.cfm 1>>/tmp/prison-email-ready-checker.log 2>>/tmp/prison-email-ready-checker.log

   if [ $? -eq 0 ]
   then
      # Archive the current email so it's only sent once
      timestamp=`date +%s`
      echo "Moving /temp/twitpris-email-v2.txt to /home/mwhitehead/tap-archive" >>/tmp/prison-email-ready-checker.log
      mv /temp/twitpris-email-v2.txt /home/mwhitehead/tap-archive/email-$timestamp.txt
   else
      echo "Error during send with curl. Trying again." >>/tmp/prison-email-ready-checker.log
      curl -k -b /tmp/curlcookies.txt -d "$emailtext" https://emaplogin.prison-technology-services.com/mailbox/compose/handlemessage.cfm 1>>/tmp/prison-email-ready-checker.log 2>>/tmp/prison-email-ready-checker.log
      if [ $? -eq 0 ]
      then
         # Now delete it
         timestamp=`date +%s`
         echo "Moving /temp/twitpris-email-v2.txt to /home/mwhitehead/tap-archive" >>/tmp/prison-email-ready-checker.log
         mv /temp/twitpris-email-v2.txt /home/mwhitehead/tap-archive/email-$timestamp.txt
      else
         echo "Error during send with curl. Leaving /temp/twitpris-email-v2.txt for next cron job." >>/tmp/prison-email-ready-checker.log
      fi
   fi
#else
   # For now, don't put anything in the log if there's nothing to send
   #date >>/tmp/prison-email-ready-checker.log
   #echo "Nothing new to send" >>/tmp/prison-email-ready-checker.log
fi

