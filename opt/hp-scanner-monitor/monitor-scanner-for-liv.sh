#!/bin/bash

export printerIP=192.168.0.15:8080

echo Starting HP printer monitor

while :
do
   sleep 5

   # For now don't print to the log just cos we're attempting to connect
   # echo Connecting to printer at $printerIP

   # Submit an initial request just to check the printer's online
   xml=$(curl -s -X GET http://$printerIP/WalkupScan/WalkupScanDestinations)

   # We should check the XML/HTTP response here and barf if it's not correct
   echo "$xml"

   if [[ -z "$xml" ]]; then

      # We didn't get a good XML response, probably because we're not connected
      # echo "No connection to printer"
      echo ""

   else

      # Send a POST request containing XML which describes us
      response=$(curl -s -v -X POST -d @ourdetails-liv.xml --header 'Content-Type: text/xml' http://$printerIP/WalkupScan/WalkupScanDestinations 2>&1 | grep Location)

      # Strip off the preceding "< Location: " text to get the URI and then trim trailing whitespace
      url="${response:12}"
      url="${url%"${url##*[![:space:]]}"}"

      if [[ -z "$url" ]]; then

         # We didn't get a good URL - we might have been disconnected
         echo "Connection lost before we got our unique URL"
      
      else

         echo ""
         echo "Response after adding ourselves to its list..."
         echo "$url"

         # Get the initial set of events from the scanner
         xml=$(curl -s -X GET http://$printerIP/EventMgmt/EventTable)

         if [[ -z "$xml" ]]; then

            # We didn't get a good response - we might have been disconnected
            echo "Connection lost before we could check the printer has acknowledged us"
       
         else

            echo ""
            echo "Response after checking for the first time for new events..."
            echo "$xml"

            while :
            do
               sleep 5

               # Send a GET request to check for new scan events
               xml=$(curl -s -X GET http://$printerIP/EventMgmt/EventTable?timeout=1200)

               echo ""
               echo "Response after polling for new events..."
               echo "$xml"

               if [[ -z "$xml" ]]; then

                  # We didn't get a good response - we might have been disconnected
                  echo "Connection lost before we could check the printer has acknowledged us"
                  break

               else

                  if [[ "$xml" == *"PoweringDownEvent"* ]]; then

                     # The printer is powering down
                     echo "The printer is powering down. Waiting for it to come back online."
                     break

                  else

                     if [[ "$xml" == *"$url"* ]]; then

                        echo "XML response contains a new event for us"
 
                        # Send a request to our unique URL to get any specific details we need
                        xml=$(curl -s -X GET $url)

                        if [[ -z "$xml" ]]; then

                           # We didn't get a good response - we might have been disconnected
                           echo "Connection lost before we could check the printer has acknowledged us"

                        else
     
                           echo ""
                           echo "Response after checking our unique URL..."
                           echo "$xml"

                           # Send a request to get the scan status
                           xml=$(curl -s -X GET http://$printerIP/Scan/Status)

                           if [[ -z "$xml" ]]; then

                              # We didn't get a good response - we might have been disconnected
                              echo "Connection lost before we could get the scanner status"

                           else
     
                              echo ""
                              echo "Response after getting the scan status..."
                              echo "$xml"

                              # Send a request to specify the scan settings
                              response=$(curl -s -v -X POST -d @scandetails.xml --header 'Content-Type: text/xml' http://$printerIP/Scan/Jobs 2>&1 | grep Location)
     
                              # Strip off the preceding "< Location: " text to get the URI and trim trailing whitespace
                              joburl=${response:12}
                              joburl="${joburl%"${joburl##*[![:space:]]}"}"

                              echo ""
                              echo "Job URL after setting scanner settings..."
                              echo "$joburl"

                              # Send a request to the job url
                              binaryurl=$(curl -s -X GET $joburl | xpath -q -e "/j:Job/ScanJob/PreScanPage/BinaryURL/text()") 
    
                              echo ""
                              echo "Binary URL is:"
                              echo "$binaryurl"

                              while :
                              do

                                 # Send a request to get the current job state 
                                 jobstate=$(curl -s -X GET $joburl | xpath -q -e "/j:Job/j:JobState/text()")

                                 echo ""
                                 echo "Job state after checking the job url..."
                                 echo "$jobstate"

                                 sleep 60

                                 echo "Get the image which is probably done by now"
                                 xml=$(curl -s -X GET -o image.jpg http://$printerIP$binaryurl)

                                 xml=$(curl -s -X GET $joburl)
          
                                 echo ""
                                 echo "Emailing the image"
                                 echo "A scanned image is attached" | mutt -s "Scanned Image" -a image.jpg -- Olivia.Hough@uk.penguingroup.com
                                 rm image.jpg

                                 break
                              done

                              # Go back to checking for new events
                              xml=$(curl -s -X GET http://$printerIP/EventMgmt/EventTable)

                              echo ""
                              echo "Response after checking for the first time for new events..."
                              echo "$xml"
                           fi
                        fi
                     else
                        echo "The URL isn't present"
                     fi
                  fi
               fi
            done
         fi
      fi
   fi
done
