#!/bin/bash

date = `date`
echo $date
echo "Taking a backup of important data. Giving each process 30 minutes to complete, otherwise they'll lock up the machine."

# Zip up everything I need to backup
mkdir /home/mwhitehead/backups-temp
mysqlfilename="/home/mwhitehead/backups-temp/mydata.sql"
zipfilename="/home/mwhitehead/backups-temp/backup.tar.gz"
echo $mysqlfilename
echo $zipfilename

echo Dump SQL data to file...
# Take a dump of the mysql database
nice -n 19 timeout 30m mysqldump --defaults-file=/home/mwhitehead/mrw-code/opt/backup/mysql-opts mydata > $mysqlfilename

echo Zip up all files of use or interest...
# Tar a list of things up - don't include the mysql database cos its too large to compress, and
# don't include the actual zip file itself which is written to one of the directories included in the tar
nice -n 19 timeout 30m tar --ignore-failed-read -cvzf $zipfilename --exclude='backup.tar.gz' --exclude='/home/mwhitehead/backups-temp/mydata.sql' /etc/init.d /etc/init /etc/crontab /etc/cron.* /etc/fstab /opt /home/mwhitehead/mrw-code/*.sh /home/mwhitehead/mrw-code/*.xml /home/mwhitehead/mrw-code/init.d /home/mwhitehead/mrw-code/Web /home/mwhitehead/mrw-code/opt /etc/samba /etc/mpd* /etc/icecast2* >/dev/null

minimumsize=90000000
actualsize=$(wc -c "$mysqlfilename" | cut -f 1 -d ' ')
if [ $actualsize -ge $minimumsize ]; then
    echo MySQL backup is over $minimumsize bytes - uploading to Dropbox...
    cp $mysqlfilename /home/mwhitehead/Dropbox/My\ Files/Data
else
    echo MySQL backup is less than $minimumsize bytes - not uploading to Dropbox in case there was an error creating the MYSQL dump
fi

minimumsize=50000000
actualsize=$(wc -c "$zipfilename" | cut -f 1 -d ' ')
if [ $actualsize -ge $minimumsize ]; then
    echo ZIP backup file is over $minimumsize bytes - uploading to Dropbox...
    cp $zipfilename /home/mwhitehead/Dropbox/My\ Files/GeneralBackup
else
    echo ZIP backup is less than $minimumsize bytes - not uploading to Dropbox in case there was an error creating the zip file
fi
