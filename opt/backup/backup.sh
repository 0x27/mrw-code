#!/bin/bash

echo "Taking a backup of important data. Giving each process 30 minutes to complete, otherwise they'll lock up the machine."

# Zip up everything I need to backup
filename="/home/mwhitehead/opt/backup/backups-temp/backup.tar.gz"
echo $filename

echo Dump SQL data to file...
# Take a dump of the mysql database
nice -n 19 timeout 30m mysqldump --defaults-file=mysql-opts mydata > /home/mwhitehead/opt/backup/backups-temp/mydata.sql

echo Zip up all files or use or interest...
# Tar a list of things up - don't include the mysql database cos its too large to compress, and
# don't include the actual zip file itself which is written to one of the directories included in the tar
nice -n 19 timeout 30m tar --ignore-failed-read -cvzf $filename --exclude='backup.tar.gz' --exclude='/home/mwhitehead/opt/backup/backups-temp/mydata.sql' /etc/init.d /etc/init /etc/crontab /etc/cron.* /etc/fstab /opt /home/mwhitehead/*.sh /home/mwhitehead/*.xml /home/mwhitehead/init.d /home/mwhitehead/Web /home/mwhitehead/opt /etc/samba /etc/mpd* /etc/icecast2* >/dev/null

minimumsize=50000000
actualsize=$(wc -c "$filename" | cut -f 1 -d ' ')
if [ $actualsize -ge $minimumsize ]; then
    echo MySQL backup is over $minimumsize bytes - uploading to Dropbox...
    nice -n 19 timeout 30m /home/mwhitehead/opt/dropboxuploader/dropbox_uploader.sh upload /home/mwhitehead/opt/backup/backups-temp/mydata.sql  "My Files/Backup"
else
    echo MySQL backup is less than $minimumsize bytes - not uploading to Dropbox
fi

